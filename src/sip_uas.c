#include <string.h>
#include "dns.h"
#include "lex.h"
#include "log.h"
#include "sip.h"

static int
sip_uas_check_mandatory_hdrs(msglines_t msglines)
{
	char *hdr;

	if (msglines == NULL)
		return (-1);

	/* Call-ID */
	hdr = msglines_find_hdr(msglines, "Call-ID");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing Call-ID header line");
		return (-1);
	}
	/* CSeq */
	hdr = msglines_find_hdr(msglines, "CSeq");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing CSeq header line");
		return (-1);
	}
	/* From */
	hdr = msglines_find_hdr(msglines, "From");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing From header line");
		return (-1);
	}
#if 0
	/* Max-Forwards */
	hdr = msglines_find_hdr(msglines, "Max-Forwards");
	if (hdr == NULL)
		log_msg(LOG_WARNING, "Missing Max-Forwards header line");
#endif
	/* To */
	hdr = msglines_find_hdr(msglines, "To");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing To header line");
		return (-1);
	}
	/* Via */
	hdr = msglines_find_hdr(msglines, "Via");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing Via header line");
		return (-1);
	}
	return 0;
}

int
sip_uas_check_mandatory_ack_hdrs(msglines_t msglines)
{
	if (msglines == NULL)
		return (-1);

	return sip_uas_check_mandatory_hdrs(msglines);
}

int
sip_uas_check_mandatory_bye_hdrs(msglines_t msglines)
{
	if (msglines == NULL)
		return (-1);

	return sip_uas_check_mandatory_hdrs(msglines);
}

int
sip_uas_check_mandatory_cancel_hdrs(msglines_t msglines)
{
	if (msglines == NULL)
		return (-1);

	return sip_uas_check_mandatory_hdrs(msglines);
}

int
sip_uas_check_mandatory_invite_hdrs(msglines_t msglines)
{
	char *hdr;
	int result;

	if (msglines == NULL)
		return (-1);

	result = sip_uas_check_mandatory_hdrs(msglines);
	if (result < 0)
		return (-1);

	/* Contact */
	hdr = msglines_find_hdr(msglines, "Contact");
	if (hdr == NULL) {
		log_msg(LOG_WARNING, "Missing Contact header line");
		return (-1);
	}
	return 0;
}

int
sip_uas_check_mandatory_options_hdrs(msglines_t msglines)
{
	if (msglines == NULL)
		return (-1);

	return sip_uas_check_mandatory_hdrs(msglines);
}

int
sip_uas_match_dialog(sip_user_agent_t ua, msglines_t msglines)
{
	struct sip_uri uri;
	char call_id[BUFSIZE], from_tag[BUFSIZE];
	int result;

	if (ua == NULL || msglines == NULL)
		return (-1);

	result = sip_parse_from_hdr(msglines, &uri, from_tag, BUFSIZE,
				    dns_avail(ua));
	if (result < 0)
		return (-1);

	sip_parse_callid(msglines, call_id, BUFSIZE);

	if (strcmp(from_tag, ua->dialog->remote_tag) != 0 ||
	    strcmp(call_id, ua->dialog->call_id) != 0)
		return (-1);

	return 0;
}

