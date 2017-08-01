#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "lex.h"
#include "log.h"
#include "sip.h"

static sip_dialog_t
sip_uac_check_to_tag(sip_user_agent_t ua,
		     sip_dialog_t *dialog_list,
		     msglines_t msglines)
{
	struct sip_uri uri;
	sip_dialog_t dialog;
	char tag[BUFSIZE];
	int result;

	if (ua == NULL || dialog_list == NULL || msglines == NULL)
		return NULL;

	/* Get tag from To header */
	result = sip_parse_to_hdr(msglines, &uri, tag, BUFSIZE,
				  dns_avail(ua));
	if (result < 0)
		return NULL;

	if (strlen(tag) > 0) {
		for (dialog = *dialog_list;
		     dialog != NULL;
		     dialog = dialog->next) {
			/* Check each dialog for a remote tag */
			if (strlen(dialog->remote_tag) == 0) {
				/*
				 * If there is no remote tag, copy the tag
				 * in the To header as the remote tag
				 */
				strcpy(dialog->remote_tag, tag);
				return dialog;
			}
			if (strcmp(tag, dialog->remote_tag) == 0)
				return dialog;
		}
		/*
		 * Arriving at this point means that there is no dialog
		 * with an empty remote tag and the To header tag did
		 * not match the remote tag of any existing dialog.
		 * Allocate a new dialog and copy any existing dialog data
		 * into it
		 */
		dialog = sip_dialog_pop();
		if (dialog == NULL)
			return NULL;
		memcpy(dialog, *dialog_list, sizeof(struct sip_dialog));

		/* The To header tag is then assigned to the new dialog */
		memset(dialog->remote_tag, 0, BUFSIZE);
		strcpy(dialog->remote_tag, tag);

		/*
		 * The dialog is placed in the list of dialogs for the
		 * client
		 */
		sip_dialog_list_insert_tail(dialog, dialog_list);

		return dialog;
	}
	/*
	 * To header has no tag. Search the dialogs for one with no
	 * remote tag.
	 */
	for (dialog = *dialog_list; dialog != NULL; dialog = dialog->next)
		if (strlen(dialog->remote_tag) == 0)
			return dialog;

	/*
	 * Bad scene.  The To header has no tag and all the dialogs
	 * have remote tags.  Not sure this should be happening but if
	 * it does, just bag the response.
	 */
	return NULL;
}

