#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "http.h"
#include "log.h"

static int log_level = LOG_INFO;

void
log_msg(int level, const char *format, ...)
{
	time_t now;
	char *timestr;
	va_list args;
	char s[BUFSIZE];
#if 0
	char s1[BUFSIZE];
#endif
	int i;
#if 0
	int j;
#endif
	if (level < 0 || level > log_level)
		return;

	memset(s, 0, BUFSIZE);

	/* Generate log entry timestamp */
	now = time(NULL);
	timestr = ctime(&now);
	for (i = 4; i < 19; i++)
		sprintf(s + strlen(s), "%c", timestr[i]);

	/* Generate version information */
	sprintf(s + strlen(s), " <%d.%d.%dc>", CORNFEDSIPUA_VERSION,
		MAJOR_RELEASE, MINOR_RELEASE);

	/* Generate level emphasis message if appropriate */
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
	/* Generate log message */
	va_start(args, format);
	vsprintf(s + strlen(s), format, args);
	sprintf(s + strlen(s), "\n");

	/* Send log message to console */
	if (ua.flags & SUAF_DEBUG)
		printf(s);
#if 0
	/* Send log message to Cornfed Systems web server */
	if (level < LOG_INFO) {
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
#endif
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
