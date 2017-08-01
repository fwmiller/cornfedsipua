#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sip.h"

#define BRANCH_LENGTH	16
#define CALL_ID_LENGTH	12
#define TAG_LENGTH	8

#define REQUEST		"%s %s SIP/2.0\r\n"
#define RESPONSE	"SIP/2.0 %d %s\r\n"
#define VIA_BRANCH	"Via: SIP/2.0/%s %s:%d;rport;branch=%s\r\n"
#define VIA		"Via: SIP/2.0/%s %s:%d;rport\r\n"
#define TO_TAG		"To: <%s>;tag=%s\r\n"
#define TO		"To: <%s>\r\n"
#define FROM_TAG	"From: <%s>;tag=%s\r\n"
#define FROM		"From: <%s>\r\n"
#define CONTACT		"Contact: <%s>\r\n"
#define CONTACT_EXPIRES	"Contact: <%s>;expires=%d\r\n"
#define CALL_ID		"Call-ID: %s\r\n"
#define CSEQ		"CSeq: %d %s\r\n"
#define ALLOW		"Allow: INVITE, ACK, OPTIONS, BYE, CANCEL, SUBSCRIBE, NOTIFY\r\n"
#define MAX_FORWARDS	"Max-Forwards: 70\r\n"
#define USER_AGENT	"User-Agent: Cornfed SIP User Agent %d.%d.%d\r\n"
#define CONTENT_TYPE	"Content-Type: %s\r\n"
#define CONTENT_LENGTH	"Content-Length: %d\r\n"
#define AUTHORIZATION	"%s: Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n"
#define ACCEPT		"Accept: %s\r\n"
#define EVENT		"Event: %s\r\n"
#define EXPIRES		"Expires: %d\r\n"

#define SDP_VERSION	"v=0\r\n"
#define SDP_OWNER	"o=%s 23456 65432 IN IP4 %s\r\n"
#define SDP_SESSION	"s=voice call\r\n"
#define SDP_CONNECTION	"c=IN IP4 %s\r\n"
#define SDP_TIME	"t=0 0\r\n"
#if _CODEC_G729
#define SDP_MEDIA	"m=audio %d RTP/AVP 18 0 8 101\r\n"
#else
#define SDP_MEDIA	"m=audio %d RTP/AVP 0 8 101\r\n"
#endif
#define SDP_MEDIA_ANS	"m=audio %d RTP/AVP %d 101\r\n"
#if _CODEC_G729
#define SDP_ATTR_G729	"a=rtpmap:18 G729/8000\r\n"
#endif
#define SDP_ATTR_ULAW	"a=rtpmap:0 PCMU/8000\r\n"
#define SDP_ATTR_ALAW	"a=rtpmap:8 PCMA/8000\r\n"
#define SDP_ATTR_PTIME	"a=ptime:20\r\n"
#define SDP_ATTR_DTMF	"a=rtpmap:101 telephone-event/8000\r\n"
#define SDP_ATTR_2833	"a=fmtp:101 0-15\r\n"

#include "sip_status_lines.c"

char *via_transports[] = { "UDP", "TCP", "TLS" };

long random(void);

void
sip_gen_branch(char *branch)
{
	unsigned int n;
	int i;

	if (branch == NULL)
		return;

	strcpy(branch, "z9hG4bK");

	for (i = 7; i < BRANCH_LENGTH; i++) {
		n = random() % 62;
		if (n < 10)
			n += (unsigned int) '0';
		else if (n < 36)
			n = (n - 10) + (unsigned int) 'a';
		else
			n = (n - 36) + (unsigned int) 'A';
		branch[i] = (char) n;
	}
}

void
sip_gen_call_id(char *call_id)
{
	unsigned int n;
	int i;

	if (call_id == NULL)
		return;

	call_id[0] = (char) ((random() % 26) + (unsigned int) 'a');
	for (i = 1; i < CALL_ID_LENGTH; i++) {
		n = random() % 62;
		if (n < 10)
			n += (unsigned int) '0';
		else if (n < 36)
			n = (n - 10) + (unsigned int) 'a';
		else
			n = (n - 36) + (unsigned int) 'A';
		call_id[i] = (char) n;
	}
}

