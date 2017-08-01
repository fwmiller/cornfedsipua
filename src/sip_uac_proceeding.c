#include "dns.h"
#include "log.h"
#include "sip.h"

void
sip_uac_proceeding(sip_user_agent_t ua, sip_dialog_t dialog,
		   msglines_t msglines, int code)
{
	char *reason_phrase;

	if (ua == NULL || dialog == NULL || msglines == NULL)
		return;

	if (!timerisset(&(dialog->rtt)) && code != 100) {
		sip_dialog_rtt_stop(dialog);
		ua->ua_set_rtt(dialog->remote_uri.endpoint.host,
			       &(dialog->rtt));
	}
	/* Cancel INVITE retry time */
	sip_timer_cancel(&(dialog->timers));

	sip_uac_check_route_set(dialog, msglines);
	sip_uac_check_remote_target(dialog, msglines, dns_avail(ua));
	sip_dialog_set_state(dialog, SIPS_UAC_PROCEEDING);

	reason_phrase = sip_get_reason_phrase(code);
	if (reason_phrase != NULL)
		log_msg(LOG_INFO, reason_phrase);
}
