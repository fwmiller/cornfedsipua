#include <string.h>
#include "dns.h"
#include "history.h"
#include "log.h"
#include "sip.h"

int
sip_uac_cancel(sip_user_agent_t ua)
{
	struct timeval rtt;
	char buf[BUFSIZE], host[BUFSIZE], s[128];
	int len, port, result;

	if (ua == NULL)
		return (-1);

	memset(host, 0, BUFSIZE);
	result = sip_get_send_host_port(ua->dialog, host, &port,
					dns_avail(ua));
	if (result < 0)
		return (-1);

	/* Generate CANCEL */
	sip_gen_cancel(ua, ua->dialog, buf, BUFSIZE);

	/*
	 * Send CANCEL
	 *
	 * Normally this state change would be applied after the CANCEL
	 * message is actually sent.  A test with a Cisco 7960 behind a
	 * Juniper Firewall at SIPIT 16 resulted in the 200 OK response
	 * to this CANCEL message arriving before the dialog state was
	 * modified.  This resulted in the subsequent 487 not being
	 * ACK'd properly
	 */
	sip_dialog_set_state(ua->dialog, SIPS_UAC_CANCELING);

	len = sip_send(ua, host, port, buf, strlen(buf));
	if (len < 0)
		return (-1);

	ua->ua_get_rtt(ua->dialog->remote_uri.endpoint.host, &rtt);
	sip_timer_start(&(ua->dialog->timers), SIPT_CANCEL_RETR,
			rtt.tv_usec);

	soundcard_flush(&(ua->soundcard));


	log_msg(LOG_CONNECTION, "Canceling Call-ID [%s]",
		ua->dialog->call_id);

	memset(s, 0, 128);
	sip_uri_gen(&(ua->dialog->remote_uri), s);
	history_add_event(ua, HIST_EVENT_CANCELED, s);

	return 0;
}

void
sip_uac_retransmit_cancel(sip_user_agent_t ua)
{
	char buf[BUFSIZE], host[BUFSIZE];
	int port, result;

	if (ua == NULL)
		return;

	memset(host, 0, BUFSIZE);
	result = sip_get_send_host_port(ua->dialog, host, &port,
					dns_avail(ua));
	if (result < 0)
		return;

	/* Generate and send CANCEL */
	sip_gen_cancel(ua, ua->dialog, buf, BUFSIZE);
	sip_send(ua, host, port, buf, strlen(buf));
}

void
sip_uac_canceling(sip_user_agent_t ua)
{
	char buf[BUFSIZE], host[BUFSIZE];
	int len, port, result;

	if (ua == NULL)
		return;

	memset(host, 0, BUFSIZE);
	result = sip_get_send_host_port(ua->dialog, host, &port,
					dns_avail(ua));
	if (result < 0)
		return;

	/* Generate ACK */
	sip_gen_ack(ua, ua->dialog, buf, BUFSIZE);

	/* Send ACK */
	len = sip_send(ua, host, port, buf, strlen(buf));
	if (len >= 0) {
		sip_user_agent_clear(ua);
		log_msg(LOG_INFO, "Request terminated");

		/* Client callback */
		ua->uac_canceled();
	}
}
