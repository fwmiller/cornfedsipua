#include <string.h>
#include "dns.h"
#include "history.h"
#include "log.h"
#include "sip.h"
#include "wav.h"

int
sip_uac_hangup(sip_user_agent_t ua)
{
	sip_via_t via;
	char buf[BUFSIZE], host[BUFSIZE], s[128];
	int len, port, result;

	if (ua == NULL)
		return (-1);

        memset(host, 0, BUFSIZE);
	result = sip_get_send_host_port(ua->dialog, host, &port,
					dns_avail(ua));
	if (result < 0)
		return (-1);

	/* Clear any Via headers */
	sip_free_via_hdrs(&(ua->dialog->via_hdrs));

	/* Generate a branch parameter for the Via header */
	via = sip_via_pop();
	if (via == NULL)
		return (-1);
	sip_gen_branch(via->branch);
	ua->dialog->via_hdrs = via;

	/* Generate BYE */
	sip_gen_bye(ua, buf, BUFSIZE);

        /*
         * Send BYE
         *
         * Normally this state change would be applied after the BYE
         * message is actually sent.  A test when the client was playing
	 * .wav files on a local LAN segment resulted in a 200 OK response
         * to this BYE message arriving before the dialog state was
         * modified.  This resulted in the subsequent ACK being sent
	 * because the state machine thought the call was still up
         */
	sip_dialog_set_state(ua->dialog, SIPS_UAC_TRYING);

	len = sip_send(ua, host, port, buf, strlen(buf));
	if (len < 0)
		return (-1);

	sip_dialog_rtt_start(ua->dialog);
	wav_rec_list_flush(ua);
	rtp_stats_stop_session(&(ua->rtp.stats));

	memset(s, 0, 128);
	sip_uri_gen(&(ua->dialog->remote_uri), s);
	history_add_event(ua, HIST_EVENT_HANGUP, s);

	log_msg(LOG_CONNECTION, "UAC Disconnecting Call-ID [%s]",
		ua->dialog->call_id);

	return 0;
}

void
sip_uac_retransmit_bye(sip_user_agent_t ua)
{
	char buf[BUFSIZE], host[BUFSIZE];
	int port, result;

        memset(host, 0, BUFSIZE);
	result = sip_get_send_host_port(ua->dialog, host, &port,
					dns_avail(ua));
	if (result < 0)
		return;

	/* Generate BYE */
	sip_gen_bye(ua, buf, BUFSIZE);

	/* Send BYE */
	sip_send(ua, host, port, buf, strlen(buf));
}