void
sip_gen_tag(char *tag)
{
	unsigned int n;
	int i;

	if (tag == NULL)
		return;
	tag[0] = (char) ((random() % 26) + (unsigned int) 'a');

	for (i = 1; i < TAG_LENGTH; i++) {
		n = random() % 62;
		if (n < 10)
			n += (unsigned int) '0';
		else if (n < 36)
			n = (n - 10) + (unsigned int) 'a';
		else
			n = (n - 36) + (unsigned int) 'A';
		tag[i] = (char) n;
	}
}

static void
sip_gen_route_set(sip_dialog_t dialog, char *buf, char *header)
{
	int i;

	if (dialog == NULL || buf == NULL || header == NULL)
		return;

	for (i = 0; i < dialog->route_set.routes; i++) {
		sprintf(buf, "%s: ", header);
		snprintf(buf + strlen(buf),
			 dialog->route_set.len[i] + 1, "%s",
			 dialog->route_set.buf + dialog->route_set.pos[i]);
		sprintf(buf + strlen(buf), "\r\n");
	}
}

static void
sip_gen_sdp(sip_user_agent_t ua, sip_dialog_t dialog, char *buf,
	    char *host, int port)
{
	if (ua == NULL || dialog == NULL || buf == NULL || host == NULL)
		return;

	sprintf(buf, SDP_VERSION);
	sprintf(buf + strlen(buf), SDP_OWNER, "cornfed", host);
	sprintf(buf + strlen(buf), SDP_SESSION);
	sprintf(buf + strlen(buf), SDP_CONNECTION, host);
	sprintf(buf + strlen(buf), SDP_TIME);
	sprintf(buf + strlen(buf), SDP_MEDIA, port);
#if _CODEC_G729
	sprintf(buf + strlen(buf), SDP_ATTR_G729);
#endif
	sprintf(buf + strlen(buf), SDP_ATTR_ULAW);
	sprintf(buf + strlen(buf), SDP_ATTR_ALAW);
	sprintf(buf + strlen(buf), SDP_ATTR_PTIME);
	sprintf(buf + strlen(buf), SDP_ATTR_DTMF);
	sprintf(buf + strlen(buf), SDP_ATTR_2833);
}

static void
sip_gen_sdp_ans(sip_user_agent_t ua, sip_dialog_t dialog, char *buf,
		char *host, int port)
{
	if (ua == NULL || dialog == NULL || buf == NULL || host == NULL)
		return;

	sprintf(buf, SDP_VERSION);
	sprintf(buf + strlen(buf), SDP_OWNER, "cornfed", host);
	sprintf(buf + strlen(buf), SDP_SESSION);
	sprintf(buf + strlen(buf), SDP_CONNECTION, host);
	sprintf(buf + strlen(buf), SDP_TIME);
	sprintf(buf + strlen(buf), SDP_MEDIA_ANS, port, ua->rtp.codec);

	if (ua->rtp.codec == RTP_PAYLOAD_G711_ULAW)
		sprintf(buf + strlen(buf), SDP_ATTR_ULAW);
	else if (ua->rtp.codec == RTP_PAYLOAD_G711_ALAW)
		sprintf(buf + strlen(buf), SDP_ATTR_ALAW);

	sprintf(buf + strlen(buf), SDP_ATTR_DTMF);
	sprintf(buf + strlen(buf), SDP_ATTR_2833);
}

