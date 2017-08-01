#include <stdio.h>
#include <string.h>
#include "cli.h"

static char *sip_states[] = {
	"SIPS_IDLE",
	"SIPS_BUSY",
	"SIPS_UAC_REGISTERING",
	"SIPS_UAC_CALLING",
	"SIPS_UAC_PROCEEDING",
	"SIPS_UAC_CANCELING",
	"SIPS_UAC_TRYING",
	"SIPS_UAS_PROCEEDING",
	"SIPS_UAS_COMPLETED",
	"SIPS_UAS_ACK_WAIT",
	"SIPS_CONNECTED"
};

void
cli_dump_dialog(sip_dialog_t dialog)
{
	char s[BUFSIZE];
	int i, state;

	state = sip_dialog_get_state(dialog);
	if (state < 0) {
		printf("Bad dialog state\n");
		return;
	}
	printf("state         : %s\n", sip_states[state]);

	printf("route_set     :\n");
	for (i = 0; i < dialog->route_set.routes; i++) {
		int j;

		printf("[");
		for (j = 0; j < dialog->route_set.len[i]; j++)
			printf("%c", *(dialog->route_set.buf +
			       dialog->route_set.pos[i] + j));
		printf("]\n");
	}

	printf("via_hdrs      :\n");
	if (dialog->via_hdrs != NULL) {
		sip_via_t via;

		for (via = dialog->via_hdrs; via != NULL; via = via->next) {
			printf("host [%s] port %d branch [%s] transport ",
			       via->endpoint.host, via->endpoint.port,
			       via->branch);

			switch (via->transport) {
			case SIPUT_UDP:
				printf("UDP");
				break;
			case SIPUT_TCP:
				printf("TCP");
				break;
			case SIPUT_TLS:
				printf("TLS");
				break;
			default:
				printf("UNDEFINED");
			}
			printf("\n");
		}
	}
	printf("call_id       : [%s]\n", dialog->call_id);
	printf("local_tag     : [%s]\n", dialog->local_tag);
	printf("remote_tag    : [%s]\n", dialog->remote_tag);
	printf("local_seq     : %d\n", dialog->local_seq);
	printf("remote_seq    : %d\n", dialog->remote_seq);

	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->local_uri), s);
	printf("local_uri     : [%s]\n", s);

	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->remote_uri), s);
	printf("remote_uri    : [%s]\n", s);

	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->reg_uri), s);
	printf("reg_uri       : [%s]\n", s);

	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->remote_target), s);
	printf("remote_target : [%s]\n", s);

	printf("authorization : [%s]\n", dialog->authorization);

	printf("last_resp     :\n[%s]\n", dialog->last_resp);
	printf("\n");
}

void
cli_dialog(char *cmdline)
{
	sip_dialog_t dialog;
	int dialogcnt = 1;

	/* Sessions */
	for (dialog = ua.dialog; dialog != NULL; dialog = dialog->next) {
		printf("\n<< Dialog %d >>\n", dialogcnt++);
		cli_dump_dialog(dialog);
	}
	/* Registration */
	printf("<< Registration >>\n");
	cli_dump_dialog(ua.registration);
}
