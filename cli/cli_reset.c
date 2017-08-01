#include <stdio.h>
#include "cli.h"

void
cli_reset(char *cmdline)
{
	sip_user_agent_clear(&ua);
	sip_dialog_init(ua.registration);

	printf("Client reset\n");
}
