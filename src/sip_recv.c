#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <string.h>
#include "log.h"
#include "sip.h"

void
sip_recv(sip_user_agent_t ua, char *buf, int len)
{
	struct msglines msglines;
	u32_t magic;
	char line[BUFSIZE];
	int i, j;
	

	if (ua == NULL || buf == NULL)
		return;

	/* Check for a STUN response */
	magic = ntohl(STUN_MAGIC);
	if (*((u32_t *) (buf + 4)) == magic) {
		stun_receive_response(ua, buf, len);
		return;
	}
	log_msg(LOG_INFO, "");
	log_msg(LOG_INFO, "<== Received %d bytes", len);

	/* Packet is a SIP message */
        memset(line, 0, BUFSIZE);
        for (i = 0, j = 0; i < len; i++) {
                if (buf[i] == '\n') {
                        log_msg(LOG_INFO, line);
                        memset(line, 0, BUFSIZE);
                        j = 0;
                        continue;
                }
                if (isprint(buf[i]))
                        line[j++] = buf[i];
	}
	/* Convert SIP message to normal form */
	lws2sws(buf, len);

	/* Overlay message lines structure on message buffer */
	memset(&msglines, 0, sizeof(msglines));
	get_msglines(buf, len, &msglines);

	if (strncmp(buf, "SIP/2.0", 7) == 0)

		sip_recv_response(ua, &msglines);
	
	else
		sip_recv_request(ua, &msglines);
}