void
sip_gen_register(sip_user_agent_t ua, char *buf, int len,
		 int contact, int expires)
{
	char s[BUFSIZE];
	char *host;
	struct sip_uri uri;
	int port;

	if (ua == NULL || buf == NULL)
		return;

	host = sip_get_local_host(
		ua, ua->registration->reg_uri.endpoint.host);
	port = sip_get_local_port(
		ua, ua->registration->reg_uri.endpoint.host);

	/* Generate REGISTER request line */
	memset(buf, 0, len);
	memset(s, 0, BUFSIZE);
	if (strlen(ua->registration->reg_uri.endpoint.domain) > 0)
		sprintf(s, "sip:%s",
			ua->registration->reg_uri.endpoint.domain);
	else
		sprintf(s, "sip:%s",
			ua->registration->reg_uri.endpoint.host);
	sprintf(buf, REQUEST, "REGISTER", s);

	/* Generate Via header line */
	if (ua->registration->via_hdrs != NULL &&
	    strlen(ua->registration->via_hdrs->branch) > 0)
		sprintf(buf + strlen(buf), VIA_BRANCH,
			via_transports[SIPUT_UDP], host, port,
			ua->registration->via_hdrs->branch);
	else
		sprintf(buf + strlen(buf), VIA, via_transports[SIPUT_UDP],
			host, port);

	/* Generate To header line */
	memset(s, 0, BUFSIZE);
	if (strlen(ua->registration->reg_uri.endpoint.domain) > 0)
		sprintf(s, "sip:%s@%s", ua->registration->reg_uri.user,
			ua->registration->reg_uri.endpoint.domain);
	else
		sprintf(s, "sip:%s@%s", ua->registration->reg_uri.user,
			ua->registration->reg_uri.endpoint.host);

	if (ua->registration->reg_uri.endpoint.port != SIP_DEFAULT_PORT)
		sprintf(s + strlen(s), ":%d",
			ua->registration->reg_uri.endpoint.port);

	if (strlen(ua->registration->remote_tag) > 0)
		sprintf(buf + strlen(buf), TO_TAG, s,
			ua->registration->remote_tag);
	else
		sprintf(buf + strlen(buf), TO, s);

	/* Generate From header line */
	sprintf(buf + strlen(buf), FROM_TAG, s, ua->registration->local_tag);

	if (contact) {
		/* Generate Contact header line */
		sip_uri_init(&uri);
		uri.prefix = SIPU_SIP;
		//strcpy(uri.user, ua->registration->local_uri.user);
		strcpy(uri.user, ua->reg_uri.user);
		strcpy(uri.endpoint.host, host);
		uri.endpoint.port = port;
		sip_uri_gen(&uri, s);
		if (expires >= 0)
			sprintf(buf + strlen(buf),
				CONTACT_EXPIRES, s, expires);
		else
			sprintf(buf + strlen(buf), CONTACT, s);
	}
	/* Generate Call-ID header line */
	sprintf(buf + strlen(buf), CALL_ID, ua->registration->call_id);

	/* Generate CSeq header line */
	sprintf(buf + strlen(buf), CSEQ, ua->registration->local_seq,
		"REGISTER");

	/* Check for Authorization header line */
	if (strlen(ua->registration->authorization) > 0)
		sprintf(buf + strlen(buf), ua->registration->authorization);

	/* Generate Expires header line */
	sprintf(buf + strlen(buf), EXPIRES, expires);

	/* Generate Max-Forwards header line */
	sprintf(buf + strlen(buf), MAX_FORWARDS);

	/* Generate User-Agent header line */
	sprintf(buf + strlen(buf), USER_AGENT, CORNFEDSIPUA_VERSION,
		MAJOR_RELEASE, MINOR_RELEASE);

	/* Generate Content-Length header line */
	sprintf(buf + strlen(buf), CONTENT_LENGTH, 0);
	sprintf(buf + strlen(buf), "\r\n");
}

