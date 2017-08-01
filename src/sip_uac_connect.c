#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "history.h"
#include "lex.h"
#include "log.h"
#include "sdp.h"
#include "sip.h"

void
sip_uac_connect(sip_user_agent_t ua, sip_dialog_t dialog,
		msglines_t msglines)
{
	sip_dialog_t d;
	sdp_codec_list_t codec_list;
	char buf[BUFSIZE], host[BUFSIZE], s[128];
	int codec, len, port, result;

	if (ua == NULL || dialog == NULL || msglines == NULL)
		return;

	if (!timerisset(&(dialog->rtt))) {
		sip_dialog_rtt_stop(dialog);
		ua->ua_set_rtt(dialog->remote_uri.endpoint.host,
			       &(dialog->rtt));
	}
	sip_uac_check_route_set(dialog, msglines);
	sip_uac_check_remote_target(dialog, msglines, dns_avail(ua));

	/* Look for remote RTP host, port, and codec in answered SDP */
	sdp_parse(ua, msglines, codec_list);
	codec = sdp_choose_codec(codec_list);
	if (codec < 0) {
		log_msg(LOG_WARNING, "No codec agreed to");
		return;
	} else
		ua->rtp.codec = codec;

        memset(host, 0, BUFSIZE);
	result = sip_get_send_host_port(dialog, host, &port,
					dns_avail(ua));
	if (result < 0)
		return;

	/*
	 * XXX Despite the fact that there are a lot of things that
	 * need to be looked at before we're sure we're going to the
	 * SIPS_CONNECTED state a little later on, we'll cancel the
	 * INVITE retry timer now
	 */
	sip_timer_cancel(&(dialog->timers));

	/*
	 * This ACK is being generated for a received 200 OK that connects
	 * a call.  The branch parameter for the Via header needs to be
	 * different than the used for the original INVITE
	 */
	memset(dialog->via_hdrs->branch, 0, BUFSIZE);
	sip_gen_branch(dialog->via_hdrs->branch);

	/* Generate ACK for the dialog that the connection will use */
	sip_gen_ack(ua, dialog, buf, BUFSIZE);

	/* Send ACK */
	len = sip_send(ua, host, port, buf, strlen(buf));
	if (len >= 0) {
		rtp_stats_start_session(&(ua->rtp.stats));
		sip_dialog_set_state(dialog, SIPS_CONNECTED);

		soundcard_flush(&(ua->soundcard));

		log_msg(LOG_CONNECTION, "UAC Connected Call-ID [%s]",
			dialog->call_id);
	}
	/*
	 * Eliminate extra dialogs that may have accumulated during a fork
	 * of the outbound INVITE
	 */
	for (; ua->dialog != NULL;) {
		d = ua->dialog;
		sip_dialog_list_remove(d, &(ua->dialog));
		if (d != dialog)
			sip_dialog_push(d);
	}
	sip_dialog_list_insert_head(dialog, &(ua->dialog));

	memset(s, 0, 128);
	sip_uri_gen(&(ua->dialog->remote_uri), s);
	history_add_event(ua, HIST_EVENT_CONNECTED, s);

	if (ua->uac_connect != NULL)
		ua->uac_connect();
}
