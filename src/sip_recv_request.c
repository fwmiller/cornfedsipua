#include <stdio.h>
#include <string.h>
#include <time.h>
#include "history.h"
#include "lex.h"
#include "log.h"
#include "sip.h"

static void
sip_recv_invite(sip_user_agent_t ua, msglines_t msglines)
{
	int result, state;

	if (ua == NULL || msglines == NULL)
		return;

	result = sip_uas_check_mandatory_invite_hdrs(msglines);
	if (result < 0)
		return;

	state = sip_dialog_get_state(ua->dialog);
	if (state == SIPS_IDLE)
		sip_uas_invite(ua, msglines);

	else if (state == SIPS_BUSY) {
		log_msg(LOG_INFO, "Do not disturb");
		sip_uas_busy_here(ua, msglines);

	} else {
		result = sip_uas_match_dialog(ua, msglines);
		if (result < 0) {
			log_msg(LOG_INFO, "Busy here");
			sip_uas_busy_here(ua, msglines);
			return;
		}
		if (state == SIPS_CONNECTED)
			sip_uas_reinvite(ua, msglines);

		else {
			int seqno;

			seqno = sip_parse_cseq(msglines, NULL, 0);
			if (seqno < ua->dialog->remote_seq)
				sip_uas_server_error(ua, msglines);
			else
				sip_uas_retransmit_response(ua);
		}
	}
}

static void
sip_recv_ack(sip_user_agent_t ua, msglines_t msglines)
{
	int result, state;

	if (ua == NULL || msglines == NULL)
		return;

	result = sip_uas_check_mandatory_ack_hdrs(msglines);
	if (result < 0)
		return;

	/* Do some checking on the ACK request message */
	result = sip_uas_check_ack(ua, msglines);
	if (result != 0)
		return;

	state = sip_dialog_get_state(ua->dialog);
	if (state == SIPS_CONNECTED) {
		char s[128];

		sip_dialog_rtt_stop(ua->dialog);
		ua->ua_set_rtt(ua->dialog->remote_uri.endpoint.host,
			       &(ua->dialog->rtt));

		sip_timer_cancel(&(ua->dialog->timers));

		memset(s, 0, 128);
		sip_uri_gen(&(ua->dialog->remote_uri), s);
		history_add_event(ua, HIST_EVENT_CONNECTED, s);

		if (ua->uas_connect != NULL)
			ua->uas_connect(ua->dialog);

	} else if (state == SIPS_UAS_COMPLETED) {
		sip_user_agent_clear(ua);
		ua->uas_completed();

	} else if (state == SIPS_UAS_ACK_WAIT) {
		sip_timer_cancel(&(ua->dialog->timers));
		sip_dialog_set_state(ua->dialog, SIPS_CONNECTED);

		soundcard_flush(&(ua->soundcard));

		log_msg(LOG_CONNECTION,
			"UAS Reinvite Connected Call-ID [%s]",
			ua->dialog->call_id);
	}
}

static void
sip_recv_options(sip_user_agent_t ua, msglines_t msglines)
{
	int result, state;

	if (ua == NULL || msglines == NULL)
		return;

	result = sip_uas_check_mandatory_options_hdrs(msglines);
	if (result < 0)
		return;

	state = sip_dialog_get_state(ua->dialog);
	if (state == SIPS_IDLE)
		sip_uas_options(ua, msglines);
	else {
		log_msg(LOG_INFO,
			"Cannot handle OPTIONS request now");
		sip_uas_busy_here(ua, msglines);
	}
}

static void
sip_recv_bye(sip_user_agent_t ua, msglines_t msglines)
{
	int result, state;

	if (ua == NULL || msglines == NULL)
		return;

	result = sip_uas_check_mandatory_bye_hdrs(msglines);
	if (result < 0)
		return;

	/* Do some checking on the BYE request message */
	result = sip_uas_check_bye(ua, msglines);
	if (result < 0) {
		log_msg(LOG_INFO, "Check BYE request failed");
		return;
	}
	state = sip_dialog_get_state(ua->dialog);
	if (state == SIPS_CONNECTED)
		sip_uas_hangup(ua, msglines);
}

static void
sip_recv_cancel(sip_user_agent_t ua, msglines_t msglines)
{
	int result, state;

	if (ua == NULL || msglines == NULL)
		return;

	result = sip_uas_check_mandatory_cancel_hdrs(msglines);
	if (result < 0)
		return;

	state = sip_dialog_get_state(ua->dialog);
	if (state == SIPS_UAS_PROCEEDING)
		sip_uas_cancel(ua, msglines);
	else
		sip_uas_call_leg_does_not_exist(ua, msglines);
}

static void
gen_method_log_msg(msglines_t msglines)
{
	char method[BUFSIZE], s[BUFSIZE];
	int pos = 0;

	if (msglines == NULL)
		return;

	memset(method, 0, BUFSIZE);
	memset(s, 0, BUFSIZE);

	nextarg(msglines->msgbuf, &pos, " ", method);
	if (strlen(method) > 0)
		sprintf(s, "Method not allowed (%s)", method);
	else {
		int i;

		for (i = 0; i < msglines->lines; i++)
			if (msglines->len[i] > 0) {
				sprintf(s, "Method not allowed (%d:[%s])", i,
					msglines->msgbuf + msglines->pos[i]);
				break;
			}
		if (i == msglines->lines)
			sprintf(s, "Method not allowed (empty request)");
	}
	log_msg(LOG_WARNING, s);
}

void
sip_recv_request(sip_user_agent_t ua, msglines_t msglines)
{
	int method;

	if (ua == NULL || msglines == NULL)
		return;

	method = msglines_get_method(msglines);
	switch (method) {
	case SIPM_INVITE:
		sip_recv_invite(ua, msglines);
		break;
	case SIPM_ACK:
		sip_recv_ack(ua, msglines);
		break;
	case SIPM_OPTIONS:
		sip_recv_options(ua, msglines);
		break;
	case SIPM_BYE:
		sip_recv_bye(ua, msglines);
		break;
	case SIPM_CANCEL:
		sip_recv_cancel(ua, msglines);
		break;
	default:
		gen_method_log_msg(msglines);
		sip_uas_method_not_allowed(ua, msglines);
	}
}