void
sip_gen_invite(sip_user_agent_t ua, sip_dialog_t dialog, char *buf, int len)
{
	char s[BUFSIZE];
	char *sip_host, *rtp_host;
	struct sip_uri uri;
	int sip_port, rtp_port;

	if (ua == NULL || dialog == NULL || buf == NULL)
		return;

	/* Determine the remote view of the local IP addresses */
	sip_host = sip_get_local_host(ua, dialog->remote_uri.endpoint.host);
	sip_port = sip_get_local_port(ua, dialog->remote_uri.endpoint.host);
	rtp_host = rtp_get_local_host(ua, dialog->remote_uri.endpoint.host);
	rtp_port = rtp_get_local_port(ua, dialog->remote_uri.endpoint.host);

	/* Generate INVITE request line */
	memset(buf, 0, len);
	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->remote_uri), s);
	sprintf(buf, REQUEST, "INVITE", s);

	/* XXX Generate route set */

	/* Generate Via header line */
	if (dialog->via_hdrs != NULL &&
	    strlen(dialog->via_hdrs->branch) > 0)
		sprintf(buf + strlen(buf), VIA_BRANCH,
			via_transports[SIPUT_UDP], sip_host, sip_port,
			dialog->via_hdrs->branch);
	else
		sprintf(buf + strlen(buf), VIA, via_transports[SIPUT_UDP],
			sip_host, sip_port);

	/* Generate To header line */
	sprintf(buf + strlen(buf), TO, s);

	/* Generate From header line */
	memset(s, 0, BUFSIZE);
	if (sip_uri_isset(&(dialog->reg_uri))) {
		/*
		 * Use a local struct uri since we don't want the password
		 * included in the From header
		 */
		sip_uri_init(&uri);
		uri.prefix = SIPU_SIP;
		strcpy(uri.user, dialog->reg_uri.user);
		strcpy(uri.endpoint.domain,
		       dialog->reg_uri.endpoint.domain);
		strcpy(uri.endpoint.host, dialog->reg_uri.endpoint.host);
		uri.endpoint.port = dialog->reg_uri.endpoint.port;
		sip_uri_gen(&uri, s);
	} else
		sip_uri_gen(&(dialog->local_uri), s);

	sprintf(buf + strlen(buf), FROM_TAG, s, dialog->local_tag);

	/* Generate Contact header line */
	sip_uri_init(&uri);
	uri.prefix = SIPU_SIP;
	strcpy(uri.user, dialog->local_uri.user);
	strcpy(uri.endpoint.host, sip_host);
	uri.endpoint.port = sip_port;
	sip_uri_gen(&uri, s);
	sprintf(buf + strlen(buf), CONTACT, s);

	/* Generate Call-ID header line */
	sprintf(buf + strlen(buf), CALL_ID, dialog->call_id);

	/* Generate CSeq header line */
	sprintf(buf + strlen(buf), CSEQ, dialog->local_seq, "INVITE");

	/* Check for Authorization header line */
	if (strlen(dialog->authorization) > 0)
		sprintf(buf + strlen(buf), dialog->authorization);

	/* Generate Max-Forwards header line */
	sprintf(buf + strlen(buf), MAX_FORWARDS);

	/* Generate Allow header line */
	sprintf(buf + strlen(buf), ALLOW);

	/* Generate User-Agent header line */
	sprintf(buf + strlen(buf), USER_AGENT, CORNFEDSIPUA_VERSION,
		MAJOR_RELEASE, MINOR_RELEASE);

	/* Generate Content-Type header line */
	sprintf(buf + strlen(buf), CONTENT_TYPE, "application/sdp");

	/* Generate SDP content for message body */
	memset(s, 0, BUFSIZE);
	sip_gen_sdp(ua, dialog, s, rtp_host, rtp_port);

	/* Generate Content-Length header line */
	sprintf(buf + strlen(buf), CONTENT_LENGTH, strlen(s));

	/* Add SDP content to message body */
	sprintf(buf + strlen(buf), "\r\n%s", s);
}

