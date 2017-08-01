#include <stdio.h>
#include "cli.h"

void
cli_hangup(char *cmdline)
{
	int result, state;

	state = sip_dialog_get_state(ua.dialog);
	if (state < 0) {
		printf("Bad dialog state\n");
		return;
	}
	if (state == SIPS_UAC_CALLING) {
		printf("No provisional response received yet\n");
		return;
	}
	if (state == SIPS_UAC_PROCEEDING) {
		result = sip_uac_cancel(&ua);
		if (result < 0)
			printf("Cancel failed\n");
		else
			printf("Canceled\n");
		return;
	}
	if (state != SIPS_CONNECTED) {
		printf("Not connected\n");
		return;
	}
	result = sip_uac_hangup(&ua);
	if (result < 0) {
		printf("Disconnect failed\n");
		return;
	}
	printf("Disconnected\n");
}
