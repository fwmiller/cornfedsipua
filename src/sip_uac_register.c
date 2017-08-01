#include <stdio.h>
#include <string.h>
#include "log.h"
#include "sip.h"

static void
sip_uac_register_common(sip_user_agent_t ua)
{
	sip_via_t via;

	if (ua == NULL)
		return;

	/* Generate a branch parameter for the Via header */
	via = sip_via_pop();
	if (via == NULL)
		return;
	sip_gen_branch(via->branch);
	ua->registration->via_hdrs = via;

	/* Generate a tag parameter for the From header */
	sip_gen_tag(ua->registration->local_tag);

	/* Generate a Call-ID */
	sip_gen_call_id(ua->registration->call_id);

	/* Set local CSeq number */
	ua->registration->local_seq = 1;

	/* Set local URI in registration */
	sip_uri_init(&(ua->registration->local_uri));
	ua->registration->local_uri.prefix = SIPU_SIP;
	strcpy(ua->registration->local_uri.endpoint.host,
	       ua->local_endpoint.host);
	ua->registration->local_uri.endpoint.port =
		ua->local_endpoint.port;

	/* Set remote URI in registration */
	sip_uri_init(&(ua->registration->remote_uri));
	memcpy(&(ua->registration->remote_uri), &(ua->remote_uri),
	       sizeof(struct sip_uri));

	/* Set register URI in registration */
	sip_uri_init(&(ua->registration->reg_uri));
	memcpy(&(ua->registration->reg_uri), &(ua->reg_uri),
	       sizeof(struct sip_uri));
}

void
sip_uac_register(sip_user_agent_t ua)
{
	char buf[BUFSIZE];
	int len;

	if (ua == NULL)
		return;

	/* Setup common fields for registration */
	sip_uac_register_common(ua);

	/* Set authorization user in registration */
	memset(ua->registration->auth_user, 0, BUFSIZE);
	if (strlen(ua->registration->reg_uri.endpoint.domain) > 0)
		ua->reg_get_auth_user(
			ua->registration->reg_uri.endpoint.domain,
			ua->registration->auth_user);
	else
		ua->reg_get_auth_user(
			ua->registration->reg_uri.endpoint.host,
			ua->registration->auth_user);

	/* Generate and send REGISTER request */
	sip_gen_register(ua, buf, BUFSIZE, 1, ua->reg_get_interval());

	/*
	 * Send REGISTER request
	 *
	 * Normally this state change would be applied after the REGISTER
	 * message is actually sent.  However, on networks with very low
	 * latency, it appears that response messages can get back in
	 * between when the REGISTER is sent and when the dialog state is
	 * updated.
	 */
	sip_dialog_set_state(ua->registration, SIPS_UAC_REGISTERING);

	/* XXX Assume that ua->registration->reg_uri.host is specified */
	len = sip_send(ua, ua->registration->reg_uri.endpoint.host,
		       ua->registration->reg_uri.endpoint.port, buf,
		       strlen(buf));
	if (len >= 0) {
		char s[BUFSIZE];

		sip_dialog_rtt_start(ua->registration);

		memset(s, 0, BUFSIZE);
		if (strlen(ua->registration->reg_uri.endpoint.domain) > 0)
			sprintf(s, "Registering sip:%s@%s",
				ua->registration->reg_uri.user,
				ua->registration->reg_uri.endpoint.domain);
		else
			sprintf(s, "Registering sip:%s@%s",
				ua->registration->reg_uri.user,
				ua->registration->reg_uri.endpoint.host);

		if (ua->registration->reg_uri.endpoint.port !=
		    SIP_DEFAULT_PORT)
			sprintf(s + strlen(s), ":%d",
				ua->registration->reg_uri.endpoint.port);

		log_msg(LOG_INFO, s);
	}
}

void
sip_uac_retransmit_register(sip_user_agent_t ua)
{
	char buf[BUFSIZE];

	if (ua == NULL)
		return;

	/* Generate and send REGISTER */
	sip_gen_register(ua, buf, BUFSIZE, 1, ua->reg_get_interval());

	/*
	 * XXX ua->registration->reg_uri.host should be specified correctly
	 * if we are retransmitting since it was checked when the original
	 * REGISTER was sent
	 */
	sip_send(ua, ua->registration->reg_uri.endpoint.host,
		 ua->registration->reg_uri.endpoint.port, buf, strlen(buf));
}

static void
sip_uac_bad_register_cleanup(sip_user_agent_t ua)
{
	if (ua == NULL)
		return;

	if (strlen(ua->registration->reg_uri.endpoint.domain) > 0)
		ua->uac_register_failed(
			ua->registration->reg_uri.endpoint.domain);
	else
		ua->uac_register_failed(
			ua->registration->reg_uri.endpoint.host);

	sip_dialog_init(ua->registration);

	log_msg(LOG_INFO, "Registration failed");
}