void
sip_gen_ack(sip_user_agent_t ua, sip_dialog_t dialog, char *buf, int len)
{
	char s[BUFSIZE];
	char *host;
	struct sip_uri uri;
	int port;

	if (ua == NULL || dialog == NULL || buf == NULL)
		return;

	/*
	 * Determine the remote view of the local IP address and generate
	 * the ACK request line
	 */
	memset(buf, 0, len);
	memset(s, 0, BUFSIZE);
	if (sip_uri_isset(&(dialog->remote_target))) {
		host = sip_get_local_host(
			ua, dialog->remote_target.endpoint.host);
		port = sip_get_local_port(
			ua, dialog->remote_target.endpoint.host);
		sip_uri_gen(&(dialog->remote_target), s);
	} else {
		host = sip_get_local_host(
			ua, dialog->remote_uri.endpoint.host);
		port = sip_get_local_port(
			ua, dialog->remote_uri.endpoint.host);
		sip_uri_gen(&(dialog->remote_uri), s);
	}
	sprintf(buf, REQUEST, "ACK", s);

	/* Generate Route header line(s) */
	sip_gen_route_set(dialog, buf + strlen(buf), "Route");

	/* Generate Via header line */
	if (dialog->via_hdrs != NULL &&
	    strlen(dialog->via_hdrs->branch) > 0)
		sprintf(buf + strlen(buf), VIA_BRANCH,
			via_transports[SIPUT_UDP], host, port,
			dialog->via_hdrs->branch);
	else
		sprintf(buf + strlen(buf), VIA, via_transports[SIPUT_UDP],
			host, port);

	/* Generate To header line */
	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->remote_uri), s);
	if (strlen(dialog->remote_tag) > 0)
		sprintf(buf + strlen(buf), TO_TAG, s, dialog->remote_tag);
	else
		sprintf(buf + strlen(buf), TO, s);

	/* Generate From header line */
	memset(s, 0, BUFSIZE);
	if (sip_uri_isset(&(dialog->reg_uri))) {
		/*
		 * Use a local struct uri since we don't want the password
		 * included in the From header
		 */
		sip_uri_init(&uri);
		uri.prefix = SIPU_SIP;
		strcpy(uri.user, dialog->reg_uri.user);
		strcpy(uri.endpoint.domain, dialog->reg_uri.endpoint.domain);
		strcpy(uri.endpoint.host, dialog->reg_uri.endpoint.host);
		uri.endpoint.port = dialog->reg_uri.endpoint.port;
		sip_uri_gen(&uri, s);
	} else
		sip_uri_gen(&(dialog->local_uri), s);

	if (strlen(dialog->local_tag) > 0)
		sprintf(buf + strlen(buf), FROM_TAG, s, dialog->local_tag);
	else
		sprintf(buf + strlen(buf), FROM, s);

	/* Generate Contact header line */
	sip_uri_init(&uri);
	uri.prefix = SIPU_SIP;
	strcpy(uri.user, dialog->local_uri.user);
	strcpy(uri.endpoint.host, host);
	uri.endpoint.port = port;
	sip_uri_gen(&uri, s);
	sprintf(buf + strlen(buf), CONTACT, s);

	/* Generate Call-ID header line */
	sprintf(buf + strlen(buf), CALL_ID, dialog->call_id);

	/* Generate CSeq header line */
	sprintf(buf + strlen(buf), CSEQ, dialog->local_seq, "ACK");

	/* Check for Authorization header line */
	if (strlen(dialog->authorization) > 0)
		sprintf(buf + strlen(buf), dialog->authorization);

	/* Generate Max-Forwards header line */
	sprintf(buf + strlen(buf), MAX_FORWARDS);

	/* Generate Content-Length header line */
	sprintf(buf + strlen(buf), CONTENT_LENGTH, 0);
	sprintf(buf + strlen(buf), "\r\n");
}

char *
sip_get_reason_phrase(int code)
{
	int i;

	for (i = 0; i < sizeof(status_lines); i++)
		if (code == status_lines[i].code)
			return status_lines[i].reason_phrase;
	return NULL;
}

