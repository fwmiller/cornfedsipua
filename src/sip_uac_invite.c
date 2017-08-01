#include <stdio.h>
#include <string.h>
#include <time.h>
#include "history.h"
#include "log.h"
#include "sip.h"

/*
 * Generate and send an initial INVITE request
 */
int
sip_uac_invite(sip_user_agent_t ua, sip_uri_t to, sip_uri_t from,
	       sip_uri_t reg)
{
	sip_via_t via;
	char buf[BUFSIZE], s[BUFSIZE];
	char *s1;
	int len;

	if (ua == NULL || to == NULL || from == NULL || reg == NULL)
		return (-1);

	/*
	 * Assume that the host portions of the to, from, and reg URIs
	 * are specified correctly
	 */

	/* Generate a branch parameter for the Via header */
	via = sip_via_pop();
	if (via == NULL)
		return (-1);
	sip_gen_branch(via->branch);
	ua->dialog->via_hdrs = via;

	/* Generate a tag parameter for the From header */
	sip_gen_tag(ua->dialog->local_tag);

	/* Generate a Call-ID */
	sip_gen_call_id(ua->dialog->call_id);

	/* XXX Set local CSeq number */
	ua->dialog->local_seq = 1;

	/* Set local URI in dialog */
	sip_uri_init(&(ua->dialog->local_uri));
	memcpy(&(ua->dialog->local_uri), from, sizeof(struct sip_uri));

	/* Set remote URI in dialog */
	sip_uri_init(&(ua->dialog->remote_uri));
	memcpy(&(ua->dialog->remote_uri), to, sizeof(struct sip_uri));

	/* Set register URI in dialog */
	sip_uri_init(&(ua->dialog->reg_uri));
	memcpy(&(ua->dialog->reg_uri), reg, sizeof(struct sip_uri));

	/* Set authorization user in dialog */
	memset(ua->dialog->auth_user, 0, BUFSIZE);
	if (strlen(ua->dialog->reg_uri.endpoint.domain) > 0)
		ua->reg_get_auth_user(ua->dialog->reg_uri.endpoint.domain,
				      ua->dialog->auth_user);
	else
		ua->reg_get_auth_user(ua->dialog->reg_uri.endpoint.host,
				      ua->dialog->auth_user);

	/* Generate INVITE request */
	sip_gen_invite(ua, ua->dialog, buf, BUFSIZE);

	/*
	 * Send INVITE request
	 *
	 * Normally this state change would be applied after the INVITE
	 * message is actually sent.  However, on networks with very low
	 * latency, it appears that response messages can get back in
	 * between when the INVITE is sent and when the dialog state is
	 * updated.
	 */
	sip_dialog_set_state(ua->dialog, SIPS_UAC_CALLING);

	len = sip_send(ua, ua->dialog->remote_uri.endpoint.host,
		       ua->dialog->remote_uri.endpoint.port, buf,
		       strlen(buf));

	if (len < 0)
		return (-1);

	sip_dialog_rtt_start(ua->dialog);

	sprintf(s, "Calling ");
	s1 = s + strlen(s);
	sip_uri_gen(&(ua->dialog->remote_uri), s1);
	history_add_event(ua, HIST_EVENT_INITIATED, s1);
	log_msg(LOG_INFO, s);

	return 0;
}

void
sip_uac_retransmit_invite(sip_user_agent_t ua, sip_dialog_t dialog)
{
	char buf[BUFSIZE];

	if (ua == NULL || dialog == NULL)
		return;

	/* Generate and send INVITE */
	sip_gen_invite(ua, dialog, buf, BUFSIZE);

	/*
	 * XXX ua->dialog->remote_uri.host should be specified correctly
	 * if we are retransmitting since it was checked when the original
	 * INVITE was sent
	 */
	sip_send(ua, ua->dialog->remote_uri.endpoint.host,
		 ua->dialog->remote_uri.endpoint.port, buf, strlen(buf));
}
