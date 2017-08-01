#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "sip.h"

static void
sip_timer_clear(sip_timers_t timers)
{
	if (timers == NULL)
		return;

	timers->timer = SIPT_NULL;
	timerclear(&(timers->a_interval));
	timerclear(&(timers->a_end));
	timerclear(&(timers->b_interval));
	timerclear(&(timers->b_end));
}

static void
sip_timer_increment(sip_timers_t timers)
{
	if (timers == NULL)
		return;

	timeradd(&(timers->a_interval), &(timers->a_interval),
		 &(timers->a_interval));
	timeradd(&(timers->a_end), &(timers->a_interval),
		 &(timers->a_end));

	log_msg(LOG_INFO, "Timer A interval %u:%u",
		timers->a_interval.tv_sec,
		timers->a_interval.tv_usec);
	log_msg(LOG_INFO, "Timer A end %u:%u",
		timers->a_end.tv_sec, timers->a_end.tv_usec);
}

#define SIP_CHECK_TIMERS(MSG, RETRANSMIT)				\
{									\
	/* Check timer B */						\
	if (timercmp(&now, &(timers->b_end), >=)) {			\
		/* Timer B expiration */				\
		log_msg(LOG_INFO, "Timer B expired");			\
		if (ua->uac_timeout != NULL)				\
			ua->uac_timeout();				\
		sip_user_agent_clear(ua);				\
		log_msg(LOG_WARNING, MSG);				\
		return;							\
	}								\
	/* Check timer A */						\
	if (timercmp(&now, &(timers->a_end), >=)) {			\
		/* Timer A expiration */				\
		log_msg(LOG_INFO, "Timer A expired");			\
		sip_timer_increment(timers);				\
		RETRANSMIT;						\
	}								\
}

static void
sip_dialog_timers(sip_user_agent_t ua, sip_dialog_t dialog)
{
	sip_timers_t timers;
	struct timeval now;
	struct timezone tz;

	if (ua == NULL || dialog == NULL)
		return;

	timers = &(dialog->timers);
	if (timers->timer == SIPT_NULL)
		return;

	gettimeofday(&now, &tz);

	switch (timers->timer) {
	case SIPT_INVITE_RETR:
		SIP_CHECK_TIMERS("INVITE retry timeout",
				 sip_uac_retransmit_invite(ua, dialog));
		break;

	case SIPT_RESPONSE_RETR:
		SIP_CHECK_TIMERS("RESPONSE retry timeout",
				 sip_uas_retransmit_response(ua));
		break;

	case SIPT_BYE_RETR:
		SIP_CHECK_TIMERS("BYE retry timeout",
				 sip_uac_retransmit_bye(ua));
		break;

	case SIPT_CANCEL_RETR:
		SIP_CHECK_TIMERS("CANCEL retry timeout",
				 sip_uac_retransmit_cancel(ua));
		break;

	case SIPT_REGISTER_RETR:
		/* Check timer B */
		if (timercmp(&now, &(timers->b_end), >=)) {
			/* Timer B expiration */
			log_msg(LOG_INFO, "Timer B expired");
			if (strlen(ua->registration->reg_uri.endpoint.domain) > 0)
				ua->uac_register_failed(ua->registration->reg_uri.endpoint.domain);
			else
				ua->uac_register_failed(ua->registration->reg_uri.endpoint.host);

			sip_user_agent_clear_registration(ua);
			log_msg(LOG_WARNING, "REGISTER retry timeout");
			return;
		}
		/* Check timer A */
		if (timercmp(&now, &(timers->a_end), >=)) {
			/* Timer A expiration */
			log_msg(LOG_INFO, "Timer A expired");
			sip_timer_increment(timers);
			sip_uac_retransmit_register(ua);
		}
		break;

	default:
		log_msg(LOG_ERROR, "Illegal timer (%d)", timers->timer);
		sip_timer_clear(timers);
	}
}

void
sip_timer_thread_func(sip_user_agent_t ua)
{
	sip_dialog_t dialog;

	if (ua == NULL)
		return;

	/* Check timers for call session dialogs */
	for (dialog = ua->dialog; dialog != NULL; dialog = dialog->next)
		sip_dialog_timers(ua, dialog);

	/* Check timers for the current registration in progress */
	sip_dialog_timers(ua, ua->registration);
}

void
sip_timer_init(sip_dialog_t dialog)
{
	if (dialog == NULL)
		return;

	sip_timer_clear(&(dialog->timers));
}

void
sip_timer_start(sip_timers_t timers, int timer, int duration)
{
	struct timeval now;
	struct timezone tz;

	if (timers == NULL)
		return;

	if (timer > SIPT_NULL && timer <= SIPT_REGISTER_RETR) {
		if (timers->timer != SIPT_NULL)
			sip_timer_cancel(timers);

		timers->timer = timer;

		gettimeofday(&now, &tz);

		/* Set timer B */
		timers->b_interval.tv_sec =
			(64 * duration) / 1000000;
		timers->b_interval.tv_usec =
			(64 * duration) % 1000000;
		timeradd(&now, &(timers->b_interval), &(timers->b_end));

		/* Set timer A */
		timers->a_interval.tv_sec = 0;
		timers->a_interval.tv_usec = duration;
		timeradd(&now, &(timers->a_interval), &(timers->a_end));

		log_msg(LOG_INFO, "Timer B interval %u:%u",
			timers->b_interval.tv_sec,
			timers->b_interval.tv_usec);
		log_msg(LOG_INFO, "Timer B end %u:%u",
			timers->b_end.tv_sec, timers->b_end.tv_usec);
		log_msg(LOG_INFO, "Timer A interval %u:%u",
			timers->a_interval.tv_sec,
			timers->a_interval.tv_usec);
		log_msg(LOG_INFO, "Timer A end %u:%u",
			timers->a_end.tv_sec, timers->a_end.tv_usec);
	}
}

void
sip_timer_cancel(sip_timers_t timers)
{
	if (timers == NULL)
		return;

	sip_timer_clear(timers);
}
