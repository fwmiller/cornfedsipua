#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

static void
dump_soundcard_status(struct audio_buf_info *iinfo,
		      struct audio_buf_info *oinfo)
{
	printf("Soundcard input buffer\n");
	printf("Avail fragments : %d\n", iinfo->fragments);
	printf("Total fragments : %d\n", iinfo->fragstotal);
	printf("Fragment size   : %d\n", iinfo->fragsize);
	printf("Avail bytes     : %d\n", iinfo->bytes);
	printf("Soundcard output buffer\n");
	printf("Avail fragments : %d\n", oinfo->fragments);
	printf("Total fragments : %d\n", oinfo->fragstotal);
	printf("Fragment size   : %d\n", oinfo->fragsize);
	printf("Avail bytes     : %d\n", oinfo->bytes);
}

static void
cli_soundcard_dump_state()
{
	struct audio_buf_info iinfo, oinfo;
	int result;

	printf("\n");
	printf("Soundcard device [%s]\n", ua.soundcard.device);
	result = soundcard_status(&(ua.soundcard), &iinfo, &oinfo);
	if (result < 0)
		printf("Get soundcard status failed\n");
	else
		dump_soundcard_status(&iinfo, &oinfo);

	printf("\n");
}

void
cli_soundcard(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 0;

	/* Skip command word */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	/* Get first argument, if any */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "device") == 0) {
		char current_soundcard[BUFSIZE];
		int result;

		memset(current_soundcard, 0, BUFSIZE);
		strcpy(current_soundcard, ua.soundcard.device);
		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);

		result = soundcard_init(&ua, s);

		if (result < 0) {
			printf("Set soundcard device to [%s] failed\n", s);

			result = soundcard_init(&ua, current_soundcard);

			if (result < 0) {
				printf("Return soundcard device to ");
				printf("[%s] failed\n", current_soundcard);
				return;
			}
		}
		cli_soundcard_dump_state();

	} else if (strcmp(s, "flush") == 0) {

		soundcard_flush(&(ua.soundcard));

	} else if (strlen(s) == 0)
		cli_soundcard_dump_state();
	else
		cli_help("help soundcard");
}
