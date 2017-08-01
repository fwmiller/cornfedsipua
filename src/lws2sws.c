#include <ctype.h>
#include <stdlib.h>

int
lws2sws(char *msgbuf, int len)
{
	int h = 0, t = 0;
	int lws = 0;

	if (msgbuf == NULL)
		return (-1);

	for (; h < len;) {
		/* Eliminate all CRs */
		if (msgbuf[h] == '\r') {
			h++;
			continue;
		}
		/* Check for end-of-line */
		if (msgbuf[h] == '\n') {
			/* Check for end-of-message */
			if (h + 1 == len)
				break;

			/* Check for a continuation line */
			if (msgbuf[h + 1] == ' ') {
				/* Merge continuation line */
				h++;
				continue;
			}
			/* Propagate LF and start new line */
			msgbuf[t++] = msgbuf[h++];
			lws = 0;
			continue;
		}
		if (msgbuf[h] == ' ' || msgbuf[h] == '\t') {
			if (lws) {
				h++;
				continue;
			}
			msgbuf[t++] = msgbuf[h++];
			lws = 1;
			continue;
		}
		msgbuf[t++] = msgbuf[h++];
		if (lws)
			lws = 0;
	}
	msgbuf[t] = '\0';
	return t;
}
