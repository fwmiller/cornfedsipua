#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "http.h"
#include "lex.h"
#include "log.h"
#include "sip.h"

char *strcasestr(char *haystack, char *needle);

void
sip_uri_init(sip_uri_t uri)
{
	if (uri == NULL)
		return;

	uri->prefix = SIPU_NULL;
	memset(uri->user, 0, BUFSIZE);
	memset(uri->passwd, 0, BUFSIZE);
	ipendpoint_init(&(uri->endpoint));
	uri->transport = SIPUT_UDP;
}

int
sip_uri_isset(sip_uri_t uri)
{
	if (uri == NULL)
		return 0;

	if (uri->prefix != SIPU_NULL && ipendpoint_isset(&(uri->endpoint)))
		return 1;

	return 0;
}

void
sip_uri_parse(char *s, sip_uri_t uri, int dns)
{
	char elem[BUFSIZE], hostport[BUFSIZE];
	char userinfo[BUFSIZE];
	char *ipaddr, *s1;
	int ipaddrlen, pos = 0, result;

	if (s == NULL || uri == NULL)
		return;

	/* Assume that uri has been cleared using sip_uri_init() */

	/* Check for leading '<' */
	s1 = index(s, '<');
	if (s1 != NULL)
		s = s1 + 1;

	/* Check for a prefix */
	if (strncmp(s, "sip:", 4) == 0) {
		uri->prefix = SIPU_SIP;
		pos = 4;
	} else if (strncmp(s, "sips:", 5) == 0) {
		uri->prefix = SIPU_SIPS;
		pos = 5;
	} else {
		/*
		 * XXX Need to deal with other URI schemes.  This will
		 * mean assigning a SIPU_NULL value to the uri->prefix
		 * field and skipping gracefully over the unsupported
		 * prefix.
		 */
	}
	/* Split into userinfo and hostport parts */
	memset(userinfo, 0, BUFSIZE);
	s1 = index(s, '@');
	if (s1 != NULL) {
		/* URI has a userinfo part */
		nextarg(s, &pos, "@", userinfo);
		pos++;
	}
	/* XXX Get hostport part ignoring URI parameters for the moment */
	memset(hostport, 0, BUFSIZE);
	nextarg(s, &pos, " ;>", hostport);
	if (strlen(userinfo) > 0) {
		/* User name */
		pos = 0;
		memset(elem, 0, BUFSIZE);
		nextarg(userinfo, &pos, ":", elem);
		strcpy(uri->user, elem);
		if (userinfo[pos] == ':') {
			/* Password */
			pos++;
			nextarg(userinfo, &pos, NULL, uri->passwd);
		}
	}
	/* Host */
	pos = 0;
	memset(elem, 0, BUFSIZE);
	nextarg(hostport, &pos, ":", elem);

	/*
	 * Check whether host is specified as a domain name or an IP
	 * address
	 */
	result = find_ip_address(elem, BUFSIZE, &ipaddr, &ipaddrlen);
	if (result == 0)
		/*
		 * Host is specified as a IP address so record the IP
		 * address in the host field
		 */
		strncpy(uri->endpoint.host, ipaddr, ipaddrlen);

	else {
		char host[128];

		if (!dns) {
			sip_uri_init(uri);
			return;
		}
		/*
		 * Host is specified as a domain name so record the
		 * domain name in domain field
		 */
		strcpy(uri->endpoint.domain, elem);

		/* Lookup the IP address associated with the domain name */
		result = dns_gethostbyname(elem, host, 128);
		if (result == 0)
			strcpy(uri->endpoint.host, host);
	}
	/* Check for port number */
	if (hostport[pos] == ':') {
		pos++;
		memset(elem, 0, BUFSIZE);
		nextarg(hostport, &pos, NULL, elem);
		uri->endpoint.port = atoi(elem);
	} else
		uri->endpoint.port = SIP_DEFAULT_PORT;

	/* Check transport parameter */
	s1 = strcasestr(s, "transport=");
	if (s1 != NULL) {
		char transport[128];

		memset(transport, 0, 128);
		pos = 10;
		nextarg(s1, &pos, " ;>", transport);
#if 0
		log_msg(LOG_INFO, "transport [%s]", transport);
#endif
		if (strcasecmp(transport, "tcp") == 0)
			uri->transport = SIPUT_TCP;
	}
}

void
sip_uri_gen(sip_uri_t uri, char *s)
{
	if (uri == NULL || s == NULL)
		return;

	/* Prefix */
	if (uri->prefix == SIPU_SIP)
		sprintf(s, "sip:");
	else if (uri->prefix == SIPU_SIPS)
		sprintf(s, "sips:");
	else {
		s[0] = '\0';
		return;
	}

	/* User with password if specified */
	if (strlen(uri->user) > 0) {
		sprintf(s + strlen(s), "%s", uri->user);
		if (strlen(uri->passwd) > 0)
			sprintf(s + strlen(s), ":%s@", uri->passwd);
		else
			sprintf(s + strlen(s), "@");
	}
	/* Host which can be either a domain name or IP address */
	if (strlen(uri->endpoint.domain) > 0)
		sprintf(s + strlen(s), "%s", uri->endpoint.domain);
	else
		sprintf(s + strlen(s), "%s", uri->endpoint.host);

	/* Port if specified as something other than the default SIP port */
	if ((uri->endpoint.port >= 0) && (uri->endpoint.port < 0x10000) &&
	    (uri->endpoint.port != SIP_DEFAULT_PORT))
		sprintf(s + strlen(s), ":%d", uri->endpoint.port);

	/* TCP transport if specified */
	if (uri->transport == SIPUT_TCP)
		sprintf(s + strlen(s), ";transport=tcp");
}

int
sip_uri_cmp(sip_uri_t uri1, sip_uri_t uri2)
{
	if (uri1 == NULL || uri2 == NULL) {
		log_msg(LOG_INFO, "URI comparison failed: NULL URI");
		return (-1);
	}
	if (uri1->prefix != uri2->prefix) {
		log_msg(LOG_INFO, "URI comparison failed: Prefix mismatch");
		return (-1);
	}
	if (strcmp(uri1->user, uri2->user) != 0) {
		log_msg(LOG_INFO, "URI comparison failed: User mismatch");
		return (-1);
	}
	if (strlen(uri1->passwd) > 0 && strlen(uri2->passwd) > 0) {
		if (strcmp(uri1->passwd, uri2->passwd) != 0) {
			log_msg(LOG_INFO, "URI comparison failed: Password mismatch (1)");
			return (-1);
		}

	} else if (strlen(uri1->passwd) > 0 || strlen(uri2->passwd) > 0) {
		log_msg(LOG_INFO, "URI comparison failed: Password mismatch (2)");
		return (-1);
	}
	return ipendpoint_cmp(&(uri1->endpoint), &(uri2->endpoint));
}
