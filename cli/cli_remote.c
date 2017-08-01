#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

static void
cli_remote_dump_state()
{
	printf("\n");
	printf("user        : [%s]\n", ua.remote_uri.user);
	printf("domain name : [%s]\n", ua.remote_uri.endpoint.domain);
	printf("host        : [%s]\n", ua.remote_uri.endpoint.host);
	printf("port        : %d\n", ua.remote_uri.endpoint.port);
	printf("\n");
}

void
cli_remote(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 6;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "user") == 0) {
		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		if (strlen(s) == 0) {
			printf("User name missing\n");
			return;
		}
		if (strcmp(s, "clear") == 0)
			memset(ua.remote_uri.user, 0, BUFSIZE);
		else {
			strcpy(ua.remote_uri.user, s);
			ua.remote_uri.prefix = SIPU_SIP;
		}
		cli_remote_dump_state();

	} else if (strcmp(s, "host") == 0) {
		struct sip_uri uri;
		int result;

		sip_uri_init(&uri);
		result = cli_set_host(cmdline, s, &pos, &uri);
		if (result == 0) {
			if (strlen(uri.endpoint.domain) > 0)
				strcpy(ua.remote_uri.endpoint.domain,
				       uri.endpoint.domain);
			else
				memset(ua.remote_uri.endpoint.domain, 0,
				       BUFSIZE);

			strcpy(ua.remote_uri.endpoint.host,
			       uri.endpoint.host);
			ua.remote_uri.prefix = SIPU_SIP;

			cli_remote_dump_state();
		}

	} else if (strcmp(s, "port") == 0) {
		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		if (strlen(s) == 0) {
			printf("Port number missing\n");
			return;
		}
		ua.remote_uri.endpoint.port = atoi(s);
		ua.remote_uri.prefix = SIPU_SIP;

		cli_remote_dump_state();

	} else if (strlen(s) == 0)
		cli_remote_dump_state();

	else
		cli_help("help remote");
}
