#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "dns.h"
#include "http.h"
#include "lex.h"

static void
cli_nat_dump_state()
{
	printf("\n");
	printf("domain name : [%s]\n", ua.stun_server.domain);
	printf("host        : [%s]\n", ua.stun_server.host);
	printf("nat         : %s\n",
	       (ua.flags & SUAF_LOCAL_NAT ? "on" : "off"));
	printf("\n");
}

static int
cli_set_stun_server(char *stun_server)
{
	char *ipaddr;
	int ipaddrlen, result;

	result = find_ip_address(stun_server, BUFSIZE, &ipaddr, &ipaddrlen);
	if (result == 0)
		strncpy(ua.stun_server.host, ipaddr, ipaddrlen);
	else {
		char ipaddr[128];

		if (!dns_avail(&ua)) {
			printf("DNS not available");
			return (-1);
		}
		result = dns_gethostbyname(stun_server, ipaddr, 128);
		if (result < 0) {
			printf("DNS lookup failed\n");
			return (-1);
		}
		strcpy(ua.stun_server.domain, stun_server);
		strcpy(ua.stun_server.host, ipaddr);
	}
	return 0;
}

void
cli_nat(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 3, result;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "on") == 0) {
		ua.flags |= SUAF_LOCAL_NAT;
		cli_nat_dump_state();

	} else if (strcmp(s, "off") == 0) {
		ua.flags &= ~SUAF_LOCAL_NAT;
		cli_nat_dump_state();

	} else if (strcmp(s, "stun") == 0) {
		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		if (strlen(s) == 0)
			printf("STUN server host missing\n");
		else {
			result = cli_set_stun_server(s);
			if (result == 0)
				cli_nat_dump_state();
		}

	} else if (strlen(s) == 0)
		cli_nat_dump_state();

	else
		cli_help("help nat");
}
