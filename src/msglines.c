#include <string.h>
#include "sip.h"

int
get_msglines(char *msgbuf, int len, msglines_t msglines)
{
	int i;

	if (msgbuf == NULL || msglines == NULL)
		return (-1);

	msglines->msgbuf = msgbuf;
	msglines->lines = 0;
	msglines->body = (-1);
	for (i = 0; i < MAX_MSG_LINES; i++) {
		msglines->pos[i] = 0;
		msglines->len[i] = 0;
	}
	for (i = 0; i < len; i++) {
		if (msgbuf[i] == '\0')
			break;

		if (msgbuf[i] == '\n') {
			if (msglines->len[msglines->lines] == 0 &&
			    msglines->body < 0)
				msglines->body = msglines->lines;

			msgbuf[i] = '\0';
			(msglines->lines)++;

			if (msglines->lines == MAX_MSG_LINES)
				break;
			if (i + 1 == len)
				break;

			msglines->pos[msglines->lines] = i + 1;
			continue;
		}
		(msglines->len[msglines->lines])++;
	}
	if (msglines->len[msglines->lines] > 0)
		msglines->lines++;
	return 0;
}

char *
msglines_find_hdr(msglines_t msglines, char *hdr)
{
	char *buf;
	int i, len;

	if (msglines == NULL || hdr == NULL)
		return NULL;

	/* Shorthand */
	buf = msglines->msgbuf;

	for (i = 0; i < msglines->lines; i++) {
		/* Shorthand */
		len = strlen(hdr);

		if (strncasecmp(buf + msglines->pos[i], hdr, len) == 0)
			break;

		/* Look for compact forms */
		if (strcasecmp(hdr, "Call-ID") == 0 &&
		    strncmp(buf + msglines->pos[i], "i:", 2) == 0)
			break;
		if (strcasecmp(hdr, "Contact") == 0 &&
		    strncmp(buf + msglines->pos[i], "m:", 2) == 0)
			break;
		if (strcasecmp(hdr, "Content-Encoding") == 0 &&
		    strncmp(buf + msglines->pos[i], "e:", 2) == 0)
			break;
		if (strcasecmp(hdr, "Content-Length") == 0 &&
		    strncmp(buf + msglines->pos[i], "l:", 2) == 0)
			break;
		if (strcasecmp(hdr, "Content-Type") == 0 &&
		    strncmp(buf + msglines->pos[i], "c:", 2) == 0)
			break;
		if (strcasecmp(hdr, "From") == 0 &&
		    strncmp(buf + msglines->pos[i], "f:", 2) == 0)
			break;
		if (strcasecmp(hdr, "Subject") == 0 &&
		    strncmp(buf + msglines->pos[i], "s:", 2) == 0)
			break;
		if (strcasecmp(hdr, "Supported") == 0 &&
		    strncmp(buf + msglines->pos[i], "k:", 2) == 0)
			break;
		if (strcasecmp(hdr, "To") == 0 &&
		    strncmp(buf + msglines->pos[i], "t:", 2) == 0)
			break;
		if (strcasecmp(hdr, "Via") == 0 &&
		    strncmp(buf + msglines->pos[i], "v:", 2) == 0)
			break;
	}
	if (i == msglines->lines)
		return NULL;
	return buf + msglines->pos[i];
}

int
msglines_get_method(msglines_t msglines)
{
	char *req = msglines->msgbuf;

	if (msglines == NULL)
		return SIPM_UNDEF;

	if (strncmp(req, "INVITE", 6) == 0)
		return SIPM_INVITE;
	else if (strncmp(req, "ACK", 3) == 0)
		return SIPM_ACK;
	else if (strncmp(req, "OPTIONS", 7) == 0)
		return SIPM_OPTIONS;
	else if (strncmp(req, "BYE", 3) == 0)
		return SIPM_BYE;
	else if (strncmp(req, "CANCEL", 6) == 0)
		return SIPM_CANCEL;
	else if (strncmp(req, "REGISTER", 8) == 0)
		return SIPM_REGISTER;
	return SIPM_UNDEF;
}
