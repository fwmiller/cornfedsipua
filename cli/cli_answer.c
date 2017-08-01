#include <stdio.h>
#include "cli.h"

void
cli_answer(char *cmdline)
{	
	int result, state;

	state = sip_dialog_get_state(ua.dialog);
	if (state < 0) {
		printf("Bad dialog state\n");
		return;
	}
	if (state != SIPS_UAS_PROCEEDING) {
		printf("No incoming call\n");
		return;
	}
	result = sip_uas_answer(&ua);
	if (result < 0) {
		printf("Connection failed\n");
		return;
	}
	printf("Connected\n");
}
