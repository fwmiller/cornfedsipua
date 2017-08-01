#include <string.h>
#include "dns.h"
#include "history.h"
#include "sip.h"

void
sip_uas_hangup(sip_user_agent_t ua, msglines_t msglines)
{
	char buf[BUFSIZE];
	int len, result, seqno;

	if (ua == NULL || msglines == NULL)
		return;
#if 0
	/* Record route set */
	result = sip_parse_route_set(msglines, ua->dialog);
	if (result < 0)
		return;
#endif
	/* Record Via headers */
	result = sip_parse_via_hdrs(msglines, ua->dialog, dns_avail(ua));
	if (result < 0)
		return;

	/* Record CSeq */
	seqno = sip_parse_cseq(msglines, NULL, 0);
	if (seqno < 0)
		return;
	ua->dialog->remote_seq = seqno;

	/* Check for Require header */
	result = sip_uas_check_require(ua, msglines, "BYE");
	if (result < 0)
		return;

	/* Format a 200 OK response */
	sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 200,
			 ua->dialog->remote_seq, "BYE", 0);
	/*
	 * XXX There should not be a case where the Via hosts are anything
	 * other than IP addresses
	 */
	len = sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
		       ua->dialog->via_hdrs->endpoint.port, buf,
		       strlen(buf));
	if (len >= 0) {
		char s[128];

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		memset(s, 0, 128);
		sip_uri_gen(&(ua->dialog->remote_uri), s);
		history_add_event(ua, HIST_EVENT_HANGUP, s);

		ua->uas_hangup(ua->dialog);
		rtp_stats_stop_session(&(ua->rtp.stats));
	}
}
