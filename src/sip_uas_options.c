#include <stdio.h>
#include <string.h>
#include "dns.h"
#include "http.h"
#include "sip.h"

void
sip_uas_options(sip_user_agent_t ua, msglines_t msglines)
{
	struct sip_via via;
	char s[BUFSIZE];
	char *call_id, *cseq, *from, *to, *viahdr;
	int result;

	if (ua == NULL || msglines == NULL)
		return;

	/*
	 * Generate a special 200 OK response.  Its different than the
	 * typical call to sip_gen_response() because we don't want to
	 * rely on the presence of a valid dialog and there are some
	 * extra header lines required.
	 */
	viahdr = msglines_find_hdr(msglines, "Via");
	if (viahdr == NULL)
		return;
	to = msglines_find_hdr(msglines, "To");
	if (to == NULL)
		return;
	from = msglines_find_hdr(msglines, "From");
	if (from == NULL)
		return;
	call_id = msglines_find_hdr(msglines, "Call-ID");
	if (call_id == NULL)
		return;
	cseq = msglines_find_hdr(msglines, "CSeq");
	if (cseq == NULL)
		return;

	/* Get host and port from Via header line */
	result = sip_via_parse(viahdr, &via, dns_avail(ua));
	if (result < 0)
		return;

	/* Generate response line */
	memset(s, 0, BUFSIZE);
	sprintf(s, "SIP/2.0 200 OK\r\n");

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

	/* Accept header */
	sprintf(s + strlen(s), "Accept: application/sdp\r\n");

	/* Accept-Encoding header */
	sprintf(s + strlen(s), "Accept-Encoding:\r\n");

	/* Accept-Language header */
	sprintf(s + strlen(s), "Accept-Language: en\r\n");

	/* Content-Length header */
	sprintf(s + strlen(s), "Content-Length: 0\r\n\r\n");

	/* XXX Need to add media types accepted in an SDP body */

	/* Via host has been verified to be an IP address */
	sip_send(ua, via.endpoint.host, via.endpoint.port, s, strlen(s));
}
