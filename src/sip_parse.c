#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "log.h"
#include "sip.h"

char *strcasestr(char *haystack, char *needle);

static void
sip_parse_tag(char *uri, char *tag)
{
	char *s;
	int pos;

	if (uri == NULL || tag == NULL)
		return;

	s = strcasestr(uri, "tag=");
	if (s != NULL) {
		s += 4;
		pos = 0;
		nextarg(s, &pos, "; ", tag);
	}
}

int
sip_parse_to_hdr(msglines_t msglines, sip_uri_t uri,
		 char *tag, int taglen, int dns)
{
	char s[BUFSIZE];
	char *hdr;
	int pos;

	if (msglines == NULL || uri == NULL)
		return (-1);

	/* Locate To header line */
	hdr = msglines_find_hdr(msglines, "To");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing To header");
		return (-1);
	}
	/* Locate URI in To header */
	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, ":", s);
	if (hdr[pos++] != ':') {
		log_msg(LOG_WARNING, "Bad To header format");
		return (-1);
	}
	/* Get URI */
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, "", s);
	sip_uri_init(uri);
	sip_uri_parse(s, uri, dns);

	/* Locate tag at end of To header */
	if (tag != NULL) {
		char *s1;

		memset(tag, 0, taglen);
		s1 = index(hdr, ';');
		if (s1 != NULL)
			sip_parse_tag(s1 + 1, tag);
	}
	return 0;
}

int
sip_parse_from_hdr(msglines_t msglines, sip_uri_t uri,
		   char *tag, int taglen, int dns)
{
	char s[BUFSIZE];
	char *hdr;
	int pos;

	if (msglines == NULL || uri == NULL)
		return (-1);

	/* Locate From header line */
	hdr = msglines_find_hdr(msglines, "From");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing From header");
		return (-1);
	}
	/* Locate URI in From header */
	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, ":", s);
	if (hdr[pos++] != ':') {
		log_msg(LOG_WARNING, "Bad From header format");
		return (-1);
	}
	/* Get URI */
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, "", s);
	sip_uri_init(uri);
	sip_uri_parse(s, uri, dns);

	/* Locate tag at end of From header */
	if (tag != NULL) {
		char *s1;

		memset(tag, 0, taglen);
		s1 = index(hdr, ';');
		if (s1 != NULL)
			sip_parse_tag(s1 + 1, tag);
	}
	return 0;
}

int
sip_parse_callid(msglines_t msglines, char *callid, int callidlen)
{
	char s[BUFSIZE];
	char *hdr;
	int pos;

	if (msglines == NULL || callid == NULL)
		return (-1);

	/* Locate Call-ID header line */
	hdr = msglines_find_hdr(msglines, "Call-ID");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing Call-ID header");
		return (-1);
	}
	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, ":", s);
	if (hdr[pos++] != ':') {
		log_msg(LOG_WARNING, "Bad Call-ID header format");
		return (-1);
	}
	/* Get Call-ID */
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, NULL, s);

	memset(callid, 0, callidlen);
	strcpy(callid, s);

	return 0;
}

int
sip_parse_cseq(msglines_t msglines, char *method, int methodlen)
{
	char s[BUFSIZE];
	char *hdr;
	int pos;

	if (msglines == NULL)
		return (-1);

	/* Locate CSeq header */
	hdr = msglines_find_hdr(msglines, "CSeq");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing CSeq header");
		return (-1);
	}
	/* Skip over CSeq header label */
	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, ":", s);
	if (hdr[pos++] != ':') {
		log_msg(LOG_WARNING, "Bad CSeq header format");
		return (-1);
	}
	/* Get CSeq number */
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, " ", s);

	/* Get the method if requested */
	if (method != NULL) {
		pos++;
		memset(method, 0, methodlen);
		nextarg(hdr, &pos, " ", method);
	}
	/* return CSeq number */
	return atoi(s);
}

int
sip_parse_contact(msglines_t msglines, sip_uri_t uri, int dns)
{
	char s[BUFSIZE];
	char *hdr;
	int pos;

	if (msglines == NULL || uri == NULL)
		return (-1);

	/* Locate Contact header line */
	hdr = msglines_find_hdr(msglines, "Contact");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing Contact header");
		return (-1);
	}
	/* Locate first URI in Contact header */
	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, ":", s);
	if (hdr[pos++] != ':') {
		log_msg(LOG_WARNING, "Bad Contact header format");
		return (-1);
	}
	/* Get URI */
	memset(s, 0, BUFSIZE);
	nextarg(hdr, &pos, ",", s);
	sip_uri_init(uri);
	sip_uri_parse(s, uri, dns);

	return 0;
}

static int
sip_parse_route_line(char *line, sip_route_set_t route_set)
{
	char s[BUFSIZE];
	int pos = 0;

	if (line == NULL || route_set == NULL)
		return (-1);

	sip_route_set_init(route_set);

	for (;;) {
		memset(s, 0, BUFSIZE);
		nextarg(line, &pos, " ,", s);
		if (strlen(s) == 0)
			return route_set->routes;

		sip_route_set_append(s, route_set);

		if (line[pos] == ',') {
			pos++;
			continue;
		}
		memset(s, 0, BUFSIZE);
		nextarg(line, &pos, ",", s);
		if (line[pos] == ',') {
			pos++;
			continue;
		}
		if (line[pos] != '\0')
			return (-1);

		return route_set->routes;
	}
}

int
sip_parse_route_set(msglines_t msglines, sip_dialog_t dialog)
{
	char *line;
	int i;

	if (msglines == NULL || dialog == NULL)
		return (-1);

	/* Check whether the route set was previously recorded */
	if (dialog->route_set.routes > 0)
		return (-1);

	/*
	 * Scan the lines of the message in reverse order and copy
	 * any Record-Route headers into the route set
	 */
	for (i = msglines->lines - 1; i >= 0; i--) {
		line = msglines->msgbuf + msglines->pos[i];

		if (strncasecmp(line, "Record-Route", 12) == 0) {
			struct sip_route_set route_set;
			char s[BUFSIZE];
			int j, pos = 12, routes;

			memset(s, 0, BUFSIZE);
			nextarg(line, &pos, ":", s);
			if (line[pos++] != ':')
				continue;

			routes = sip_parse_route_line(line + pos,
						      &route_set);
			if (routes <= 0)
				continue;
			for (j = routes - 1; j >= 0; j--) {
				memset(s, 0, BUFSIZE);
				strncpy(s, route_set.buf + route_set.pos[j],
					route_set.len[j]);
				sip_route_set_append(
					s, &(dialog->route_set));
			}
		}
	}
	return dialog->route_set.routes;
}

int
sip_parse_via_hdrs(msglines_t msglines, sip_dialog_t dialog, int dns)
{
	char *buf;
	int i;

	if (msglines == NULL || dialog == NULL)
		return (-1);

	/* Clear any existing Via headers */
	sip_free_via_hdrs(&(dialog->via_hdrs));

	buf = msglines->msgbuf;
	for (i = 0; i < msglines->lines; i++) {
		char *hdr = buf + msglines->pos[i];

		if (strncasecmp(hdr, "Via:", 4) == 0 ||
		    strncasecmp(hdr, "v:", 2) == 0) {
			sip_via_t via;
			int result;

			via = sip_via_pop();
			if (via == NULL)
				return (-1);

			result = sip_via_parse(hdr, via, dns);
			if (result < 0) {
				sip_via_push(via);
				return (-1);
			}
			sip_via_list_insert_tail(via, &(dialog->via_hdrs));
		}
	}
	return 0;
}
