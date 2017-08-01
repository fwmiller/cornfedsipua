#include "gui.h"
#include "log.h"

void
ua_set_rtt(char *host, struct timeval *rtt)
{
	if (rtt == NULL)
		return;

	ua.rtt.tv_sec = 0;
	if (rtt->tv_sec > 0 || rtt->tv_usec > 1000 * SIP_TIMER_1)
		ua.rtt.tv_usec = 1000 * SIP_TIMER_1;
	else
		ua.rtt.tv_usec = rtt->tv_usec;
}
