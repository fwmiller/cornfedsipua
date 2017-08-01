#include <time.h>
#include <stdio.h>
#include <string.h>
#include "history.h"

void
cli_history(char *cmdline)
{
	history_event_t e;
	char s[128];
	int cnt = 0;

	for (e = history_first_event();
	     e != NULL;
	     e = history_next_event(e)) {
		if (cnt++ == 0)
			printf("\n");

		memset(s, 0, 128);
		history_dump_event(e, s);
		printf("%s\n", s);
	}
	if (cnt > 0)
		printf("\n");
}
