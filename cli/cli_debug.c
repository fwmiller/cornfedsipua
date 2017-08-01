#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

static void
cli_debug_dump_state()
{
	printf("Debugging is %s\n", (ua.flags & SUAF_DEBUG ? "on" : "off"));
}

void
cli_debug(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 5;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "on") == 0) {
		ua.flags |= SUAF_DEBUG;
		cli_debug_dump_state();

	} else if (strcmp(s, "off") == 0) {
		ua.flags &= ~SUAF_DEBUG;
		cli_debug_dump_state();

	} else if (strlen(s) == 0)
		cli_debug_dump_state();

	else
		cli_help("help debug");
}
