#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "history.h"
#include "lex.h"
#include "log.h"
#include "sdp.h"
#include "sip.h"

void
sip_uas_invite(sip_user_agent_t ua, msglines_t msglines)
{
	sdp_codec_list_t codec_list;
	char buf[BUFSIZE], s[BUFSIZE];
	int codec, len, result, seqno;

	if (ua == NULL || msglines == NULL)
		return;

	/* Record route set */
	result = sip_parse_route_set(msglines, ua->dialog);
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	/* Record Via headers */
	result = sip_parse_via_hdrs(msglines, ua->dialog, dns_avail(ua));
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	/* Record Call-ID */
	result = sip_parse_callid(msglines, ua->dialog->call_id, BUFSIZE);
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	/* Record CSeq */
	seqno = sip_parse_cseq(msglines, NULL, 0);
	if (seqno < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	ua->dialog->remote_seq = seqno;

	/* Record URI from To header */
	result = sip_parse_to_hdr(msglines, &(ua->dialog->local_uri),
				  NULL, 0, dns_avail(ua));
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	/* Record URI and tag from From header */
	result = sip_parse_from_hdr(msglines, &(ua->dialog->remote_uri),
				    ua->dialog->remote_tag, BUFSIZE,
				    dns_avail(ua));
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	/* Record URI from Contact header */
	result = sip_parse_contact(msglines, &(ua->dialog->remote_target),
				   dns_avail(ua));
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	/* Check for Require header */
	result = sip_uas_check_require(ua, msglines, "INVITE");
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	/* Check for Supported header */
	result = sip_uas_check_supported(ua, msglines);
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		return;
	}
	/* Look for remote RTP host, port, and codec in offered SDP */
	sdp_parse(ua, msglines, codec_list);
	codec = sdp_choose_codec(codec_list);
	if (codec < 0) {
		sip_uas_not_acceptable(ua, msglines);

		log_msg(LOG_WARNING, "No acceptable codec in offered SDP");
		return;
	} else
		ua->rtp.codec = codec;

	/* Generate a local tag */
	sip_gen_tag(ua->dialog->local_tag);

	/* Do some checking on the INVITE request message */
	result = sip_uas_check_invite(ua, msglines);
	if (result < 0) {
		sip_dialog_init(ua->dialog);
		rtp_endpoint_init(&(ua->rtp.remote));
		ua->rtp.codec = (-1);
		return;
	}
	/* Format a 100 Trying response */
	sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 100,
			 ua->dialog->remote_seq, "INVITE", 0);

	/* All Via hosts must be IP addresses */
	len = sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
		       ua->dialog->via_hdrs->endpoint.port, buf,
		       strlen(buf));
	if (len >= 0) {
		sip_dialog_set_state(ua->dialog, SIPS_UAS_PROCEEDING);

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		log_msg(LOG_INFO, "Proceeding");
	}
	/* Signal user of an incoming call */
	sip_ringing_start(ua);

	/* Format a 180 Ringing response */
	sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 180,
			 ua->dialog->remote_seq, "INVITE", 0);

	/* All Via hosts must be IP addresses */
	sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
		 ua->dialog->via_hdrs->endpoint.port, buf, strlen(buf));

	memset(ua->dialog->last_resp, 0, BUFSIZE);
	memcpy(ua->dialog->last_resp, buf, strlen(buf));

	memset(s, 0, 128);
	sip_uri_gen(&(ua->dialog->remote_uri), s);
	history_add_event(ua, HIST_EVENT_RECEIVED, s);
}

void
sip_uas_reinvite(sip_user_agent_t ua, msglines_t msglines)
{
	char buf[BUFSIZE];
	int len, result, seqno;

	if (ua == NULL || msglines == NULL)
		return;

	/* Record route set */
	result = sip_parse_route_set(msglines, ua->dialog);
	if (result < 0)
		return;

	/* Record Via headers */
	result = sip_parse_via_hdrs(msglines, ua->dialog, dns_avail(ua));
	if (result < 0)
		return;

	/* Record CSeq */
	seqno = sip_parse_cseq(msglines, NULL, 0);
	if (seqno < 0)
		return;
	ua->dialog->remote_seq = seqno;

	/* Record URI from Contact header */
	result = sip_parse_contact(msglines, &(ua->dialog->remote_target),
				   dns_avail(ua));
	if (result < 0)
		return;

        /* Check for Require header */
	result = sip_uas_check_require(ua, msglines, "INVITE");
	if (result < 0)
		return;

	/* Format a 200 OK response */
	sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 200,
			 ua->dialog->remote_seq, "INVITE", 1);

	/* All Via hosts must be IP addresses */
	len = sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
		       ua->dialog->via_hdrs->endpoint.port, buf,
		       strlen(buf));
	if (len >= 0) {
		struct timeval rtt;

		sip_dialog_set_state(ua->dialog, SIPS_UAS_ACK_WAIT);

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		ua->ua_get_rtt(ua->dialog->remote_uri.endpoint.host, &rtt);
		sip_timer_start(&(ua->dialog->timers), SIPT_RESPONSE_RETR,
				rtt.tv_usec);

		log_msg(LOG_INFO, "Session parameters changed");
	}
}

void
sip_uas_retransmit_response(sip_user_agent_t ua)
{
	if (ua == NULL)
		return;

	/*
	 * XXX The Via hosts must all be IP addresses if we are
	 * retransmitting since they were checked when the original
	 * request was received
	 */
	sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
		 ua->dialog->via_hdrs->endpoint.port,
		 ua->dialog->last_resp, strlen(ua->dialog->last_resp));
}
