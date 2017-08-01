#include "gui.h"

void
reg_set_expires(char *host, int expires)
{
	struct timeval now, dur;
	struct timezone tz;
	char s[128];
	int min, sec;

	reg_expires = expires;

	if (expires <= 0) {
		timerclear(&reg_end);
		return;
	}
	min = expires / 60;
	sec = expires % 60;
	memset(s, 0, 128);
	sprintf(s, "Registration expires in ");
	if (min > 0)
		sprintf(s + strlen(s), "%d min ", min);
	if (sec > 0)
		sprintf(s + strlen(s), "%d sec", sec);
	status(s);

	dur.tv_sec = expires + 1;
	dur.tv_usec = 0;
	gettimeofday(&now, &tz);
	timeradd(&now, &dur, &reg_end);
}
