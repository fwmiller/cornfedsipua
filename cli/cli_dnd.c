#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

static void
cli_dnd_dump_state()
{
	int state = sip_dialog_get_state(ua.dialog);
	printf("Do not disturb is %s\n",
	       (state == SIPS_BUSY ? "on" : "off"));
}

void
cli_dnd(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 3;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "on") == 0) {
		sip_user_agent_set_do_not_disturb(&ua);
		cli_dnd_dump_state();

	} else if (strcmp(s, "off") == 0) {
		sip_user_agent_clear_do_not_disturb(&ua);
		cli_dnd_dump_state();

	} else if (strlen(s) == 0)
		cli_dnd_dump_state();

	else
		cli_help("help dnd");
}
