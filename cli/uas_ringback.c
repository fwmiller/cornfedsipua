#include <stdio.h>
#include <string.h>
#include "cli.h"

void
uas_ringback(sip_dialog_t dialog)
{
	char s[BUFSIZE];

	sprintf(s, "Incoming call from ");
	sip_uri_gen(&(dialog->remote_uri), s + strlen(s));
	printf("%s\n", s);

	ringtone_play(&ua);
}
