#include <string.h>
#include "cli.h"
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

	if (reg_norereg) {
		if (reg_expires < reg_buffer - 2) {
			reg_expires = (-1);
			timerclear(&reg_end);
		}
		return;
	}
	if (reg_expires < reg_buffer) {
		log_msg(LOG_INFO, "Re-registering with %s",
			(strlen(ua.reg_uri.endpoint.domain) > 0 ?
			 ua.reg_uri.endpoint.domain :
			 ua.reg_uri.endpoint.host));
		reg_set_expires(NULL, reg_interval);
		sip_uac_register(&ua);
		sip_timer_start(&(ua.registration->timers),
				SIPT_REGISTER_RETR, 1000 * SIP_TIMER_1);
	}
}
