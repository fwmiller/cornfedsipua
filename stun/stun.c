#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http.h"
#include "dns.h"
#include "log.h"
#include "sip.h"
#include "stun.h"
#include "stun_impl.h"
#include "stun_transactions.h"

struct stun_response_code {
	int code;
	char *reason_phrase;
};

static struct stun_response_code response_codes[] = {
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{420, "Unknown Attribute"},
	{430, "Stale Credentials"},
	{431, "Integrity Check Failed"},
	{432, "Missing Username"},
	{433, "Use TLS"},
	{500, "Server Error"},
	{600, "Global Failure"}
};

static char *
stun_get_reason_phrase(int code)
{
	int i;

	for (i = 0; i < sizeof(response_codes); i++)
		if (code == response_codes[i].code)
			return response_codes[i].reason_phrase;
	return NULL;
}

u8_t *
stun_find_attr(u8_t *msg, int len, u16_t type)
{
	int pos;

	for (pos = sizeof(struct stun_hdr); pos < len;) {
		stun_attr_hdr_t hdr;
		u16_t attr_type, attr_len;

		hdr = (stun_attr_hdr_t) (msg + pos);
		attr_type = ntohs(hdr->type);
		attr_len = ntohs(hdr->length);

		if (attr_type == type)
			return msg + pos + sizeof(struct stun_attr_hdr);

		pos += sizeof(struct stun_attr_hdr) + attr_len;
	}
	return NULL;
}

static void
stun_check_error_response(u8_t *msg, u16_t type, int len)
{
	if (type == STUN_TYPE_BINDING_ERROR) {
		u8_t *attr;
		stun_error_code_t error_code;
		char *reason_phrase;
		int code;

		/* Find ERROR-CODE attribute */
		attr = stun_find_attr(msg, len, STUN_ATTR_ERROR_CODE);
		if (attr == NULL) {
			log_msg(LOG_WARNING, "Missing ERROR-CODE attribute");
			return;
		}
		/* Parse ERROR_CODE attribute */
		error_code = (stun_error_code_t) attr;
		*((u32_t *) &error_code) = ntohl(*((u32_t *) &error_code));
		code = (error_code->code.error_class * 100) +
		       error_code->code.error_number;
		reason_phrase = stun_get_reason_phrase(code);
		if (reason_phrase == NULL)
			log_msg(LOG_WARNING, "Binding Error received %d",
				code);
		else
			log_msg(LOG_WARNING, "Binding Error received %d %s",
				code, reason_phrase);
		return;
	}
	log_msg(LOG_WARNING, "Expected Binding Response (%04x)", type);
}

int
stun_parse_binding_response(u8_t *msg,
			    char *mapped_address,
			    int *mapped_port)
{
	stun_hdr_t hdr;
	stun_address_t addr;
	struct in_addr in;
	u8_t *attr;
	u16_t type;
	int len;

	hdr = (stun_hdr_t) msg;
	type = ntohs(hdr->type);
	len = ntohs(hdr->length);

	if (type != STUN_TYPE_BINDING_RESPONSE) {
		stun_check_error_response(msg, type, len);
		return (-1);
	}
	/* Find MAPPED-ADDRESS attribute */
	attr = stun_find_attr(msg, len, STUN_ATTR_MAPPED_ADDRESS);
	if (attr == NULL) {
		log_msg(LOG_WARNING, "Missing MAPPED-ADDRESS attribute");
		return (-1);
	}
	addr = (stun_address_t) attr;
	if (addr->family != STUN_ADDRESS_FAMILY_IPV4) {
		log_msg(LOG_WARNING, "Expected IPv4 address family");
		return (-1);
	}
	in.s_addr = addr->address;
	strcpy(mapped_address, inet_ntoa(in));
	*mapped_port = ntohs(addr->port);
#if 0
	log_msg(LOG_INFO, "Mapped IP address %s", mapped_address);
	log_msg(LOG_INFO, "Mapped port %u", *mapped_port);
#endif
	return 0;
}

void
stun_dump_msg(char *action, u8_t *msg)
{
	stun_hdr_t hdr;
	char s[256], tid[64];
	int i;

	hdr = (stun_hdr_t) msg;

	memset(tid, 0, 64);
	for (i = 0; i < 12; i++)
		sprintf(tid + strlen(tid), "%02X", hdr->tid[i]);

	memset(s, 0, 256);
	sprintf(s, "%s STUN ", action);

	switch (ntohs(hdr->type)) {
	case STUN_TYPE_BINDING_REQUEST:
		sprintf(s + strlen(s), "Binding Request");
		break;

	case STUN_TYPE_BINDING_RESPONSE:
		sprintf(s + strlen(s), "Binding Response");
		break;

	case STUN_TYPE_BINDING_ERROR:
		sprintf(s + strlen(s), "Binding Error");
		break;

	case STUN_TYPE_SHARED_SECRET_REQUEST:
		sprintf(s + strlen(s), "Shared Secret Request");
		break;

	case STUN_TYPE_SHARED_SECRET_RESPONSE:
		sprintf(s + strlen(s), "Shared Secret Response");
		break;

	case STUN_TYPE_SHARED_SECRET_ERROR:
		sprintf(s + strlen(s), "Shared Secret Error");
		break;

	default:
		sprintf(s + strlen(s), "message");
	}
	sprintf(s + strlen(s), " tid [%s]", tid);
	log_msg(LOG_INFO, s);
}

