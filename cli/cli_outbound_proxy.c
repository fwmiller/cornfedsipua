#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

static void
cli_outbound_proxy_dump_state()
{
	printf("\n");
	printf("outbound proxy host : [%s]\n", ua.outbound_proxy.host);
	printf("outbound proxy port : %d\n", ua.outbound_proxy.port);
	printf("\n");
}

void
cli_outbound_proxy(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 0;

	/* Skip command word */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	/* Get first argument, if any */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "host") == 0) {
		struct sip_uri uri;
		char s1[128];
		int tpos = pos;

		memset(s1, 0, 128);
		nextarg(cmdline, &tpos, "", s1);
		if (strcasecmp(s1, "clear") == 0)
			memset(ua.outbound_proxy.host, 0, BUFSIZE);
		else {
			int result = cli_set_host(cmdline, s, &pos, &uri);
			if (result == 0 && strlen(uri.endpoint.host) > 0)
				strcpy(ua.outbound_proxy.host,
				       uri.endpoint.host);
		}
		cli_outbound_proxy_dump_state();

	} else if (strcmp(s, "port") == 0) {
		char s1[128];
		int tpos = pos;

		memset(s1, 0, 128);
		nextarg(cmdline, &tpos, "", s1);
		if (strcasecmp(s1, "clear") == 0)
			ua.outbound_proxy.port = (-1);
		else {
			memset(s, 0, BUFSIZE);
			nextarg(cmdline, &pos, " ", s);
			if (strlen(s) == 0)
				printf("Port number missing\n");
			else {
				ua.outbound_proxy.port = atoi(s);
			}
		}
		cli_outbound_proxy_dump_state();

	} else if (strlen(s) == 0)
		cli_outbound_proxy_dump_state();

	else
		cli_help("help outbound");
}