static int
sip_uas_check_request_uri(sip_user_agent_t ua, msglines_t msglines,
			  char *method)
{
	struct sip_uri request_uri;
	char buf[BUFSIZE], s[BUFSIZE];
	char *request;
	int pos, result;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/* Get Request URI */
	request = msglines->msgbuf;
	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(request, &pos, " ", s);
	memset(s, 0, BUFSIZE);
	nextarg(request, &pos, " ", s);
	log_msg(LOG_INFO, "Request URI: [%s]", s);

	/* Parse Request URI */
	sip_uri_init(&request_uri);
	sip_uri_parse(s, &request_uri, dns_avail(ua));

	/* XXX Check for SIP URI (only supported URI scheme currently) */
	if (request_uri.prefix != SIPU_SIP && method != NULL) {
		sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 416,
				 ua->dialog->remote_seq, method, 0);
		/*
		 * XXX There should not be a case where the Via hosts
		 * are anything other than IP addresses
		 */
		sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
			 ua->dialog->via_hdrs->endpoint.port, buf,
			 strlen(buf));

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		log_msg(LOG_WARNING, "Unsupported URI Scheme");
		return (-1);
	}
	if (method == NULL)
		return 0;

	/* Match Request URI against UA local URIs */
	log_msg(LOG_INFO, "Request URI host: [%s]",
		request_uri.endpoint.host);
	result = ua->uas_request_uri_lookup(request_uri.endpoint.host);
	if (result < 0) {
		char *host;

		host = sip_get_local_host(ua, request_uri.endpoint.host);
		if (strcmp(request_uri.endpoint.host, host) == 0 ||
		    (strlen(ua->visible_endpoint.host) > 0 &&
		     strcmp(request_uri.endpoint.host,
			    ua->visible_endpoint.host) == 0))
			return 0;

		/*
		 * Request URI does not match this user agent so send
		 * a 404 Not Found
		 */
		sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 404,
				 ua->dialog->remote_seq, method, 0);
		/*
		 * XXX There should not be a case where the Via hosts
		 * are anything other than IP addresses
		 */
		sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
			 ua->dialog->via_hdrs->endpoint.port, buf,
			 strlen(buf));

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		log_msg(LOG_WARNING, "Not Found");
		return (-1);
	}
	return 0;
}

static int
sip_uas_check_content_type(sip_user_agent_t ua, msglines_t msglines)
{
	char buf[BUFSIZE], s[BUFSIZE];
	char *content_type;
	int pos;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/*
	 * Check for Content-Type header.  If it is not present or
	 * does not contain 'application/sdp', send 415 Unsupported
	 * Media Type response.
	 */
	content_type = msglines_find_hdr(msglines, "Content-Type");
	if (content_type == NULL) {
		memset(buf, 0, BUFSIZE);
		sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 415,
				 ua->dialog->remote_seq, "INVITE", 0);
		/*
		 * XXX There should not be a case where the Via hosts
		 * are anything other than IP addresses
		 */
		sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
			 ua->dialog->via_hdrs->endpoint.port, buf,
			 strlen(buf));

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		log_msg(LOG_WARNING, "Missing Content-Type header");
		return (-1);
	}
	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(content_type, &pos, ":", s);
	if (content_type[pos++] != ':') {
		log_msg(LOG_WARNING,
			"Missing colon on Content-Type header line");
		return (-1);
	}
	memset(s, 0, BUFSIZE);
	nextarg(content_type, &pos, " ;", s);
	if (strcasecmp(s, "application/sdp") != 0) {
		sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 415,
				 ua->dialog->remote_seq, "INVITE", 0);
		/*
		 * XXX There should not be a case where the Via hosts
		 * are anything other than IP addresses
		 */
		sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
			 ua->dialog->via_hdrs->endpoint.port, buf,
			 strlen(buf));

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		log_msg(LOG_WARNING, "Unsupported Media Type");
		return (-1);
	}
	return 0;
}

int
sip_uas_check_route_set(sip_dialog_t dialog, msglines_t msglines)
{
	char *line;
	int i;
                                                                                
	if (dialog == NULL || msglines == NULL)
		return (-1);

	if (dialog->route_set.routes > 0)
		return (-1);
                                                                                
	/*
	 * Scan the lines of the message in order and copy any
	 * Record-Route headers into the route set
	 */
	for (i = 0; i < msglines->lines; i++) {
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
			for (j = 0; j < routes; j++) {
                                memset(s, 0, BUFSIZE);
                                strncpy(s, route_set.buf + route_set.pos[j],
                                        route_set.len[j]);
#if 0
				log_msg(LOG_INFO, "Route: [%s]",
					s, strlen(s));
#endif
                                sip_route_set_append(s,
                                                     &(dialog->route_set));
			}
		}
	}
	return dialog->route_set.routes;
}

