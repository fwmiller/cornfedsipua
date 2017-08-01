#include "gui.h"
#include "log.h"

void
uas_ringback(sip_dialog_t dialog)
{
	int state = sip_dialog_get_state(ua.dialog);
	if (state == SIPS_UAS_PROCEEDING) {
		char s[128];

		gdk_threads_enter();
		gtk_button_set_label(GTK_BUTTON(dial_button), "Answer");
		gtk_button_set_label(GTK_BUTTON(hangup_button), "Refuse");
		gdk_threads_leave();

		memset(s, 0, 128);
		sprintf(s, "Call from ");
		sip_uri_gen(&(dialog->remote_uri), s + strlen(s));

		log_msg(LOG_INFO, "%s", s);
		status(s);
	}
	ringtone_play(&ua);
}
