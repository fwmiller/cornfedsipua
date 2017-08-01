#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

int reg_interval = REGISTER_INTERVAL;
int reg_buffer = REGISTER_BUFFER;
int reg_norereg = 0;
int reg_expires = (-1);
struct timeval reg_end;

static void
cli_register_dump_state()
{
	int expires = reg_get_expires(NULL);

	printf("\n");
	printf("user        : [%s]\n", ua.reg_uri.user);
	printf("password    : [%s]\n", ua.reg_uri.passwd);
	printf("domain name : [%s]\n", ua.reg_uri.endpoint.domain);
	printf("host        : [%s]\n", ua.reg_uri.endpoint.host);
	printf("port        : %d\n", ua.reg_uri.endpoint.port);
	printf("interval    : %d\n", reg_interval);
	printf("buffer      : %d\n", reg_buffer);
	printf("re-register : %s\n",
	       (reg_norereg ? "no" : "yes"));
	if (expires < 0)
		printf("expires     : -1\n");
	else {
		int min, sec;

		min = expires / 60;
		sec = expires % 60;

		printf("expires     :");
		if (min > 1)
			printf(" %d minutes", min);
		else if (min == 1)
			printf(" 1 minute");

		if (sec > 1)
			printf(" %d seconds", sec);
		else if (sec == 1)
			printf(" 1 second");

		printf("\n");
	}
	printf("\n");
}

void
cli_register(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 0;

	/* Skip command word */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	/* Get first argument, if any */
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
			memset(ua.reg_uri.user, 0, BUFSIZE);
		else if (strcmp(ua.reg_uri.user, s) != 0) {
			strcpy(ua.reg_uri.user, s);
			ua.reg_uri.prefix = SIPU_SIP;
			reg_set_expires(NULL, -1);
		}
		cli_register_dump_state();

	} else if (strcmp(s, "host") == 0) {
		struct sip_uri uri;
		int result;

		sip_uri_init(&uri);
		result = cli_set_host(cmdline, s, &pos, &uri);
		if (result == 0) {
			if (strlen(uri.endpoint.domain) > 0)
				strcpy(ua.reg_uri.endpoint.domain,
				       uri.endpoint.domain);
			else
				memset(ua.reg_uri.endpoint.domain, 0,
				       BUFSIZE);

			strcpy(ua.reg_uri.endpoint.host, uri.endpoint.host);
			ua.reg_uri.prefix = SIPU_SIP;
			reg_set_expires(NULL, -1);

			cli_register_dump_state();
		}

	} else if (strcmp(s, "port") == 0) {
		int port;

		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		if (strlen(s) == 0) {
			printf("Port number missing\n");
			return;
		}
		port = atoi(s);
		if (ua.reg_uri.endpoint.port != port) {
			ua.reg_uri.endpoint.port = atoi(s);
			ua.reg_uri.prefix = SIPU_SIP;
			reg_set_expires(NULL, -1);
		}
		cli_register_dump_state();

	} else if (strcmp(s, "passwd") == 0) {
		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);

		if (strlen(s) == 0)
			memset(ua.reg_uri.passwd, 0, BUFSIZE);
		else {
			strcpy(ua.reg_uri.passwd, s);
			ua.reg_uri.prefix = SIPU_SIP;
		}
		cli_register_dump_state();

	} else if (strcmp(s, "send") == 0) {
		int state = sip_dialog_get_state(ua.registration);
		if (state < 0) {
			printf("Bad dialog state\n");
			return;
		}
		if (state != SIPS_IDLE) {
			printf("Not idle\n");
			return;
		}
		if (!sip_uri_isset(&(ua.reg_uri))) {
			printf("Register address not set correctly\n");
			return;
		}
		/* reg_set_expires(NULL, reg_interval); */
		sip_uac_register(&ua);
		sip_timer_start(&(ua.registration->timers),
				SIPT_REGISTER_RETR, 1000 * SIP_TIMER_1);

		printf("Registration request sent\n");

	} else if (strcmp(s, "remove") == 0) {
		int state = sip_dialog_get_state(ua.dialog);
		if (state < 0) {
			printf("Bad dialog state\n");
			return;
		}
		if (state != SIPS_IDLE && state != SIPS_CONNECTED) {
			printf("Transaction in progress\n");
			return;
		}
		reg_set_expires(NULL, 0);
		sip_uac_unregister(&ua);
		sip_timer_start(&(ua.registration->timers),
				SIPT_REGISTER_RETR, 1000 * SIP_TIMER_1);

		printf("Registration removal request sent\n");

	} else if (strcmp(s, "interval") == 0) {
		int interval;

		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		if (strlen(s) == 0) {
			printf("Interval missing\n");
			return;
		}
		interval = atoi(s);
		if (interval < 0) {
			printf("Illegal interval\n");
			return;
		}
		if (interval < reg_buffer + 5)
			printf("Interval must be at least 5 seconds greater than the buffer\n");
		else
			reg_interval = interval;

		cli_register_dump_state();

	} else if (strcmp(s, "buffer") == 0) {
		int buffer;

		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		if (strlen(s) == 0) {
			printf("Buffer missing\n");
			return;
		}
		buffer = atoi(s);
		if (buffer < 0) {
			printf("Illegal buffer\n");
			return;
		}
		if (buffer > reg_interval - 5)
			printf("Buffer must be at least 5 seconds less than the interval\n");
		else
			reg_buffer = buffer;

		cli_register_dump_state();

	} else if (strcmp(s, "rereg") == 0) {
		reg_norereg = 0;
		cli_register_dump_state();

	} else if (strcmp(s, "norereg") == 0) {
		reg_norereg = 1;
		cli_register_dump_state();

	} else if (strlen(s) == 0)
		cli_register_dump_state();

	else
		cli_help("help register");
}
