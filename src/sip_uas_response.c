#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "sip.h"

extern char *via_transports[];

long random(void);

static int
sip_uas_get_hdrs(msglines_t msglines, sip_via_t *via_hdrs, char **to,
		 char **from, char **call_id, char **cseq, int dns)
{
	char *buf;
	int i;

	if (msglines == NULL || via_hdrs == NULL || to == NULL ||
	    from == NULL || call_id == NULL || cseq == NULL)
		return (-1);

	*via_hdrs = NULL;
	buf = msglines->msgbuf;
	for (i = 0; i < msglines->lines; i++) {
		char *hdr = buf + msglines->pos[i];
		if (strncasecmp(hdr, "Via:", 4) == 0 ||
		    strncasecmp(hdr, "v:", 2) == 0) {
			sip_via_t via;
			int result;

			via = sip_via_pop();
			if (via == NULL)
				return (-1);
			result = sip_via_parse(hdr, via, dns);
			if (result < 0) {
				sip_via_push(via);
				return (-1);
			}
			sip_via_list_insert_tail(via, via_hdrs);
		}
	}
	if (*via_hdrs == NULL)
		return (-1);
	*to = msglines_find_hdr(msglines, "To");
	if (*to == NULL)
		return (-1);
	*from = msglines_find_hdr(msglines, "From");
	if (*from == NULL)
		return (-1);
	*call_id = msglines_find_hdr(msglines, "Call-ID");
	if (*call_id == NULL)
		return (-1);
	*cseq = msglines_find_hdr(msglines, "CSeq");
	if (*cseq == NULL)
		return (-1);

	return 0;
}

static void
sip_uas_gen_resp_hdrs(char *s, sip_via_t via_hdrs, char *to, char *from,
		      char *call_id, char *cseq)
{
	sip_via_t via;

	if (s == NULL || via_hdrs == NULL || to == NULL ||
	    from == NULL || call_id == NULL || cseq == NULL)
		return;

	/* Via headers */
	for (via = via_hdrs; via != NULL; via = via->next)
		sprintf(s + strlen(s),
			"Via: SIP/2.0/%s %s:%d;branch=%s\r\n",
			via_transports[via->transport], via->endpoint.host,
			via->endpoint.port, via->branch);

	/* To header */
	sprintf(s + strlen(s), "%s\r\n", to);

	/* From header */
	sprintf(s + strlen(s), "%s\r\n", from);

	/* Call-ID header */
	sprintf(s + strlen(s), "%s\r\n", call_id);

	/* CSeq header */
	sprintf(s + strlen(s), "%s\r\n", cseq);

	/* Allow header */
	sprintf(s + strlen(s),
		"Allow: INVITE, ACK, OPTIONS, BYE, CANCEL\r\n");
}

void
sip_uas_method_not_allowed(sip_user_agent_t ua, msglines_t msglines)
{
	sip_via_t via_hdrs;
	char s[BUFSIZE];
	char *call_id, *cseq, *from, *to;
	int result;

	if (ua == NULL || msglines == NULL)
		return;

	/*
	 * Generate the 405 Method Not Allowed response.  This is different
	 * than a typical call to sip_gen_response() because we don't want
	 * to rely on the presence of a valid dialog, everything we need
	 * has to come from the incoming request message.
	 */
	result = sip_uas_get_hdrs(msglines, &via_hdrs, &to, &from,
				  &call_id, &cseq, dns_avail(ua));
	if (result < 0)
		return;

	/* Generate response */
	memset(s, 0, BUFSIZE);
	sprintf(s, "SIP/2.0 405 Method Not Allowed\r\n");
	sip_uas_gen_resp_hdrs(s, via_hdrs, to, from, call_id, cseq);

	/* Content-Length header */
	sprintf(s + strlen(s), "Content-Length: 0\r\n\r\n");

	/* Via hosts have been verified to be IP addresses */
	sip_send(ua, via_hdrs->endpoint.host, via_hdrs->endpoint.port, s,
		 strlen(s));

	sip_free_via_hdrs(&via_hdrs);
}

void
sip_uas_busy_here(sip_user_agent_t ua, msglines_t msglines)
{
	sip_via_t via_hdrs;
	char s[BUFSIZE];
	char *call_id, *cseq, *from, *to;
	int result;

	if (ua == NULL || msglines == NULL)
		return;

	/*
	 * Generate the 486 Busy Here response.  This is different than
	 * a typical call to sip_gen_response() because we don't want
	 * to rely on the presence of a valid dialog, everything we need
	 * has to come from the incoming request message.
	 */
	result = sip_uas_get_hdrs(msglines, &via_hdrs, &to, &from,
				  &call_id, &cseq, dns_avail(ua));
	if (result < 0)
		return;

	/* Generate response */
	memset(s, 0, BUFSIZE);
	sprintf(s, "SIP/2.0 486 Busy Here\r\n");
	sip_uas_gen_resp_hdrs(s, via_hdrs, to, from, call_id, cseq);

	/* Content-Length header */
	sprintf(s + strlen(s), "Content-Length: 0\r\n\r\n");

	/* Via hosts have been verified to be IP addresses */
	sip_send(ua, via_hdrs->endpoint.host, via_hdrs->endpoint.port, s,
		 strlen(s));

	sip_free_via_hdrs(&via_hdrs);
}

