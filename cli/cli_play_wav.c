#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

void
cli_play_wav(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 4, result;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strlen(s) > 0) {
		/* Queue the file to be played out */
		result = wav_play(&ua, s);
		if (result < 0) {
			printf("Play [%s] failed\n", s);
			return;
		}
		printf("Queued [%s] for playback\n", s);
	}
}
