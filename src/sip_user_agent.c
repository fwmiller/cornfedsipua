#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "sip.h"

void
sip_user_agent_init(sip_user_agent_t ua)
{
	if (ua == NULL)
		return;

	ua->flags = 0;
	memset(ua->cornfeddir, 0, BUFSIZE);
	memset(ua->configfile, 0, BUFSIZE);
	memset(ua->if_name, 0, BUFSIZE);
	ua->sipfd = (-1);
	memset(ua->local_netmask, 0, BUFSIZE);
	ipendpoint_init(&(ua->local_endpoint));
	ipendpoint_init(&(ua->stun_server));
	ipendpoint_init(&(ua->visible_endpoint));
	ipendpoint_init(&(ua->outbound_proxy));
	sip_uri_init(&(ua->remote_uri));
	sip_uri_init(&(ua->reg_uri));
	ua->rtt.tv_sec = 0;
	ua->rtt.tv_usec = 1000 * SIP_TIMER_1;
	ua->stun_keepalive_interval.tv_sec = 10;
	ua->stun_keepalive_interval.tv_usec = 0;
	timerclear(&(ua->stun_keepalive_end));
	ua->dialog = sip_dialog_pop();
	sip_dialog_init(ua->dialog);
	ua->registration = sip_dialog_pop();
	sip_dialog_init(ua->registration);
	ua->uac_canceled = NULL;
	ua->uac_connect = NULL;
	ua->uac_completed = NULL;
	ua->uac_register_prompt_for_user = NULL;
	ua->uac_register_failed = NULL;
	ua->uac_timeout = NULL;
	ua->uas_request_uri_lookup = NULL;
	ua->uas_cancel = NULL;
	ua->uas_ringback = NULL;
	ua->uas_completed = NULL;
	ua->uas_connect = NULL;
	ua->uas_hangup = NULL;
	ua->reg_get_interval = NULL;
	ua->reg_get_expires = NULL;
	ua->reg_set_expires = NULL;
	ua->reg_get_auth_user = NULL;
	ua->reg_thread_func = NULL;
	ua->ua_history_update = NULL;
	ua->ua_history_clear = NULL;
	timerclear(&(ua->ringing_interval));
	ua->rtp_port = (-1);
	rtp_session_clear(&(ua->rtp));
	rtp_stats_clear_all(&(ua->rtp.stats));

	memset(&(ua->soundcard), 0, sizeof(struct soundcard));

	memset(&(ua->ringtone), 0, sizeof(struct ringtone));
	memset(ua->record_file, 0, BUFSIZE);
	ua->recordfd = (-1);
	ua->record_cnt = 0;
	wav_rec_list_init(&(ua->wavs));
	timerclear(&(ua->wav_end));
}

void
sip_user_agent_clear(sip_user_agent_t ua)
{
	sip_dialog_t dialog;

	if (ua == NULL)
		return;

	if (ua->flags & SUAF_RINGING)
		ua->flags &= ~SUAF_RINGING;
	if (ua->flags & SUAF_RECORDING)
		ua->flags &= ~SUAF_RECORDING;

	timerclear(&(ua->ringing_interval));

	for (;;) {
		if (ua->dialog->next == NULL)
			break;

		dialog = ua->dialog;
		sip_dialog_list_remove(dialog, &(ua->dialog));
		sip_dialog_push(dialog);
	}
	sip_dialog_init(ua->dialog);
	wav_rec_list_flush(ua);
	rtp_endpoint_init(&(ua->rtp.remote));
	ua->rtp.codec = (-1);

	soundcard_flush(&(ua->soundcard));

}

void
sip_user_agent_clear_registration(sip_user_agent_t ua)
{
	if (ua == NULL)
		return;

	sip_dialog_init(ua->registration);
}

void
sip_user_agent_clear_do_not_disturb(sip_user_agent_t ua)
{
	int state;

	if (ua == NULL)
		return;

	state = sip_dialog_get_state(ua->dialog);
	if (state != SIPS_BUSY)
		return;

	sip_dialog_set_state(ua->dialog, SIPS_IDLE);
}

void
sip_user_agent_set_do_not_disturb(sip_user_agent_t ua)
{
	int state;

	if (ua == NULL)
		return;

	state = sip_dialog_get_state(ua->dialog);
	if (state != SIPS_IDLE)
		return;

	sip_dialog_set_state(ua->dialog, SIPS_BUSY);
}
