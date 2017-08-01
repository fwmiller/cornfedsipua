#include <stdio.h>
#include <string.h>

#include "cli.h"

void
reg_set_expires(char *host, int expires)
{
	struct timeval now, dur;
	struct timezone tz;
	char s[128];

	reg_expires = expires;

	if (expires <= 0) {
		timerclear(&reg_end);
		return;
	}
	dur.tv_sec = expires + 1;
	dur.tv_usec = 0;
	gettimeofday(&now, &tz);
	timeradd(&now, &dur, &reg_end);

	memset(s, 0, 128);
	sprintf(s, "Registration expires in");

	if (expires > 120)
		sprintf(s + strlen(s), " %d minutes",
			(int) (expires / 60));
	else if (expires >= 60)
		sprintf(s + strlen(s), " 1 minute");

	if ((expires % 60) > 1)
		sprintf(s + strlen(s), " %d seconds",
			(int) (expires % 60));
	else if ((expires % 60) == 1)
		sprintf(s + strlen(s), " 1 second");

	printf("%s\n", s);
}
