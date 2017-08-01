#include "gui.h"
#include "log.h"

void
uas_cancel(sip_dialog_t dialog)
{
	char s[128];

	memset(s, 0, 128);
	sprintf(s, "Canceling Call-ID [%s]", dialog->call_id);

	log_msg(LOG_CONNECTION, s);
	status(s);

	sip_dialog_init(dialog);
	rtp_endpoint_init(&(ua.rtp.remote));
	ua.rtp.codec = (-1);
	soundcard_flush(&(ua.soundcard));

	gdk_threads_enter();
	gtk_button_set_label(GTK_BUTTON(dial_button), "Dial");
	gtk_button_set_label(GTK_BUTTON(hangup_button), "Hangup");
	gdk_threads_leave();
}