void
sip_gen_response(sip_user_agent_t ua, sip_dialog_t dialog, char *buf,
		 int len, int code, int cseqno, char *method, int sdp)
{
	sip_via_t via;
	struct sip_uri uri;
	char s[BUFSIZE];
	char *reason_phrase;
	char *sip_host, *rtp_host;
	int sip_port, rtp_port;

	if (ua == NULL || dialog == NULL || buf == NULL)
		return;

	/* Determine the remote view of the local IP addresses */
	sip_host = sip_get_local_host(ua, dialog->remote_uri.endpoint.host);
	sip_port = sip_get_local_port(ua, dialog->remote_uri.endpoint.host);
	rtp_host = rtp_get_local_host(ua, dialog->remote_uri.endpoint.host);
	rtp_port = rtp_get_local_port(ua, dialog->remote_uri.endpoint.host);

	reason_phrase = sip_get_reason_phrase(code);
	if (reason_phrase == NULL)
		return;

	/* Generate response line */
	memset(buf, 0, len);
	sprintf(buf, RESPONSE, code, reason_phrase);

	/* Generate Record-Route header line(s) */
	sip_gen_route_set(dialog, buf + strlen(buf), "Record-Route");

	/* Generate Via header lines */
	for (via = dialog->via_hdrs; via != NULL; via = via->next) {
		char *via_host;

		if (strlen(via->endpoint.domain) > 0)
			via_host = via->endpoint.domain;
		else
			via_host = via->endpoint.host;

		if (strlen(via->branch) > 0)
			sprintf(buf + strlen(buf), VIA_BRANCH,
				via_transports[via->transport], via_host,
				via->endpoint.port, via->branch);
		else
			sprintf(buf + strlen(buf), VIA,
				via_transports[via->transport], via_host,
				via->endpoint.port);
	}
	/* Generate To header line */
	memset(s, 0, BUFSIZE);
	if (sip_uri_isset(&(dialog->reg_uri))) {
		struct sip_uri uri;

		/*
		 * Use a local struct uri since we don't want the password
		 * included in the To header
		 */
		sip_uri_init(&uri);
		uri.prefix = SIPU_SIP;
		strcpy(uri.user, dialog->reg_uri.user);
		strcpy(uri.endpoint.domain, dialog->reg_uri.endpoint.domain);
		strcpy(uri.endpoint.host, dialog->reg_uri.endpoint.host);
		uri.endpoint.port = dialog->reg_uri.endpoint.port;
		sip_uri_gen(&uri, s);
	} else
		sip_uri_gen(&(dialog->local_uri), s);

	if (strlen(dialog->local_tag) > 0)
		sprintf(buf + strlen(buf), TO_TAG, s, dialog->local_tag);
	else
		sprintf(buf + strlen(buf), TO, s);

	/* Generate From header line */
	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->remote_uri), s);
	if (strlen(dialog->remote_tag) > 0)
		sprintf(buf + strlen(buf), FROM_TAG, s,
			dialog->remote_tag);
	else
		sprintf(buf + strlen(buf), FROM, s);

	/* Generate Contact header line */
	sip_uri_init(&uri);
	uri.prefix = SIPU_SIP;
	/* strcpy(uri.user, ua->local_uri.user); */
	strcpy(uri.endpoint.host, sip_host);
	uri.endpoint.port = sip_port;
	sip_uri_gen(&uri, s);
	sprintf(buf + strlen(buf), CONTACT, s);

	/* Generate Call-ID header line */
	sprintf(buf + strlen(buf), CALL_ID, dialog->call_id);

	/* Generate CSeq header line */
	sprintf(buf + strlen(buf), CSEQ, cseqno,
		(method == NULL ? "" : method));

	/* Code specific headers */
	if (code == 415)
		sprintf(buf + strlen(buf), ACCEPT, "application/sdp");

	/*
	 * Generate appropriate Content-Type (if necessary) and
	 * Content-Length header lines
	 */
	if (sdp) {
		memset(s, 0, BUFSIZE);
		sip_gen_sdp_ans(ua, dialog, s, rtp_host, rtp_port);
		sprintf(buf + strlen(buf), CONTENT_TYPE, "application/sdp");
		sprintf(buf + strlen(buf), CONTENT_LENGTH, strlen(s));
		sprintf(buf + strlen(buf), "\r\n%s", s);
	} else {
		sprintf(buf + strlen(buf), CONTENT_LENGTH, 0);
		sprintf(buf + strlen(buf), "\r\n");
	}
}

