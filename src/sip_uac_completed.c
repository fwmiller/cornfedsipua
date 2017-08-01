#include <string.h>
#include "dns.h"
#include "log.h"
#include "sip.h"

static void
sip_uac_completed_cleanup(sip_user_agent_t ua, int code)
{
	char *reason_phrase;

	if (ua == NULL)
		return;

	sip_user_agent_clear(ua);

	reason_phrase = sip_get_reason_phrase(code);
	if (reason_phrase != NULL)
		log_msg(LOG_INFO, reason_phrase);

	ua->uac_completed();
}

void
sip_uac_completed(sip_user_agent_t ua, sip_dialog_t dialog,
		  msglines_t msglines, int code)
{
	char buf[BUFSIZE], host[BUFSIZE];
	int len, port, result;

	if (ua == NULL || dialog == NULL || msglines == NULL)
		return;

	if (!timerisset(&(dialog->rtt))) {
		sip_dialog_rtt_stop(dialog);
		ua->ua_set_rtt(dialog->remote_uri.endpoint.host,
			       &(dialog->rtt));
	}
	sip_uac_check_route_set(dialog, msglines);
	sip_uac_check_remote_target(dialog, msglines, dns_avail(ua));

        memset(host, 0, BUFSIZE);
	result = sip_get_send_host_port(dialog, host, &port, dns_avail(ua));
	if (result < 0)
		return;

	/* Cancel INVITE retry timer now */
	sip_timer_cancel(&(dialog->timers));

	/* Generate ACK */
	sip_gen_ack(ua, dialog, buf, BUFSIZE);

	/* Send ACK */
	len = sip_send(ua, host, port, buf, strlen(buf));

	/*
	 * At this point, the original INVITE transaction has been
	 * completed.  One of two things needs to happen.  Either
	 * dialog has been establish and we just need to clean things
	 * up for the call or we were challenged and we need to prepare
	 * another transaction that has authorization.
	 */
	if (code >= 400 && code < 500) {
		char *proxy_auth, *www_auth;

		/* Check for authentication line */
		proxy_auth = msglines_find_hdr(msglines,
					       "Proxy-Authenticate");
		www_auth = msglines_find_hdr(msglines, "WWW-Authenticate");

		if (proxy_auth == NULL && www_auth == NULL)
			log_msg(LOG_WARNING, "No authentication header line");

		else if (strlen(ua->registration->authorization) == 0) {
			struct timeval rtt;
			char buf[BUFSIZE];

			/* Try to authenticate */
			result = sip_uac_authenticate(
				ua, dialog, msglines, "INVITE",
				dialog->auth_user);
			if (result < 0) {
				sip_uac_completed_cleanup(ua, code);
				return;
			}
			/* Generate and send INVITE */
			dialog->local_seq++;
			sip_gen_invite(ua, dialog, buf, BUFSIZE);
			sip_send(ua, dialog->remote_uri.endpoint.host,
				 dialog->remote_uri.endpoint.port, buf,
				 strlen(buf));

			/*
			 * Move dialog back to a state similar to when the
			 * initial INVITE was sent
			 */
			sip_dialog_set_state(ua->dialog, SIPS_UAC_CALLING);
			memset(ua->dialog->remote_tag, 0, BUFSIZE);

			/* Restart INVITE retry timer */
			ua->ua_get_rtt(ua->dialog->remote_uri.endpoint.host,
				       &rtt);
			sip_timer_start(&(dialog->timers), SIPT_INVITE_RETR,
					rtt.tv_usec);
#if 0
			log_msg(LOG_INFO, "Send authenticated INVITE");
#endif
			return;
		}

	}
	if (len > 0)
		sip_uac_completed_cleanup(ua, code);
}