int
sip_uas_check_via_hdrs(sip_user_agent_t ua, sip_dialog_t dialog,
		       msglines_t msglines)
{
	char *buf;
	int i;

	if (ua == NULL || dialog == NULL || msglines == NULL)
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
			result = sip_via_parse(hdr, via, dns_avail(ua));
			if (result < 0) {
				sip_via_push(via);
				return (-1);
			}
			sip_via_list_insert_tail(via,
						 &(dialog->via_hdrs));
		}
	}
	return 0;
}

static int
sip_uas_check_request(sip_user_agent_t ua, msglines_t msglines, char *method)
{
	int result;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/* Compare local URI to Request-URI */
	result = sip_uas_check_request_uri(ua, msglines, method);
	if (result < 0)
		return (-1);

	return 0;
}

int
sip_uas_check_invite(sip_user_agent_t ua, msglines_t msglines)
{
	int result;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/* Do some generic checking on the INVITE request */
	result = sip_uas_check_request(ua, msglines, "INVITE");
	if (result < 0)
		return (-1);

	/* Check Content-Type header */
	result = sip_uas_check_content_type(ua, msglines);
	if (result < 0)
		return (-1);

	return 0;
}

int
sip_uas_check_reinvite(sip_user_agent_t ua, msglines_t msglines)
{
	int result;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/* Do some generic checking on the INVITE request */
	result = sip_uas_check_request(ua, msglines, "INVITE");
	if (result < 0)
		return (-1);

	/* Check Content-Type header */
	result = sip_uas_check_content_type(ua, msglines);
	if (result < 0)
		return (-1);

	return 0;
}

int
sip_uas_check_ack(sip_user_agent_t ua, msglines_t msglines)
{
	int result;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/* Do some generic checking on the ACK request */
	result = sip_uas_check_request(ua, msglines, NULL);
	if (result < 0)
		return (-1);
	return 0;
}

int
sip_uas_check_bye(sip_user_agent_t ua, msglines_t msglines)
{
	int result;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/* Do some generic checking on the BYE request */
	result = sip_uas_check_request(ua, msglines, "BYE");
	if (result < 0)
		return (-1);

	return 0;
}

int
sip_uas_check_require(sip_user_agent_t ua, msglines_t msglines, char *method)
{
	char buf[BUFSIZE];
	char *require;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/*
	 * Check for Require header.  The current release does not
	 * support any extensions so if the Require header is present,
	 * send a 420 Bad Extension response
	 */
	require = msglines_find_hdr(msglines, "Require");
	if (require == NULL)
		return 0;

	memset(buf, 0, BUFSIZE);
	sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 420,
			 ua->dialog->remote_seq, method, 0);
	/*
	 * XXX There should not be a case where the Via hosts are anything
	 * other than IP addresses
	 */
	sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
		 ua->dialog->via_hdrs->endpoint.port, buf, strlen(buf));

	memset(ua->dialog->last_resp, 0, BUFSIZE);
	memcpy(ua->dialog->last_resp, buf, strlen(buf));

	log_msg(LOG_WARNING, "Bad Extension");
	return (-1);
}

int
sip_uas_check_supported(sip_user_agent_t ua, msglines_t msglines)
{
	char s[BUFSIZE];
	char *supported;
	int pos;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/* Check for Supported header */
	supported = msglines_find_hdr(msglines, "Supported");
	if (supported == NULL)
		return 0;

	pos = 0;
	memset(s, 0, BUFSIZE);
	nextarg(supported, &pos, ":", s);
	if (supported[pos++] != ':') {
		log_msg(LOG_WARNING,
			"Missing colon on Supported header line");
		return (-1);
	}
	for (;;) {
		memset(s, 0, BUFSIZE);
		nextarg(supported, &pos, ",", s);
		if (strlen(s) == 0)
			break;

		if (strcmp(s, "100rel") == 0) {
			log_msg(LOG_WARNING, "100rel extension supported");
		} else {
			log_msg(LOG_WARNING, "[%s] extension supported", s);
		}
	}
	return  0;
}
