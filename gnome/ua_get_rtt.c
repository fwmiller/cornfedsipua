#include "gui.h"
#include "log.h"

void
ua_get_rtt(char *host, struct timeval *rtt)
{
	if (rtt == NULL)
		return;

	rtt->tv_sec = 0;
	rtt->tv_usec = ua.rtt.tv_usec;
}