void
sip_gen_bye(sip_user_agent_t ua, char *buf, int len)
{
	char s[BUFSIZE];
	char *host;
	int port;

	if (ua == NULL || buf == NULL)
		return;

	/*
	 * Determine the remote view of the local IP address and generate
	 * the BYE request line
	 */
	memset(buf, 0, len);
	memset(s, 0, BUFSIZE);
	if (sip_uri_isset(&(ua->dialog->remote_target))) {
		host = sip_get_local_host(
			ua, ua->dialog->remote_target.endpoint.host);
		port = sip_get_local_port(
			ua, ua->dialog->remote_target.endpoint.host);
		sip_uri_gen(&(ua->dialog->remote_target), s);
	} else {
		host = sip_get_local_host(
			ua, ua->dialog->remote_uri.endpoint.host);
		port = sip_get_local_port(
			ua, ua->dialog->remote_uri.endpoint.host);
		sip_uri_gen(&(ua->dialog->remote_uri), s);
	}
	sprintf(buf, REQUEST, "BYE", s);

	/* Generate Route header line(s) */
	sip_gen_route_set(ua->dialog, buf + strlen(buf), "Route");

	/* Generate Via header line */
	if (ua->dialog->via_hdrs != NULL &&
	    strlen(ua->dialog->via_hdrs->branch) > 0)
		sprintf(buf + strlen(buf), VIA_BRANCH,
			via_transports[SIPUT_UDP], host, port,
			ua->dialog->via_hdrs->branch);
	else
		sprintf(buf + strlen(buf), VIA, via_transports[SIPUT_UDP],
			host, port);

	/* Generate To header line */
	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(ua->dialog->remote_uri), s);
	sprintf(buf + strlen(buf), TO_TAG, s, ua->dialog->remote_tag);

	/* Generate From header line */
	memset(s, 0, BUFSIZE);
	if (sip_uri_isset(&(ua->dialog->reg_uri))) {
		struct sip_uri uri;

		/*
		 * Use a local struct uri since we don't want the password
		 * included in the From header
		 */
		sip_uri_init(&uri);
		uri.prefix = SIPU_SIP;
		strcpy(uri.user, ua->dialog->reg_uri.user);
		strcpy(uri.endpoint.domain,
		       ua->dialog->reg_uri.endpoint.domain);
		strcpy(uri.endpoint.host, ua->dialog->reg_uri.endpoint.host);
		uri.endpoint.port = ua->dialog->reg_uri.endpoint.port;
		sip_uri_gen(&uri, s);
	} else
		sip_uri_gen(&(ua->dialog->local_uri), s);

	sprintf(buf + strlen(buf), FROM_TAG, s, ua->dialog->local_tag);

	/* Generate Call-ID header line */
	sprintf(buf + strlen(buf), CALL_ID, ua->dialog->call_id);

	/* Generate CSeq header line */
	if (ua->dialog->local_seq > 0)
		ua->dialog->local_seq++;
	else
		ua->dialog->local_seq = 1;
	sprintf(buf + strlen(buf), CSEQ, ua->dialog->local_seq, "BYE");

	/* Generate Max-Forwards header line */
	sprintf(buf + strlen(buf), MAX_FORWARDS);

	/* Generate Content-Length header line */
	sprintf(buf + strlen(buf), CONTENT_LENGTH, 0);
	sprintf(buf + strlen(buf), "\r\n");
}

void
sip_gen_proxy_authorization(sip_user_agent_t ua, sip_dialog_t dialog,
			    char *buf, int len, char *realm, char *nonce,
			    char *user, char *response)
{
	char s[BUFSIZE];

	if (ua == NULL || dialog == NULL || buf == NULL || realm == NULL ||
	    nonce == NULL || user == NULL || response == NULL)
		return;

	/* Generate Proxy-Authorization header line */
	memset(s, 0, BUFSIZE);
	if (strlen(dialog->reg_uri.endpoint.domain) > 0)
		sprintf(s, "sip:%s", dialog->reg_uri.endpoint.domain);
	else
		sprintf(s, "sip:%s", dialog->reg_uri.endpoint.host);

	sprintf(buf, AUTHORIZATION, "Proxy-Authorization",
		user, realm, nonce, s, response);
}

