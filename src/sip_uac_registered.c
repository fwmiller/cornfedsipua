#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "log.h"
#include "lex.h"
#include "sip.h"

char *strcasestr(char *haystack, char *needle);

static int
sip_compare_contact_hdr(sip_user_agent_t ua, char *host,
			int port, char *hdr)
{
	struct sip_uri uri;
	char contact[BUFSIZE], expires[128];
	char *s, *s1;
	int pos = 0;

	if (ua == NULL || host == NULL || hdr == NULL)
		return (-1);

	s = index(hdr, ':');
	if (s == NULL)
		return (-1);

	for (;;) {
		memset(contact, 0, BUFSIZE);
		s = s + pos + 1;
		pos = 0;
		nextarg(s, &pos, ",", contact);
		memset(&uri, 0, sizeof(struct sip_uri));
		sip_uri_parse(contact, &uri, dns_avail(ua));

		if (strlen(uri.user) > 0) {
			if (strcmp(uri.user, 
				   ua->registration->local_uri.user) != 0 &&
			    strcmp(uri.user, 
				   ua->registration->reg_uri.user) != 0) {
				if (s[pos] == ',')
					continue;
				break;
			}
		}
		if (strcmp(uri.endpoint.host, host) != 0) {
			if (s[pos] == ',')
				continue;
			break;
		}
		if (uri.endpoint.port != port) {
			if (s[pos] == ',')
				continue;
			break;
		}
		/* Get the expires value from the Contact header */
		s1 = strcasestr(s, "expires=");
		if (s1 == NULL)
			break;

		s1 += 8;
		memset(expires, 0, 128);
		pos = 0;
		nextarg(s1, &pos, "; ", expires);

		
		return atoi(expires);

	}
	return (-1);
}

static int
sip_find_registered_contact_hdr(sip_user_agent_t ua, msglines_t msglines)
{
	char *hdr, *host;
	int i, port;

	if (ua == NULL || msglines == NULL){
		return (-1);
}
	host = sip_get_local_host(
		ua, ua->registration->reg_uri.endpoint.host);
	port = sip_get_local_port(
		ua, ua->registration->reg_uri.endpoint.host);

	for (i = 0; i < msglines->lines; i++) {
		hdr = msglines->msgbuf + msglines->pos[i];
		if (strncasecmp(hdr, "Contact:", 8) == 0 ||
		    strncasecmp(hdr, "m:", 2) == 0) {
			int result =
				sip_compare_contact_hdr(ua, host, port, hdr);
			if (result < 0)
				continue;
			return result;
		}
	}
	return (-1);
}

void
sip_uac_registered(sip_user_agent_t ua, msglines_t msglines)
{
	int expires;

	if (ua == NULL || msglines == NULL)
		return;

	/* Cancel REGISTER retry timer */
	sip_timer_cancel(&(ua->registration->timers));

	sip_dialog_rtt_stop(ua->registration);
	ua->ua_set_rtt(ua->registration->remote_uri.endpoint.host,
		       &(ua->registration->rtt));

	expires = sip_find_registered_contact_hdr(ua, msglines);
	
	if (expires >= 0) {
		int min, sec;
		char s[128];

		min = expires / 60;
		sec = expires % 60;

		memset(s, 0, 128);
		sprintf(s, "Registration expires in");

		if (min > 1)
			sprintf(s + strlen(s), " %d minutes", min);
		else if (min == 1)
			sprintf(s + strlen(s), " 1 minute");

		if (sec > 1)
			sprintf(s + strlen(s), " %d seconds", sec);
		else if (sec == 1)
			sprintf(s + strlen(s), " 1 second");

		log_msg(LOG_INFO, s);

		(ua->reg_set_expires)(
			ua->registration->reg_uri.endpoint.host, expires);

	} else {
		log_msg(LOG_INFO, "Registration not present");

		(ua->reg_set_expires)(
			ua->registration->reg_uri.endpoint.host, (-1));
	}
	sip_dialog_init(ua->registration);
}
