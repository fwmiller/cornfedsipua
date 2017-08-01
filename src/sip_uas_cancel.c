#include <string.h>
#include <time.h>
#include "history.h"
#include "lex.h"
#include "sip.h"

void
sip_uas_cancel(sip_user_agent_t ua, msglines_t msglines)
{
	char buf[BUFSIZE], call_id[BUFSIZE];
	int result;

	if (ua == NULL || msglines == NULL)
		return;

	/* XXX Need to tighten up the matching here */

	/* Record Call-ID */
	result = sip_parse_callid(msglines, call_id, BUFSIZE);
	if (result < 0)
		return;

	memset(buf, 0, BUFSIZE);
	if (strcmp(call_id, ua->dialog->call_id) != 0) {
		/*
		 * Format a 481 Call Leg/Transaction Does Not Exist response
		 */
		sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 481,
				 ua->dialog->remote_seq, "CANCEL", 0);
		/*
		 * XXX There should not be a case where the Via hosts
		 * are anything other than IP addresses
		 */
		sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
			 ua->dialog->via_hdrs->endpoint.port, buf,
			 strlen(buf));

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

	} else {
		char s[BUFSIZE];
		int result;

		/* Check for Require header */
		result = sip_uas_check_require(ua, msglines, "INVITE");
		if (result < 0)
			return;

		/* Format a 200 OK response for CANCEL */
		sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 200,
				 ua->dialog->remote_seq, "CANCEL", 0);
		/*
		 * XXX There should not be a case where the Via hosts
		 * are anything other than IP addresses
		 */
		sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
			 ua->dialog->via_hdrs->endpoint.port, buf,
			 strlen(buf));

		memset(ua->dialog->last_resp, 0, BUFSIZE);
		memcpy(ua->dialog->last_resp, buf, strlen(buf));

		/* Format a 487 Request Terminated response for INVITE */
		memset(buf, 0, BUFSIZE);
		sip_gen_response(ua, ua->dialog, buf, BUFSIZE, 487,
				 ua->dialog->remote_seq, "INVITE", 0);
		/*
		 * XXX There should not be a case where the Via hosts
		 * are anything other than IP addresses
		 */
		sip_send(ua, ua->dialog->via_hdrs->endpoint.host,
			 ua->dialog->via_hdrs->endpoint.port, buf,
			 strlen(buf));

		memset(s, 0, BUFSIZE);
		sip_uri_gen(&(ua->dialog->remote_uri), s);
		history_add_event(ua, HIST_EVENT_CANCELED, s);

		/* XXX Don't worry about the expected ACK */
		sip_user_agent_clear(ua);

		/* Client callback */
		ua->uas_cancel(ua->dialog);
	}
}