void
sip_uas_request_pending(sip_user_agent_t ua, msglines_t msglines)
{
	sip_via_t via_hdrs;
	char s[BUFSIZE];
	char *call_id, *cseq, *from, *to;
	int result;

	if (ua == NULL || msglines == NULL)
		return;

	/*
	 * Generate the 491 Request Pending response.  This is different
	 * than a typical call to sip_gen_response() because we don't want
	 * to rely on the presence of a valid dialog, everything we need
	 * has to come from the incoming request message.
	 */
	result = sip_uas_get_hdrs(msglines, &via_hdrs, &to, &from,
				  &call_id, &cseq, dns_avail(ua));
	if (result < 0)
		return;

	/* Generate response */
	memset(s, 0, BUFSIZE);
	sprintf(s, "SIP/2.0 491 Request Pending\r\n");
	sip_uas_gen_resp_hdrs(s, via_hdrs, to, from, call_id, cseq);

	/* Content-Length header */
	sprintf(s + strlen(s), "Content-Length: 0\r\n\r\n");

	/* Via hosts have been verified to be IP addresses */
	sip_send(ua, via_hdrs->endpoint.host, via_hdrs->endpoint.port, s,
		 strlen(s));

	sip_free_via_hdrs(&via_hdrs);
}

void
sip_uas_server_error(sip_user_agent_t ua, msglines_t msglines)
{
	sip_via_t via_hdrs;
	char s[BUFSIZE];
	char *call_id, *cseq, *from, *to;
	int result, retry;

	if (ua == NULL || msglines == NULL)
		return;

	/*
	 * Generate the 500 Internal Server Error response.  This is
	 * different than a typical call to sip_gen_response() because we
	 * don't want to rely on the presence of a valid dialog, everything
	 * we need has to come from the incoming request message.
	 */
	result = sip_uas_get_hdrs(msglines, &via_hdrs, &to, &from,
				  &call_id, &cseq, dns_avail(ua));
	if (result < 0)
		return;

	/* Generate response */
	memset(s, 0, BUFSIZE);
	sprintf(s, "SIP/2.0 500 Internal Server Error\r\n");
	sip_uas_gen_resp_hdrs(s, via_hdrs, to, from, call_id, cseq);

	/* Retry-After header */
	retry = (random() % 10) + 1;
	sprintf(s + strlen(s), "Retry-After: %d\r\n", retry);

	/* Content-Length header */
	sprintf(s + strlen(s), "Content-Length: 0\r\n\r\n");

	/* Via hosts have been verified to be IP addresses */
	sip_send(ua, via_hdrs->endpoint.host, via_hdrs->endpoint.port, s,
		 strlen(s));

	sip_free_via_hdrs(&via_hdrs);
}

void
sip_uas_not_acceptable(sip_user_agent_t ua, msglines_t msglines)
{
	sip_via_t via_hdrs;
	char s[BUFSIZE];
	char *call_id, *cseq, *from, *to;
	int result, retry;

	if (ua == NULL || msglines == NULL)
		return;

	/*
	 * Generate the 500 Internal Server Error response.  This is
	 * different than a typical call to sip_gen_response() because we
	 * don't want to rely on the presence of a valid dialog, everything
	 * we need has to come from the incoming request message.
	 */
	result = sip_uas_get_hdrs(msglines, &via_hdrs, &to, &from,
				  &call_id, &cseq, dns_avail(ua));
	if (result < 0)
		return;

	/* Generate response */
	memset(s, 0, BUFSIZE);
	sprintf(s, "SIP/2.0 488 Not Acceptable Here\r\n");
	sip_uas_gen_resp_hdrs(s, via_hdrs, to, from, call_id, cseq);

	/* Warning header */
	sprintf(s + strlen(s),
		"Warning: 304 %s \"Media type not available\"\r\n",
		ua->local_endpoint.host);

	/* Retry-After header */
	retry = (random() % 10) + 1;
	sprintf(s + strlen(s), "Retry-After: %d\r\n", retry);

	/* Content-Length header */
	sprintf(s + strlen(s), "Content-Length: 0\r\n\r\n");

	/* Via hosts have been verified to be IP addresses */
	sip_send(ua, via_hdrs->endpoint.host, via_hdrs->endpoint.port, s,
		 strlen(s));

	sip_free_via_hdrs(&via_hdrs);
}

void
sip_uas_call_leg_does_not_exist(sip_user_agent_t ua, msglines_t msglines)
{
	sip_via_t via_hdrs;
	char s[BUFSIZE];
	char *call_id, *cseq, *from, *to;
	int result;

	if (ua == NULL || msglines == NULL)
		return;

	/*
	 * Generate the 481 Call Leg/Transaction Does Not Exist response.
	 * This is different than a typical call to sip_gen_response()
	 * because we don't want to rely on the presence of a valid dialog,
	 * everything we need has to come from the incoming request message.
	 */
	result = sip_uas_get_hdrs(msglines, &via_hdrs, &to, &from,
				  &call_id, &cseq, dns_avail(ua));
	if (result < 0)
		return;

	/* Generate response */
	memset(s, 0, BUFSIZE);
	sprintf(s, "SIP/2.0 481 Call Leg/Transaction Does Not Exist\r\n");
	sip_uas_gen_resp_hdrs(s, via_hdrs, to, from, call_id, cseq);

	/* Content-Length header */
	sprintf(s + strlen(s), "Content-Length: 0\r\n\r\n");

	/* Via hosts have been verified to be IP addresses */
	sip_send(ua, via_hdrs->endpoint.host, via_hdrs->endpoint.port, s,
		 strlen(s));

	sip_free_via_hdrs(&via_hdrs);
}
