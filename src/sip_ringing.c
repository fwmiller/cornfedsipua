#include <string.h>
#include <unistd.h>
#include "log.h"
#include "sip.h"

#define SIP_RINGING_INTERVAL	3      /* seconds */

void
sip_ringing_thread_func(sip_user_agent_t ua)
{
	struct timeval interval, now;
	struct timezone tz;

	if (ua == NULL || !(ua->flags & SUAF_RINGING))
		return;

	gettimeofday(&now, &tz);
	if (timercmp(&now, &(ua->ringing_interval), <))
		return;

	ua->uas_ringback(ua->dialog);

	interval.tv_sec = SIP_RINGING_INTERVAL;
	interval.tv_usec = 0;
	timeradd(&(ua->ringing_interval), &interval,
		 &(ua->ringing_interval));
}

void
sip_ringing_start(sip_user_agent_t ua)
{
	struct timezone tz;

	if (ua == NULL)
		return;

	gettimeofday(&(ua->ringing_interval), &tz);
	ua->flags |= SUAF_RINGING;
}

void
sip_ringing_stop(sip_user_agent_t ua)
{
	if (ua == NULL)
		return;

	ua->flags &= ~SUAF_RINGING;
	timerclear(&(ua->ringing_interval));
}

void
sip_ringing_answer(sip_user_agent_t ua)
{
	if (ua == NULL)
		return;

	ua->flags &= ~SUAF_RINGING;
	timerclear(&(ua->ringing_interval));

	log_msg(LOG_INFO, "Call answered");
}
