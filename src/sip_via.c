#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "http.h"
#include "lex.h"
#include "log.h"
#include "sip.h"

char *strcasestr(char *haystack, char *needle);

#define MAX_VIAS	128

static struct sip_via sip_via_pool[MAX_VIAS];
static sip_via_t sip_via_stack = NULL;



void
sip_via_stack_init()
{
	int i;

	memset(sip_via_pool, 0, MAX_VIAS * sizeof(struct sip_via));

	for (i = 0; i < MAX_VIAS; i++)
		sip_via_push(&(sip_via_pool[i]));
}

void
sip_via_push(sip_via_t via)
{
	if (via == NULL)
		return;

	sip_via_init(via);
	via->next = sip_via_stack;
	sip_via_stack = via;
}

sip_via_t
sip_via_pop()
{
	sip_via_t via = sip_via_stack;

	if (sip_via_stack != NULL) {
		sip_via_stack = sip_via_stack->next;
		via->next = NULL;
	}
	return via;
}

void
sip_via_init(sip_via_t via)
{
	if (via == NULL)
		return;

	via->prev = NULL;
	via->next = NULL;
	ipendpoint_init(&(via->endpoint));
	memset(via->branch, 0, BUFSIZE);
	memset(via->rport, 0, BUFSIZE);
	memset(via->received, 0, BUFSIZE);
	via->transport = SIPUT_UDP;
}

int
sip_via_parse(char *hdr, sip_via_t via, int dns)
{
	char local_host[BUFSIZE], s[BUFSIZE], sentby[BUFSIZE];
	char version_transport[BUFSIZE];
	char *ipaddr, *s1;
	int i, ipaddrlen, len, pos, result;

	if (hdr == NULL || via == NULL)
		return (-1);

	/* Skip over Via header label */
	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, ":", s);
	if (hdr[pos] != ':') {
		log_msg(LOG_WARNING, "Missing colon");
		return (-1);
	}
	/* Get transport protocol */
	pos++;
	memset(version_transport, 0, BUFSIZE);
	nextarg(hdr, &pos, " ", version_transport);
	len = strlen(version_transport);
	if (len > 0) {
		for (i = len - 1; i >= 0; i--)
			if (version_transport[i] == '/') {
				i++;
				break;
			}

		if (i < 0) {
			log_msg(LOG_WARNING, "Malformed transport");
			return (-1);
		}
		if (strcasecmp(version_transport + i, "udp") == 0) {
#if 0
			log_msg(LOG_INFO, "Via transport UDP");
#endif
			via->transport = SIPUT_UDP;

		} else if (strcasecmp(version_transport + i, "tcp") == 0) {
#if 0
			log_msg(LOG_INFO, "Via transport TCP");
#endif
			via->transport = SIPUT_TCP;

		} else if (strcasecmp(version_transport + i, "tls") == 0) {
#if 0
			log_msg(LOG_INFO, "Via transport TLS");
#endif
			via->transport = SIPUT_TLS;

		} else {
#if 0
			log_msg(LOG_INFO, "Unrecognized Via transport [%s]",
				version_transport + i);
#endif
			return (-1);
		}

	} else {
		log_msg(LOG_WARNING, "Missing version and transport");
		return (-1);
	}
	/* Get first sent-by address */
	memset(sentby, 0, BUFSIZE);
	nextarg(hdr, &pos, ",", sentby);

	/* Get host address */
	pos = 0;
	memset(local_host, 0, BUFSIZE);
	nextarg(sentby, &pos, ":;", local_host);

	/*
	 * Restrict Via hosts to those that are either IP addresses or
	 * those domain names that resolve to an IP address
	 */
	result = find_ip_address(local_host, BUFSIZE, &ipaddr, &ipaddrlen);
	if (result == 0)
		strcpy(via->endpoint.host, local_host);

	else {
		char hostent[128];

		if (!dns)
			return (-1);

		result = dns_gethostbyname(local_host, hostent, 128);
		if (result < 0)
			return (-1);

		strcpy(via->endpoint.host, hostent);
	}
	/* Get port */
	if (sentby[pos] == ':') {
		pos++;
		memset(s, 0, BUFSIZE);
		nextarg(sentby, &pos, ";", s);
		via->endpoint.port = atoi(s);
	} else
		via->endpoint.port = SIP_DEFAULT_PORT;

	/* Get branch value (if any) */
	s1 = strcasestr(hdr, "branch=");
	if (s1 != NULL) {
		pos = 7;
		nextarg(s1, &pos, ";", via->branch);
	}
	/* Get rport value (if any) */
	s1 = strcasestr(hdr, "rport=");
	if (s1 != NULL) {
		pos = 6;
		nextarg(s1, &pos, ";", via->rport);
	}
	/* Get received IP address (if any) */
	s1 = strcasestr(hdr, "received=");
	if (s1 != NULL) {
		pos = 9;
		nextarg(s1, &pos, ";", via->received);
	}
	return 0;
}

void
sip_via_list_insert_tail(sip_via_t via, sip_via_t *list)
{
	sip_via_t tail;

	if (via == NULL || list == NULL)
		return;

	if (*list == NULL) {
		via->prev = NULL;
		via->next = NULL;
		*list = via;
		return;
	}
	for (tail = *list;; tail = tail->next)
		if (tail->next == NULL) {
			via->prev = tail;
			via->next = NULL;
			tail->next = via;
			break;
		}
}

void
sip_free_via_hdrs(sip_via_t *via_hdrs)
{
	sip_via_t via;

	if (via_hdrs == NULL)
		return;

	for (via = *via_hdrs; via != NULL; via = *via_hdrs) {
		*via_hdrs = via->next;
		sip_via_push(via);
	}
}
