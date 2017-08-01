#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "sip.h"

void
cli_dial(char *cmdline)
{
	struct sip_uri uri;
	struct timeval rtt;
	char s[BUFSIZE];
	int result, state;

	state = sip_dialog_get_state(ua.dialog);
	if (state < 0) {
		printf("Bad dialog state\n");
		return;
	}
        if (state != SIPS_IDLE) {
		printf("Not idle\n");
		return;
	}
	if (!sip_uri_isset(&(ua.remote_uri))) {
		printf("Remote address not set correctly\n");
		return;
	}
	sip_uri_init(&uri);
	strcpy(uri.endpoint.host, ua.local_endpoint.host);
	uri.endpoint.port = ua.local_endpoint.port;
	uri.prefix = SIPU_SIP;

	result = sip_uac_invite(&ua, &(ua.remote_uri), &uri, &(ua.reg_uri));
	if (result < 0) {
		printf("Call setup failed\n");
		return;
	}
	ua_get_rtt(NULL, &rtt);
	sip_timer_start(&(ua.dialog->timers), SIPT_INVITE_RETR, rtt.tv_usec);

	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(ua.remote_uri), s);
	printf("Calling %s\n", s);
}
