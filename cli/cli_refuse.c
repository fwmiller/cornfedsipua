#include <stdio.h>
#include "cli.h"

void
cli_refuse(char *cmdline)
{
	int state = sip_dialog_get_state(ua.dialog);
	if (state < 0) {
		printf("Bad dialog state\n");
		return;
	}
	if (state != SIPS_UAS_PROCEEDING) {
		printf("No incoming call\n");
		return;
	}
	sip_uas_refuse(&ua);

	printf("Call refused\n");
}
