#include <stdio.h>
#include <string.h>
#include "log.h"
#include "sip.h"
#include "soundcard.h"


int
sip_uas_answer(sip_user_agent_t ua)
{
	struct timeval rtt;
	char buf[BUFSIZE];
	int len;

	if (ua == NULL)
		return (-1);

	/* Answer ringing */
	sip_ringing_answer(ua);

	/* Format a 200 OK response */
	sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 200,
			 ua->dialog->remote_seq, "INVITE", 1);

	/* XXX Assume that all the Via header hosts are IP addresses */
	len = sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
		       ua->dialog->via_hdrs->endpoint.port, buf,
		       strlen(buf));
	if (len < 0)
		return (-1);

	sip_dialog_rtt_start(ua->dialog);
	sip_dialog_set_state(ua->dialog, SIPS_CONNECTED);

	soundcard_flush(&(ua->soundcard));

	rtp_stats_start_session(&(ua->rtp.stats));

	/* Save copy of 200 OK response */
	memset(ua->dialog->last_resp, 0, BUFSIZE);
	memcpy(ua->dialog->last_resp, buf, strlen(buf));

	ua->ua_get_rtt(ua->dialog->remote_uri.endpoint.host, &rtt);
	sip_timer_start(&(ua->dialog->timers), SIPT_RESPONSE_RETR,
			rtt.tv_usec);

	log_msg(LOG_CONNECTION, "UAS Connected Call-ID [%s]",
		ua->dialog->call_id);

	return 0;
}