void
sip_gen_www_authorization(sip_user_agent_t ua, sip_dialog_t dialog,
			  char *buf, int len, char *realm, char *nonce,
			  char *user, char *response)
{
	char s[BUFSIZE];

	if (ua == NULL || dialog == NULL || buf == NULL || realm == NULL ||
	    nonce == NULL || user == NULL || response == NULL)
		return;

	/* Generate WWW-Authorization header line */
	memset(s, 0, BUFSIZE);
	if (strlen(dialog->reg_uri.endpoint.domain) > 0)
		sprintf(s, "sip:%s", dialog->reg_uri.endpoint.domain);
	else
		sprintf(s, "sip:%s", dialog->reg_uri.endpoint.host);

	sprintf(buf, AUTHORIZATION, "Authorization",
		user, realm, nonce, s, response);
}

void
sip_gen_cancel(sip_user_agent_t ua, sip_dialog_t dialog, char *buf, int len)
{
	char s[BUFSIZE];
	char *host;
	struct sip_uri uri;
	int port;

	if (ua == NULL || dialog == NULL || buf == NULL)
		return;

	/* Determine the remote view of the local IP address */
	host = sip_get_local_host(ua, dialog->remote_uri.endpoint.host);
	port = sip_get_local_port(ua, dialog->remote_uri.endpoint.host);

	/* Generate CANCEL request line */
	memset(buf, 0, len);
	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->remote_uri), s);
	sprintf(buf, REQUEST, "CANCEL", s);

	/* XXX Generate route set */

	/* Generate Via header line */
	if (dialog->via_hdrs != NULL &&
	    strlen(dialog->via_hdrs->branch) > 0)
		sprintf(buf + strlen(buf), VIA_BRANCH,
			via_transports[SIPUT_UDP], host, port,
			dialog->via_hdrs->branch);
	else
		sprintf(buf + strlen(buf), VIA, via_transports[SIPUT_UDP],
			host, port);

	/* Generate To header line */
	memset(s, 0, BUFSIZE);
	sip_uri_gen(&(dialog->remote_uri), s);
	if (strlen(dialog->remote_tag) > 0)
		sprintf(buf + strlen(buf), TO_TAG, s, dialog->remote_tag);
	else
		sprintf(buf + strlen(buf), TO, s);

	/* Generate From header line */
	memset(s, 0, BUFSIZE);
	if (sip_uri_isset(&(dialog->reg_uri))) {
		/*
		 * Use a local struct uri since we don't want the password
		 * included in the From header
		 */
		sip_uri_init(&uri);
		uri.prefix = SIPU_SIP;
		strcpy(uri.user, dialog->reg_uri.user);
		strcpy(uri.endpoint.domain, dialog->reg_uri.endpoint.domain);
		strcpy(uri.endpoint.host, dialog->reg_uri.endpoint.host);
		uri.endpoint.port = dialog->reg_uri.endpoint.port;
		sip_uri_gen(&uri, s);
	} else
		sip_uri_gen(&(dialog->local_uri), s);

	sprintf(buf + strlen(buf), FROM_TAG, s, dialog->local_tag);

	/* Generate Call-ID header line */
	sprintf(buf + strlen(buf), CALL_ID, dialog->call_id);

	/* Generate CSeq header line */
	sprintf(buf + strlen(buf), CSEQ, dialog->local_seq, "CANCEL");

	/* Generate Max-Forwards header line */
	sprintf(buf + strlen(buf), MAX_FORWARDS);

	/* Generate Content-Length header line */
	sprintf(buf + strlen(buf), CONTENT_LENGTH, 0);
	sprintf(buf + strlen(buf), "\r\n");
}
