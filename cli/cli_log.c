#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "lex.h"
#include "log.h"

static void
cli_log_dump_state()
{
	printf("Log level ");
	switch (log_get_level()) {
	case LOG_ERROR:
		printf("error");
		break;
	case LOG_WARNING:
		printf("warning");
		break;
	case LOG_CONNECTION:
		printf("connection");
		break;
	case LOG_EVENT:
		printf("event");
		break;
	case LOG_INFO:
		printf("info");
		break;
	default:
		printf("ILLEGAL");
	}
	printf("\n");
}

void
cli_log(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 3;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "info") == 0) {
		log_set_level(LOG_INFO);
		cli_log_dump_state();

	} else if (strcmp(s, "event") == 0) {
		log_set_level(LOG_EVENT);
		cli_log_dump_state();

	} else if (strcmp(s, "connection") == 0 || strcmp(s, "conn") == 0) {
		log_set_level(LOG_CONNECTION);
		cli_log_dump_state();

	} else if (strcmp(s, "warning") == 0 || strcmp(s, "warn") == 0) {
		log_set_level(LOG_WARNING);
		cli_log_dump_state();

	} else if (strcmp(s, "error") == 0) {
		log_set_level(LOG_ERROR);
		cli_log_dump_state();

	} else if (strlen(s) == 0)
		cli_log_dump_state();

	else
		cli_help("help log");
}
