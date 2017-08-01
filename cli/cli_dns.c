#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "dns.h"
#include "lex.h"

void
cli_dns(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 3;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strlen(s) == 0)
		printf("DNS lookups are %s\n",
		       (dns_avail(&ua) ? "on" : "off"));
	else
		cli_help("help dns");
}