void
sip_uac_bad_register(sip_user_agent_t ua, msglines_t msglines, int code)
{
	if (ua == NULL || msglines == NULL)
		return;

	/* Cancel existing REGISTER retry timer */
	sip_timer_cancel(&(ua->registration->timers));

	sip_dialog_rtt_stop(ua->registration);
	ua->ua_set_rtt(ua->registration->remote_uri.endpoint.host,
		       &(ua->registration->rtt));

	if (code >= 400 && code < 500) {
		char *proxy_auth, *www_auth;

		/* Check for authentication line */
		proxy_auth = msglines_find_hdr(msglines,
					       "Proxy-Authenticate");
		www_auth = msglines_find_hdr(msglines, "WWW-Authenticate");

		if (proxy_auth == NULL && www_auth == NULL) {
#if 0
			log_msg(LOG_WARNING, "No authentication header line");
#endif
		} else if (strlen(ua->registration->authorization) != 0) {
			char user[BUFSIZE];
			int result;

			/*
			 * An authorization has already been attempted
			 * and has failed to allow the registration to
			 * complete.  It is possible that the user name
			 * used for the authentication was wrong.  Prompt
			 * the user for an alternate user name and retry
			 * the authentication if the user specifies a new
			 * user name.
			 */
			memset(user, 0, BUFSIZE);
			result = ua->uac_register_prompt_for_user(user,
								  BUFSIZE);
			if (result == 0) {
				struct timeval rtt;
				char buf[BUFSIZE];

				memset(ua->registration->auth_user, 0,
				       BUFSIZE);
				strncpy(ua->registration->auth_user, user,
					BUFSIZE - 1);
				/*
				 * Try to authenticate using the specified
				 * alternate user
				 */
				result = sip_uac_authenticate(
					ua, ua->registration,
					msglines, "REGISTER", user);
				if (result < 0) {
					sip_uac_bad_register_cleanup(ua);
					return;
				}
				/* Generate and send REGISTER */
				ua->registration->local_seq++;
				sip_gen_register(ua, buf, BUFSIZE,
						 1, ua->reg_get_interval());

				/*
				 * XXX ua->registration->reg_uri.host should
				 * be specified correctly since this looks
				 * something like a retransmission and it
				 * was checked when the original REGISTER
				 * was sent
				 */
				sip_send(ua, ua->registration->reg_uri.endpoint.host, ua->registration->reg_uri.endpoint.port, buf, strlen(buf));

				/* Restart REGISTER retry timer */
				ua->ua_get_rtt(ua->registration->reg_uri.endpoint.host, &rtt);
				sip_timer_start(&(ua->registration->timers),
						SIPT_REGISTER_RETR,
						rtt.tv_usec);
				return;
			}

		} else if (strlen(ua->registration->authorization) == 0) {
			struct timeval rtt;
			char buf[BUFSIZE], user[BUFSIZE];
			int result;

			/* Try to authenticate */
			memset(user, 0, BUFSIZE);
			if (strlen(ua->registration->reg_uri.endpoint.domain) > 0)
				ua->reg_get_auth_user(ua->registration->reg_uri.endpoint.domain, user);
			else
				ua->reg_get_auth_user(ua->registration->reg_uri.endpoint.host, user);

			result = sip_uac_authenticate(ua, ua->registration,
						      msglines, "REGISTER",
						      user);
			if (result < 0) {
				sip_uac_bad_register_cleanup(ua);
				return;
			}
			/* Generate and send REGISTER */
			ua->registration->local_seq++;
			sip_gen_register(ua, buf, BUFSIZE,
					 1, ua->reg_get_interval());

			/*
			 * XXX ua->registration->reg_uri.host should be
			 * specified correctly since this looks something
			 * like a retransmission and it was checked when
			 * the original REGISTER was sent
			 */
			sip_send(ua, ua->registration->reg_uri.endpoint.host,
				 ua->registration->reg_uri.endpoint.port,
				 buf, strlen(buf));

			/* Restart REGISTER retry timer */
			ua->ua_get_rtt(ua->registration->reg_uri.endpoint.host, &rtt);
			sip_timer_start(&(ua->registration->timers),
					SIPT_REGISTER_RETR, rtt.tv_usec);
			return;
		}
	}
	sip_uac_bad_register_cleanup(ua);
}

void
sip_uac_unregister(sip_user_agent_t ua)
{
	char buf[BUFSIZE];
	int len;

	if (ua == NULL)
		return;

	/* Setup common fields for registration */
	sip_uac_register_common(ua);

	/* Generate and send REGISTER request */
	sip_gen_register(ua, buf, BUFSIZE, 1, ua->reg_get_interval());

	/* XXX Assume that ua->registration->reg_uri.host is specified */
	len = sip_send(ua, ua->registration->reg_uri.endpoint.host,
		       ua->registration->reg_uri.endpoint.port, buf,
		       strlen(buf));
	if (len >= 0) {
		char s[BUFSIZE];

		sip_dialog_rtt_start(ua->registration);
		sip_dialog_set_state(ua->registration, SIPS_UAC_REGISTERING);

		memset(s, 0, BUFSIZE);
		if (strlen(ua->registration->reg_uri.endpoint.domain) > 0)
			sprintf(s, "Removing registration sip:%s@%s",
				ua->registration->reg_uri.user,
				ua->registration->reg_uri.endpoint.domain);
		else
			sprintf(s, "Removing registration sip:%s@%s",
				ua->registration->reg_uri.user,
				ua->registration->reg_uri.endpoint.host);

		if (ua->registration->reg_uri.endpoint.port !=
		    SIP_DEFAULT_PORT)
			sprintf(s + strlen(s), ":%d",
				ua->registration->reg_uri.endpoint.port);

		log_msg(LOG_INFO, s);
	}
}
