#include <stdio.h>
#include <string.h>
#include "cornfedsipua.h"
#include "expat.h"
#include "lex.h"

#include "cli_help_info.c"

#define HS_NULL			0
#define HS_COMMAND		1
#define HS_BRIEF		2
#define HS_SYNTAX		3
#define HS_DESCRIPTION		4

#define CMD_LENGTH		128
#define BRIEF_LENGTH		256
#define SYNTAX_LENGTH		512
#define DESCRIPTION_LENGTH	BUFSIZE
#define LINE_LENGTH		78

struct cmd_help {
	struct cmd_help *next;
	char cmd[CMD_LENGTH];
	char brief[BRIEF_LENGTH];
	char syntax[SYNTAX_LENGTH];
	char desc[DESCRIPTION_LENGTH];
};

typedef struct cmd_help *cmd_help_t;

static int help_state = HS_NULL;
static char buf[BUFSIZE];
static cmd_help_t cmd = NULL;
static cmd_help_t cmdlist;

void
cmd_insert(cmd_help_t cmd)
{
	cmd_help_t c;

	cmd->next = NULL;
	if (cmdlist == NULL) {
		cmdlist = cmd;
		return;
	}
	for (c = cmdlist; c->next != NULL; c = c->next);
	c->next = cmd;
}

static void
XMLCALL Start(void *data, const char *el, const char **attr)
{
	if (strcmp(el, "command") == 0) {
		help_state = HS_COMMAND;
		memset(buf, 0, BUFSIZE);

		if (cmd != NULL)
			cmd_insert(cmd);

		cmd = malloc(sizeof(struct cmd_help));
		memset(cmd, 0, sizeof(struct cmd_help));

	} else if (strcmp(el, "brief") == 0) {
		help_state = HS_BRIEF;
		memset(buf, 0, BUFSIZE);

	} else if (strcmp(el, "syntax") == 0) {
		help_state = HS_SYNTAX;
		memset(buf, 0, BUFSIZE);

	} else if (strcmp(el, "description") == 0) {
		help_state = HS_DESCRIPTION;
		memset(buf, 0, BUFSIZE);
	}
}

static void
XMLCALL Text(void *data, const XML_Char *s, int len)
{
	if (help_state > HS_NULL)
		strncat(buf, s, len);
}

static void
XMLCALL End(void *data, const char *el)
{
	switch (help_state) {
	case HS_COMMAND:
		if (strcmp(el, "command") == 0) {
			help_state = HS_NULL;
			if (cmd != NULL)
				strncpy(cmd->cmd, buf, CMD_LENGTH - 1);
		}
		break;

	case HS_BRIEF:
		if (strcmp(el, "brief") == 0) {
			help_state = HS_NULL;
			if (cmd != NULL)
				strncpy(cmd->brief, buf, BRIEF_LENGTH - 1);
		}
		break;

	case HS_SYNTAX:
		if (strcmp(el, "syntax") == 0) {
			help_state = HS_NULL;
			if (cmd != NULL)
				strncpy(cmd->syntax, buf, SYNTAX_LENGTH - 1);
		}
		break;

	case HS_DESCRIPTION:
		if (strcmp(el, "description") == 0) {
			help_state = HS_NULL;
			if (cmd != NULL)
				strncpy(cmd->desc, buf,
					DESCRIPTION_LENGTH - 1);
		}
		break;
	}
}

void
help_init()
{
	XML_Parser parser;

	parser = XML_ParserCreate(NULL);
	XML_SetElementHandler(parser, Start, End);
	XML_SetCharacterDataHandler(parser, Text);
	XML_Parse(parser, help, strlen(help), 1);

	if (cmd != NULL)
		cmd_insert(cmd);

	XML_ParserFree(parser);
}

static void
cli_help_brief()
{
	cmd_help_t c;
	int cnt = 0, len = 0;

	for (c = cmdlist; c != NULL; c = c->next)
		if (strlen(c->cmd) > len)
			len = strlen(c->cmd);

	for (c = cmdlist; c != NULL; c = c->next) {
		int i;

		if (cnt++ == 0)
			printf("\n");

		printf("%s", c->cmd);

		for (i = 0; i < (len + 1) - strlen(c->cmd); i++)
			printf(" ");

		printf(": %s\n", c->brief);
	}
	if (cnt > 0)
		printf("\n");
}

static void
cli_help_syntax()
{
	cmd_help_t c;
	int cnt = 0, len = 0;

	for (c = cmdlist; c != NULL; c = c->next)
		if (strlen(c->cmd) > len)
			len = strlen(c->cmd);

	for (c = cmdlist; c != NULL; c = c->next) {
		int i;

		if (cnt++ == 0)
			printf("\n");

		printf("%s", c->cmd);

		for (i = 0; i < (len + 1) - strlen(c->cmd); i++)
			printf(" ");

		printf(": %s\n", c->syntax);
	}
	if (cnt > 0)
		printf("\n");
}

static void
cli_help_layout_paragraph(char *s)
{
	char word[BUFSIZE];
	int len, pos, wordlen;

	for (len = 0, pos = 0;;) {
		memset(word, 0, BUFSIZE);
		nextarg(s, &pos, " ", word);

		wordlen = strlen(word);
		if (wordlen == 0)
			break;
		if (len + wordlen + 1 >= LINE_LENGTH) {
			printf("\n");
			len = 0;
		}
		printf("%s ", word);
		len += wordlen + 1;
	}
}

static void
cli_help_command(cmd_help_t c)
{
	printf("\n");

	printf("%s\n\n", c->cmd);

	printf("Syntax: %s\n\n", c->syntax);

	printf("Description:\n");
	cli_help_layout_paragraph(c->desc);

	printf("\n\n");
}

void
cli_help(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 4;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strlen(s) > 0) {
		cmd_help_t c;

		if (strcmp(s, "syntax") == 0) {
			cli_help_syntax();
			return;
		}
		for (c = cmdlist; c != NULL; c = c->next)
			if (strcmp(s, c->cmd) == 0) {
				cli_help_command(c);
				return;
			}
	}
	cli_help_brief();
}
