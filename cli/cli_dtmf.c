#include <ctype.h>
#include <string.h>
#include "cli.h"
#include "dtmf.h"

int
cli_dtmf_scan(char *s)
{
	int i, len;

	len = strlen(s);
	for (i = 0; i < len; i++) {
		unsigned char ch = s[i];

		if (isdigit(ch))
			continue;
		if (ch == '#')
			continue;
		if (ch == '*')
			continue;
		if (ch == 'A' || ch == 'a')
			continue;
		if (ch == 'B' || ch == 'b')
			continue;
		if (ch == 'C' || ch == 'c')
			continue;
		if (ch == 'D' || ch == 'd')
			continue;

		break;
	}
	if (i == len)
		return 0;
	return (-1);
}

void
cli_dtmf_send(char *s)
{
	int i, len;

	for (len = strlen(s), i = 0; i < len; i++) {
		if (s[i] == '0') {
			dtmf_send(&ua, DTMF_2833_EVENT_0);
			continue;
		} else if (s[i] == '1') {
			dtmf_send(&ua, DTMF_2833_EVENT_1);
			continue;
		} else if (s[i] == '2') {
			dtmf_send(&ua, DTMF_2833_EVENT_2);
			continue;
		} else if (s[i] == '3') {
			dtmf_send(&ua, DTMF_2833_EVENT_3);
			continue;
		} else if (s[i] == '4') {
			dtmf_send(&ua, DTMF_2833_EVENT_4);
			continue;
		} else if (s[i] == '5') {
			dtmf_send(&ua, DTMF_2833_EVENT_5);
			continue;
		} else if (s[i] == '6') {
			dtmf_send(&ua, DTMF_2833_EVENT_6);
			continue;
		} else if (s[i] == '7') {
			dtmf_send(&ua, DTMF_2833_EVENT_7);
			continue;
		} else if (s[i] == '8') {
			dtmf_send(&ua, DTMF_2833_EVENT_8);
			continue;
		} else if (s[i] == '9') {
			dtmf_send(&ua, DTMF_2833_EVENT_9);
			continue;
		} else if (s[i] == 'A') {
			dtmf_send(&ua, DTMF_2833_EVENT_A);
			continue;
		} else if (s[i] == 'B') {
			dtmf_send(&ua, DTMF_2833_EVENT_B);
			continue;
		} else if (s[i] == 'C') {
			dtmf_send(&ua, DTMF_2833_EVENT_C);
			continue;
		} else if (s[i] == 'D') {
			dtmf_send(&ua, DTMF_2833_EVENT_D);
			continue;
		} else if (s[i] == '#') {
			dtmf_send(&ua, DTMF_2833_EVENT_POUND);
			continue;
		} else if (s[i] == '*') {
			dtmf_send(&ua, DTMF_2833_EVENT_STAR);
			continue;
		}
	}
}
