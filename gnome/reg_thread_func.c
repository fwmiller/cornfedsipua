#include "gui.h"
#include "log.h"

void
reg_thread_func()
{
	struct timeval now, dur;
	struct timezone tz;

	if (reg_expires <= 0)
		return;

	gettimeofday(&now, &tz);
	timersub(&reg_end, &now, &dur);
	reg_expires = dur.tv_sec;

	if (reg_expires < REGISTER_BUFFER) {
		char s[128];

		memset(s, 0, 128);
		sprintf(s, "Re-registering with %s",
			(strlen(ua.reg_uri.endpoint.domain) > 0 ?
			 ua.reg_uri.endpoint.domain :
			 ua.reg_uri.endpoint.host));

		log_msg(LOG_INFO, s);
		status(s);

		reg_set_expires(NULL, REGISTER_INTERVAL);
		sip_uac_register(&ua);
		sip_timer_start(&(ua.registration->timers),
				SIPT_REGISTER_RETR, 1000 * SIP_TIMER_1);
	}
}
