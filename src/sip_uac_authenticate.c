#include <stdio.h>
#include <string.h>
#include "lex.h"
#include "log.h"
#include "md5.h"
#include "sip.h"

static void
sip_md5_hash(char *in, char *out)
{
	struct MD5Context md5;
	unsigned char digest[16];
	char *ptr;
	int i;

	if (in == NULL || out == NULL)
		return;

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char *) in, strlen(in));
	MD5Final(digest, &md5);
	ptr = out;
	for (i = 0; i < 16; i++)
		ptr += sprintf(ptr, "%2.2x", digest[i]);
}

static void
sip_uac_auth_response(sip_user_agent_t ua, sip_dialog_t dialog, char *user,
		      char *realm, char *passwd, char *method, char *uri,
		      char *nonce, char *response)
{
	char a1[256];
	char a2[256];
	char resp[256];
	char a1hash[256];
	char a2hash[256];
	char resphash[256];

	if (ua == NULL || dialog == NULL || user == NULL || realm == NULL ||
	    passwd == NULL || method == NULL || uri == NULL ||
	    nonce == NULL || response == NULL)
		return;

	snprintf(a1, sizeof(a1), "%s:%s:%s", user, realm, passwd);
	snprintf(a2, sizeof(a2), "%s:%s", method, uri);

	log_msg(LOG_INFO, "A1: [%s]", a1);
	log_msg(LOG_INFO, "A2: [%s]", a2);

	sip_md5_hash(a1, a1hash);
	sip_md5_hash(a2, a2hash);

	log_msg(LOG_INFO, "H(A1): [%s]", a1hash);
	log_msg(LOG_INFO, "H(A2): [%s]", a2hash);

	snprintf(resp, sizeof(resp), "%s:%s:%s", a1hash, nonce, a2hash);
	sip_md5_hash(resp, resphash);

	log_msg(LOG_INFO, "H(response): [%s]", resphash);

	strcpy(response, resphash);
}

int
sip_uac_authenticate(sip_user_agent_t ua, sip_dialog_t dialog,
		     msglines_t msglines, char *method, char *user)
{
	char s[BUFSIZE];
	char *auth;
	int pos;

	if (ua == NULL || dialog == NULL || msglines == NULL ||
	    method == NULL || user == NULL)
		return (-1);

	if (strlen(dialog->reg_uri.passwd) == 0) {
		log_msg(LOG_WARNING, "No password specified");
		return (-1);
	}
	auth = msglines_find_hdr(msglines, "Proxy-Authenticate");
	if (auth != NULL)
		dialog->auth_type = SIPA_PROXY;
	else {
		auth = msglines_find_hdr(msglines, "WWW-Authenticate");
		if (auth == NULL) {
			log_msg(LOG_WARNING, "No authentication header line");

			if (dialog->auth_type == SIPA_NULL &&
			    strlen(dialog->auth_nonce) == 0 &&
			    strlen(dialog->auth_realm) == 0)
				return (-1);

			log_msg(LOG_WARNING,
				"Use existing authentication parameters");

		} else
			dialog->auth_type = SIPA_WWW;
	}
	if (auth != NULL) {
		/* Skip over header label */
		pos = 0;
		memset(s, 0, BUFSIZE);
		nextarg(auth, &pos, ":", s);
		if (auth[pos] != ':') {
			log_msg(LOG_ERROR,
				"Malformed authentication header line");
			return (-1);
		}
		/* Look for Digest authentication type */
		pos++;
		memset(s, 0, BUFSIZE);
		nextarg(auth, &pos, " ", s);
		if (strncasecmp(s, "Digest", 6) != 0) {
			log_msg(LOG_WARNING,
				"%s authentication not supported", s);
			return (-1);
		}
		/* Process authentication parameters one-by-one */
		for (;;) {
				memset(s, 0, BUFSIZE);
			nextarg(auth, &pos, ",", s);
			if (strlen(s) == 0)
				continue;

			if (strncmp(s, "realm", 5) == 0) {
				int realmpos = 0;

				memset(dialog->auth_realm, 0, BUFSIZE);
				nextarg(s, &realmpos, "\"",
					dialog->auth_realm);
				realmpos++;
				memset(dialog->auth_realm, 0, BUFSIZE);
				nextarg(s, &realmpos, "\"",
					dialog->auth_realm);

				log_msg(LOG_INFO, "Realm: [%s]",
					dialog->auth_realm);

			} else if (strncmp(s, "nonce", 5) == 0) {
				int noncepos = 0;

				memset(dialog->auth_nonce, 0, BUFSIZE);
				nextarg(s, &noncepos, "\"",
					dialog->auth_nonce);
				noncepos++;
				memset(dialog->auth_nonce, 0, BUFSIZE);
				nextarg(s, &noncepos, "\"",
					dialog->auth_nonce);

				log_msg(LOG_INFO, "Nonce: [%s]",
					dialog->auth_nonce);

			}
			if (auth[pos++] != ',')
				break;
		}
	}
	if (strlen(dialog->auth_realm) == 0 ||
	    strlen(dialog->auth_nonce) == 0) {
		log_msg(LOG_WARNING, "Missing authentication parameter(s)");
		return (-1);
	}
	memset(dialog->auth_response, 0, BUFSIZE);
	memset(dialog->authorization, 0, BUFSIZE);
	memset(s, 0, BUFSIZE);

	if (strlen(dialog->reg_uri.endpoint.domain) > 0)
		sprintf(s, "sip:%s", dialog->reg_uri.endpoint.domain);
	else
		sprintf(s, "sip:%s", dialog->reg_uri.endpoint.host);

	sip_uac_auth_response(ua, dialog, user, dialog->auth_realm,
			      dialog->reg_uri.passwd, method, s,
			      dialog->auth_nonce,
			      dialog->auth_response);
	if (dialog->auth_type == SIPA_PROXY)
		sip_gen_proxy_authorization(ua, dialog,
					    dialog->authorization, BUFSIZE,
					    dialog->auth_realm,
					    dialog->auth_nonce, user,
					    dialog->auth_response);
	else
		sip_gen_www_authorization(ua, dialog, dialog->authorization,
					  BUFSIZE, dialog->auth_realm,
					  dialog->auth_nonce, user,
					  dialog->auth_response);

	return 0;
}
