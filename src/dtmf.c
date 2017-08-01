#include <netinet/in.h>
#include <string.h>
#include "dtmf.h"
#include "sip.h"

void
dtmf_send(sip_user_agent_t ua, int digit)
{
	char buf[BUFSIZE];
	dtmf_2833_payload_t dtmf;
	int i, state;

	if (ua == NULL)
		return;

	state = sip_dialog_get_state(ua->dialog);
	if (state != SIPS_CONNECTED)
		return;

	memset(buf, 0, BUFSIZE);
	dtmf = (dtmf_2833_payload_t) buf;
	dtmf->ev = digit;
	dtmf->dur = htons(0x800);       /* Appox. 250 milliseconds */

	/* Send the DTMF digit packet twice */
	for (i = 0; i < 2; i++)
		rtp_send(&(ua->rtp), buf, sizeof(struct dtmf_2833_payload),
			 RTP_PAYLOAD_DTMF_2833);

	/* Mark packet as the end */
	dtmf->flags.e = 1;

	/* Send the end DTMF digit packet twice */
	for (i = 0; i < 3; i++)
		rtp_send(&(ua->rtp), buf, sizeof(struct dtmf_2833_payload),
			 RTP_PAYLOAD_DTMF_2833);
}
