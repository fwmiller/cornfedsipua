#include <stdio.h>
#include <string.h>
#include "http.h"
#include "log.h"
#include "sip.h"

extern struct sip_user_agent ua;

static int log_level = LOG_INFO;

void
log_msg(int level, const char *format, ...)
{
	time_t now;
	char *timestr;
	va_list args;
	char s[BUFSIZE];
	int i;

	if (level < 0 || level > log_level)
		return;

	memset(s, 0, BUFSIZE);

	now = time(NULL);
	timestr = ctime(&now);
	for (i = 4; i < 19; i++)
		sprintf(s + strlen(s), "%c", timestr[i]);

	sprintf(s + strlen(s), " <%d.%d.%dG>", CORNFEDSIPUA_VERSION,
		MAJOR_RELEASE, MINOR_RELEASE);

	switch (level) {
	case LOG_ERROR:
		sprintf(s + strlen(s), " <<ERROR>> ");
		break;
	case LOG_WARNING:
		sprintf(s + strlen(s), " <WARNING> ");
		break;
	default:
		sprintf(s + strlen(s), " ");
	}
	va_start(args, format);
	vsprintf(s + strlen(s), format, args);
	sprintf(s + strlen(s), "\n");
#if 0
	if (ua.flags & SUAF_DEBUG)
		printf(s);
#endif
	printf(s);

	if (level < LOG_INFO) {
		char s1[BUFSIZE];
		int j;

		memset(s1, 0, BUFSIZE);
		for (i = 0, j = 0; i < strlen(s); i++)
			if (s[i] == ' ' || s[i] == '\r' || s[i] == '\n') {
				s1[j++] = '%';
				s1[j++] = '2';
				s1[j++] = '0';
			} else
				s1[j++] = s[i];

		http_log(&ua, s1);
	}
}

int
log_get_level()
{
	return log_level;
}
 
void
log_set_level(int level)
{
	if (level < 0 || level > LOG_INFO)
		return;
 
	log_level = level;
}