void
stun_init(sip_user_agent_t ua, char *host)
{
	char *ipaddr;
	int ipaddrlen, result;
	struct timeval now, dur;
	struct timezone tz;

	stun_transactions_init();

	log_msg(LOG_INFO, "STUN server [%s]", host);

	/* Check whether ua->stun_server_host is an IP address */
	result = find_ip_address(host, BUFSIZE, &ipaddr, &ipaddrlen);
	if (result == 0) {
		strncpy(ua->stun_server.host, ipaddr, ipaddrlen);

		log_msg(LOG_INFO, "STUN server IP address [%s]",
			ua->stun_server.host);

	} else {
		char ipaddr[128];

		strcpy(ua->stun_server.domain, host);

		if (!dns_avail(ua)) {
			log_msg(LOG_ERROR, "STUN server specified using domain name and DNS is disabled");
			return;
		}
		result = dns_gethostbyname(host, ipaddr, 128);
		if (result < 0) {
			log_msg(LOG_INFO, "STUN server IP address not found");
			return;
		}
		strcpy(ua->stun_server.host, ipaddr);
	}
	log_msg(LOG_INFO, "STUN server IP address [%s]",
		ua->stun_server.host);

	/* Set initial STUN keepalive timeout */
	dur.tv_sec = 2;
	dur.tv_usec = 0;

	gettimeofday(&now, &tz);

	timeradd(&now, &dur, &(ua->stun_keepalive_end));
}

void
stun_receive_response(sip_user_agent_t ua, char *msg, int len)
{
	stun_transaction_t transaction;
	char mapped_address[128];
	int mapped_port;
	char *tid;
#if 0
	stun_dump_msg("Received", (u8_t *) msg);
#endif
	memset(mapped_address, 0, 128);
	stun_parse_binding_response((u8_t *) msg, mapped_address,
				    &mapped_port);

	/* Variable part of transaction ID */
	tid = msg + 8;

	transaction = stun_transaction_find(tid);
	if (transaction == NULL)
		return;

	stun_transaction_list_remove(transaction);

	if (transaction->protocol == STUN_PROTOCOL_SIP) {
#if 0
		log_msg(LOG_INFO, "SIP visible address [%s] port %d",
			mapped_address, mapped_port);
#endif
		strcpy(ua->visible_endpoint.host, mapped_address);
		ua->visible_endpoint.port = mapped_port;

	} else if (transaction->protocol == STUN_PROTOCOL_RTP) {
#if 0
		log_msg(LOG_INFO, "RTP visible address [%s] port %d",
			mapped_address, mapped_port);
#endif
		strcpy(ua->rtp.visible.host, mapped_address);
		ua->rtp.visible.port = mapped_port;
	}
	stun_transaction_push(transaction);
}

void
stun_gen_tid(u8_t *tid)
{
	int i;

	for (i = 0; i < 3; i++) {
		u32_t r = (u32_t) rand();
		tid[i << 2] = r & 0xff;
		tid[(i << 2) + 1] = (r >> 8) & 0xff;
		tid[(i << 2) + 2] = (r >> 16) & 0xff;
		tid[(i << 2) + 3] = (r >> 24) & 0xff;
	}
}

void
stun_gen_binding_request(u8_t *tid, u8_t *msg)
{
	stun_hdr_t hdr;

	hdr = (stun_hdr_t) msg;
	hdr->type = htons(STUN_TYPE_BINDING_REQUEST);
	hdr->length = 0;
	hdr->magic = ntohl(STUN_MAGIC);
	memcpy(hdr->tid, tid, 12);
}

int
stun_send_binding_request(sip_user_agent_t ua, int fd, int protocol)
{
	stun_transaction_t transaction;
	struct in_addr in;
	struct sockaddr_in addr;
	stun_hdr_t hdr;
	u8_t msg[BUFSIZE];
	int sent;

	/* Allocate STUN transaction */
	transaction = stun_transaction_pop();
	if (transaction == NULL)
		return (-1);

	/* Generate Binding Request */
	memset(msg, 0, BUFSIZE);
	stun_gen_tid(transaction->tid);
	stun_gen_binding_request(transaction->tid, msg);

	transaction->protocol = protocol;

	/* Send Binding Request */
	inet_aton(ua->stun_server.host, &in);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = in.s_addr;
	addr.sin_port = htons(STUN_PORT);
	hdr = (stun_hdr_t) msg;
	sent = sendto(fd, msg, sizeof(struct stun_hdr) + ntohs(hdr->length),
		      0, (struct sockaddr *) &addr, sizeof(addr));
	if (sent < 0) {
		log_msg(LOG_ERROR, "Socket send failed (%s)",
			strerror(errno));
		return (-1);
	}
	stun_transaction_list_insert(transaction);

	return 0;
}

void
stun_thread_func(sip_user_agent_t ua)
{
	struct timeval now;
	struct timezone tz;


	if (!timerisset(&(ua->stun_keepalive_end)))
		return;

	gettimeofday(&now, &tz);
	
	if (timercmp(&now, &(ua->stun_keepalive_end), >=)) {
		if (strlen(ua->stun_server.host) > 0) {
			/* Send STUN requests out SIP port */
			if (ua->sipfd >= 0)
				stun_send_binding_request(ua, ua->sipfd,
							  STUN_PROTOCOL_SIP);

			/* Send STUN requests out RTP port */
			if (ua->rtp.rtpfd >= 0)
				stun_send_binding_request(ua, ua->rtp.rtpfd,
							  STUN_PROTOCOL_RTP);
		}
		timeradd(&(ua->stun_keepalive_end),
			 &(ua->stun_keepalive_interval),
			 &(ua->stun_keepalive_end));
	}
}
