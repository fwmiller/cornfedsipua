#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cornfedsipua.h"

#define CLI_CMD_HISTORY_LINES	16

char *cli_cmd_history_list[CLI_CMD_HISTORY_LINES];

void
cli_cmd_history_init()
{
	int i;

	for (i = 0; i < CLI_CMD_HISTORY_LINES; i++) {
		cli_cmd_history_list[i] = malloc(BUFSIZE);
		if (cli_cmd_history_list[i] != NULL)
			memset(cli_cmd_history_list[i], 0, BUFSIZE);
	}
}

void
cli_cmd_history_dump()
{
	int i, j;

	for (i = 0; i < CLI_CMD_HISTORY_LINES; i++)
		if (strlen(cli_cmd_history_list[i]) == 0)
			break;

	if (i > 0)
		printf("\n");
	for (j = i - 1; j >= 0; j--)
		printf("%2d: %s\n", j, cli_cmd_history_list[j]);
	if (i > 0)
		printf("\n");
}

void
cli_cmd_history_push(char *cmd)
{
	int i;

	if (strcmp(cmd, cli_cmd_history_list[0]) == 0)
		return;

	for (i = CLI_CMD_HISTORY_LINES - 1; i > 0; i--) {
		memset(cli_cmd_history_list[i], 0, BUFSIZE);
		strcpy(cli_cmd_history_list[i], cli_cmd_history_list[i - 1]);
	}
	memset(cli_cmd_history_list[0], 0, BUFSIZE);
	strcpy(cli_cmd_history_list[0], cmd);
}

void
cli_cmd_history_substitute(int cmd, char *cmdline)
{
	if (cmd >= 0 && cmd < CLI_CMD_HISTORY_LINES)
		strcpy(cmdline, cli_cmd_history_list[cmd]);
}
