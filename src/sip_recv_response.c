#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "lex.h"
#include "log.h"
#include "sip.h"

void
sip_recv_response(sip_user_agent_t ua, msglines_t msglines)
{
	sip_dialog_t dialog;
	char s[BUFSIZE];
	int code, pos = 7;

	if (ua == NULL || msglines == NULL)
		return;

	/*
	 * Do some checking on the response message.  This routine returns
	 * the dialog for which the response is applicable.  The only states
	 * where there should be a chance of there being more than one
	 * dialog associated with the UA are the SIP_UAC_CALLING and
	 * SIP_UAC_PROCEEDING states.  This is because the only request
	 * that should be forked is an outbound INVITE and the forking
	 * proxy handles any CANCELs.
	 */
	dialog = sip_uac_check_response(ua, msglines);
	if (dialog == NULL) {
		log_msg(LOG_WARNING, "Dialog not found for response");
		return;
	}
	memset(s, 0, BUFSIZE);
	nextarg(msglines->msgbuf, &pos, " ", s);
	code = atoi(s);

	switch (dialog->state) {
	case SIPS_UAC_REGISTERING:
		if (code >= 200 && code < 300)
			sip_uac_registered(ua, msglines);
		else if (code >= 300)
			sip_uac_bad_register(ua, msglines, code);
		break;

	case SIPS_UAC_CALLING:
		if (code < 200)
			sip_uac_proceeding(ua, dialog, msglines, code);
		else if (code >= 200 && code < 300)
			sip_uac_connect(ua, dialog, msglines);
		else if (code >= 300)
			sip_uac_completed(ua, dialog, msglines, code);
		break;

	case SIPS_UAC_PROCEEDING:
		if (code < 200)
			sip_uac_proceeding(ua, dialog, msglines, code);
		else if (code >= 200 && code < 300)
			sip_uac_connect(ua, dialog, msglines);
		else if (code >= 300)
			sip_uac_completed(ua, dialog, msglines, code);
		break;

	case SIPS_UAC_CANCELING:
		if (code == 487)
			sip_uac_canceling(ua);
		break;

	case SIPS_UAC_TRYING:
		if (code >= 200)
			sip_uac_teardown(ua, code);
		break;

	case SIPS_CONNECTED:
		if (code >= 200 && code < 300) {
			char host[BUFSIZE];
			char *buf = s;
			int len, port, result;

			/* Generate ACK */
			memset(buf, 0, BUFSIZE);
			sip_gen_ack(ua, dialog, buf, BUFSIZE);

			/* Send ACK */
			memset(host, 0, BUFSIZE);
			result = sip_get_send_host_port(
				ua->dialog, host, &port, dns_avail(ua));
			if (result == 0) {
				len = sip_send(ua, host, port, buf,
					       strlen(buf));
				if (len >= 0)
					log_msg(LOG_WARNING, "Resend ACK");
			}
		}
		break;
	}
}
