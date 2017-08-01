#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "cli.h"
#include "lex.h"

static void
cli_ringtone_dump_state()
{
	printf("\n");
	printf("Ringtone device [%s]\n", ua.ringtone.device);
	printf("Ringtone .wav file [%s]\n", ua.ringtone.file);
	printf("\n");
}

void
cli_ringtone(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 0;

	/* Skip command word */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	/* Get first argument, if any */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "file") == 0) {
		char current_ringtone_file[BUFSIZE];
		int result;

		memset(current_ringtone_file, 0, BUFSIZE);
		strcpy(current_ringtone_file, ua.ringtone.file);

		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		result = ringtone_file_init(&(ua.ringtone),
					    (const char *) s);
		if (result < 0) {
			printf("Set ringtone file to [%s] failed\n", s);

			result = ringtone_file_init(
				&(ua.ringtone),
				(const char *) current_ringtone_file);
			if (result < 0) {
				printf("Return ringtone file to ");
				printf("[%s] failed\n",
				       current_ringtone_file);
				return;
			}
		}
		cli_ringtone_dump_state();

	} else if (strcmp(s, "device") == 0) {
		char current_ringtone_device[BUFSIZE];
		int result;

		memset(current_ringtone_device, 0, BUFSIZE);
		strcpy(current_ringtone_device, ua.ringtone.device);

		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		result = ringtone_device_init(&ua, s);
		if (result < 0) {
			printf("Set ringtone device to [%s] failed\n", s);

			result = ringtone_device_init(
				&ua, current_ringtone_device);
			if (result < 0) {
				printf("Return ringtone device to ");
				printf("[%s] failed\n",
				       current_ringtone_device);
				return;
			}
		}
		cli_ringtone_dump_state();

	} else if (strlen(s) == 0)
		cli_ringtone_dump_state();

	else
		cli_help("help ringtone");
}
