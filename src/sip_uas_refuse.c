#include <string.h>
#include "log.h"
#include "sip.h"

void
sip_uas_refuse(sip_user_agent_t ua)
{
	char buf[BUFSIZE];
	int len;

	if (ua == NULL)
		return;

	/* Stop ringing */
	sip_ringing_stop(ua);

	/* Format a 480 Temporarily Not Available response */
	sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 480,
			 ua->dialog->remote_seq, "INVITE", 0);

	/* XXX Assume that all the Via header hosts are IP addresses */
	len = sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
		       ua->dialog->via_hdrs->endpoint.port, buf,
		       strlen(buf));
	if (len >= 0) {
		struct timeval rtt;

		sip_dialog_set_state(ua->dialog, SIPS_UAS_COMPLETED);

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		ua->ua_get_rtt(ua->dialog->remote_uri.endpoint.host, &rtt);
		sip_timer_start(&(ua->dialog->timers), SIPT_RESPONSE_RETR,
				rtt.tv_usec);

		log_msg(LOG_INFO, "Refused");
	}
}
