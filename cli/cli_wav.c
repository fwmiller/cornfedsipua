#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

void
cli_wav(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 3;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "flush") == 0)
		wav_rec_list_flush(&ua);

	else if (strlen(s) == 0)
		wav_dump(&ua);

	else
		cli_help("help wav");
}
