#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "dns.h"
#include "http.h"
#include "lex.h"
#include "readline.h"
#include "history.h"

struct cli_cmd {
	char *cmd;
	void (*f) (char *);
};

static struct cli_cmd cmds[] = {
	{"about", cli_about },
	{"ans", cli_answer },
	{"answer", cli_answer },
	{"debug", cli_debug },
	{"dial", cli_dial },
	{"dialog", cli_dialog },
	{"dnd", cli_dnd },
	{"dns", cli_dns },
	{"hang", cli_hangup },
	{"hangup", cli_hangup },
	{"help", cli_help },
	{"hist", cli_history },
	{"history", cli_history },
	{"local", cli_local },
	{"log", cli_log },
	{"nat", cli_nat },
	{"out", cli_outbound_proxy },
	{"outbound", cli_outbound_proxy },
	{"play", cli_play_wav },
	{"sound", cli_soundcard },
	{"soundcard", cli_soundcard },
	{"rec", cli_record_wav },
	{"record", cli_record_wav },
	{"reg", cli_register },
	{"register", cli_register },
	{"refuse", cli_refuse },
	{"remote", cli_remote },
	{"reset", cli_reset },
	{"ring", cli_ringtone },
	{"ringtone", cli_ringtone },
	{"rtp", cli_rtp },
	{"vol", cli_volume },
	{"volume", cli_volume },
	{"wav", cli_wav },
	{"0123456789ABCD#*", NULL }
};

static void
cli_usage()
{
	cli_help(NULL);
}

int
cli_set_host(char *cmdline, char *s, int *pos, sip_uri_t uri)
{
	char host[128];
	char *ipaddr;
	int ipaddrlen, result;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	if (strlen(s) == 0) {
		printf("Host address missing\n");
		return (-1);
	}
	result = find_ip_address(s, BUFSIZE, &ipaddr, &ipaddrlen);
	if (result == 0) {
		/* Found IP address */
		memset(uri->endpoint.domain, 0, BUFSIZE);
		memset(uri->endpoint.host, 0, BUFSIZE);
		strncpy(uri->endpoint.host, ipaddr, ipaddrlen);

		return 0;
	}
	if (!dns_avail(&ua)) {
		printf("DNS lookups not available\n");
		return (-1);
	}
	result = dns_gethostbyname(s, host, 128);
	if (result == 0) {
		memset(uri->endpoint.domain, 0, BUFSIZE);
		memset(uri->endpoint.host, 0, BUFSIZE);

		strcpy(uri->endpoint.domain, s);
		strcpy(uri->endpoint.host, host);
#if 0
		printf("\n");
		printf("domain name : [%s]\n", uri->endpoint.domain);
		printf("host        : [%s]\n", uri->endpoint.host);
		printf("\n");
#endif
		return 0;
	}
	printf("No host ip address for domain name\n");
	return (-1);
}

void
cli()
{
	char *cmdline = NULL;
	char cmd[BUFSIZE];
	int i, ncmds, pos;

	ncmds = sizeof(cmds) / sizeof(struct cli_cmd);

	for (;;) {
	
		if (cmdline != NULL) {
			free(cmdline);
			cmdline = NULL;
		}
		cmdline = readline(PROMPT);
		if (cmdline == NULL)
			continue;

		if (strlen(cmdline) == 0)
			continue;

		if (cli_dtmf_scan(cmdline) == 0) {
			cli_dtmf_send(cmdline);
			continue;
		}
		if (strcmp(cmdline, "quit") == 0)
			break;

		if (cmdline[0] == '!') {
			if (cmdline[1] == '!')
				cli_cmd_history_substitute(0, cmdline);

			else if (isdigit(cmdline[1]))
				cli_cmd_history_substitute(atoi(cmdline + 1),
								cmdline);

			else {
				cli_cmd_history_dump();
				continue;
			}
		}
		memset(cmd, 0, BUFSIZE);
		pos = 0;
		nextarg(cmdline, &pos, " ", cmd);

		for (i = 0; i < ncmds; i++)
			if (strcmp(cmd, cmds[i].cmd) == 0) {
				if (cmds[i].f != NULL)
					(*(cmds[i].f)) (cmdline);

				cli_cmd_history_push(cmdline);
				break;
			}
		if (i == ncmds)
			cli_usage();
	}
}