sip_dialog_t
sip_uac_check_response(sip_user_agent_t ua, msglines_t msglines)
{
	struct sip_uri uri;
	sip_dialog_t dialog;
	char method[BUFSIZE], s[BUFSIZE], tag[BUFSIZE];
	char *via;
#if 0
	int dialogcnt = 0, i, len, pos, seqno, result, viacnt;
#endif
	int dialogcnt = 0, i, len, seqno, result, viacnt;

	if (ua == NULL || msglines == NULL)
		return NULL;

	/* Determine the dialog we're working with */
	seqno = sip_parse_cseq(msglines, method, BUFSIZE);
	if (seqno < 0)
		return NULL;

	if (strcmp(method, "REGISTER") == 0){
		/* Response is for a registration */
		dialog = ua->registration;
			}

	else {
		int state;

		/* Response is for a session */
		for (dialog = ua->dialog;
		     dialog != NULL;
		     dialog = dialog->next)
			dialogcnt++;

		state = sip_dialog_get_state(ua->dialog);
		if (dialogcnt == 1 && state == SIPS_IDLE)
			return NULL;

		/* Check whether To header has a tag parameter */
		dialog = sip_uac_check_to_tag(ua, &(ua->dialog), msglines);
		if (dialog == NULL)
			return NULL;
	}
	/* Check for multiple Via headers */
	for (viacnt = 0, i = 0; i < msglines->lines; i++) {
		char *s = msglines->msgbuf + msglines->pos[i];

		if (i == msglines->body)
			break;
		if (strncasecmp(s, "Via:", 4) == 0 ||
		    strncasecmp(s, "v:", 2) == 0)
			viacnt++;
	}
	if (viacnt != 1) {
		if (viacnt == 0)
			log_msg(LOG_WARNING,
				"Response has no Via header lines");
		else
			log_msg(LOG_WARNING,
				"Response has too many Via header lines");
		return NULL;
	}
	/* Check sent-by address in Via header */
	via = msglines_find_hdr(msglines, "Via");
#if 0
	if (via != NULL &&
	    dialog->via_hdrs != NULL &&
	    strlen(dialog->via_hdrs->endpoint.host) > 0) {
		char sent_by[BUFSIZE], sent_by_resp[BUFSIZE];

		/* Generate expected sent-by value */
		memset(sent_by, 0, BUFSIZE);
		sprintf(sent_by, "%s:%d", dialog->via_hdrs->endpoint.host,
			dialog->via_hdrs->endpoint.port);

		/* Get sent-by value from Via header */
		pos = 0;
		memset(sent_by_resp, 0, BUFSIZE);
		nextarg(via, &pos, " :", sent_by_resp);

		pos++;
		memset(sent_by_resp, 0, BUFSIZE);
		nextarg(via, &pos, " ", sent_by_resp);

		pos++;
		memset(sent_by_resp, 0, BUFSIZE);
		nextarg(via, &pos, " ;", sent_by_resp);

		if (strcmp(sent_by, sent_by_resp) != 0) {
			log_msg(LOG_WARNING,
				"Wrong sent-by param in Via header line");
			log_msg(LOG_WARNING,
				"Expected sent_by [%s]", sent_by);
			log_msg(LOG_WARNING,
				"Received sent-by [%s]", sent_by_resp);
		}
	}
#endif
	/* Check rport and received in Via header */
	if (via != NULL) {
		struct sip_via via_fields;

		sip_via_init(&via_fields);
		result = sip_via_parse(via, &via_fields, dns_avail(ua));
		if (result == 0) {
#if 0
			log_msg(LOG_INFO, "Via received [%s] rport [%s]",
				via_fields.received, via_fields.rport);
#endif
			if (strlen(via_fields.received) > 0) {
				memset(ua->visible_endpoint.host,
				       0, BUFSIZE);
				strcpy(ua->visible_endpoint.host,
				       via_fields.received);
			}
			if (strlen(via_fields.rport) > 0)
				ua->visible_endpoint.port =
					atoi(via_fields.rport);
		}
	}
        /* Record URI and tag from From header */
	result = sip_parse_from_hdr(msglines, &uri, tag, BUFSIZE,
				    dns_avail(ua));
	if (result < 0)
		return NULL;



	if (strncmp(tag, dialog->local_tag, strlen(dialog->local_tag)) != 0) {
		log_msg(LOG_WARNING, "Local tag does not match");
		return NULL;
	}
	/* Compare dialog Call-ID to Call-ID header */
	result = sip_parse_callid(msglines, s, BUFSIZE);
	if (result < 0)
		return NULL;

	len = strlen(dialog->call_id);
	if (strncmp(s, dialog->call_id, len) != 0) {
		log_msg(LOG_WARNING, "Call-ID does not match Call-ID [%s] dialog->call_id [%s]", s, dialog->call_id);
		return NULL;
	}
	/* Compare local sequence number to CSeq header */
	if (seqno != dialog->local_seq) {
		log_msg(LOG_INFO, "CSeq does not match");
		return NULL;
	}
	return dialog;
}

int
sip_check_route_line(char *line, sip_route_set_t route_set)
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
sip_uac_check_route_set(sip_dialog_t dialog, msglines_t msglines)
{
	char *line;
	int i;

	if (dialog == NULL || msglines == NULL)
		return (-1);

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
			if (line[pos] != ':')
				continue;
			pos++;
			routes = sip_check_route_line(line + pos,
						      &route_set);
			if (routes <= 0)
				continue;
			for (j = routes - 1; j >= 0; j--) {
				memset(s, 0, BUFSIZE);
				strncpy(s, route_set.buf + route_set.pos[j],
					route_set.len[j]);
				sip_route_set_append(s, &(dialog->route_set));
			}
		}
	}
	return dialog->route_set.routes;
}

void
sip_uac_check_remote_target(sip_dialog_t dialog, msglines_t msglines, int dns)
{
	if (dialog == NULL || msglines == NULL)
		return;

	if (!sip_uri_isset(&(dialog->remote_target)))
		/* Record URI from Contact header */
		sip_parse_contact(msglines, &(dialog->remote_target), dns);
}

void
sip_uac_teardown(sip_user_agent_t ua, int code)
{
	char *reason_phrase;

	if (ua == NULL)
		return;

	if (!timerisset(&(ua->dialog->rtt))) {
		sip_dialog_rtt_stop(ua->dialog);
		ua->ua_set_rtt(ua->dialog->remote_uri.endpoint.host,
			       &(ua->dialog->rtt));
	}
	sip_user_agent_clear(ua);

	reason_phrase = sip_get_reason_phrase(code);
	if (reason_phrase != NULL)
		log_msg(LOG_INFO, reason_phrase);

	ua->uac_completed();
}
