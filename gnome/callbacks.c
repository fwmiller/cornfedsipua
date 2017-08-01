#include "dtmf.h"
#include "gui.h"
#include "log.h"

gboolean
callback_delete_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_main_quit();
	return TRUE;
}

static void
callback_dial_button_pressed()
{
	int state;

	state = sip_dialog_get_state(ua.dialog);
	if (state == SIPS_IDLE) {
		struct sip_uri uri;
		struct timeval rtt;
		char *text;
		int state;

		state = sip_dialog_get_state(ua.dialog);
		if (state < 0 || state != SIPS_IDLE)
			return;

		text = (char *) gtk_entry_get_text(GTK_ENTRY(dialed_number));

		sip_uri_init(&(ua.remote_uri));
		ua.remote_uri.prefix = SIPU_SIP;
		strcpy(ua.remote_uri.user, text);
		strcpy(ua.remote_uri.endpoint.domain,
		       ua.reg_uri.endpoint.domain);
		strcpy(ua.remote_uri.endpoint.host,
		       ua.reg_uri.endpoint.host);
		ua.remote_uri.endpoint.port = ua.reg_uri.endpoint.port;

		sip_uri_init(&uri);
		uri.prefix = SIPU_SIP;
		strcpy(uri.endpoint.host, ua.local_endpoint.host);
		uri.endpoint.port = ua.local_endpoint.port;

		sip_uac_invite(&ua, &(ua.remote_uri), &uri, &(ua.reg_uri));

		ua_get_rtt(NULL, &rtt);
		sip_timer_start(
			&(ua.dialog->timers), SIPT_INVITE_RETR, rtt.tv_usec);

	} else if (state == SIPS_UAS_PROCEEDING) {
		sip_uas_answer(&ua);

		gtk_button_set_label(GTK_BUTTON(dial_button), "Dial");
		gtk_button_set_label(GTK_BUTTON(hangup_button), "Hangup");
	}
}

static void
callback_hangup_button_pressed()
{
	int state;

	state = sip_dialog_get_state(ua.dialog);
	if (state == SIPS_UAC_PROCEEDING) {
		sip_uac_cancel(&ua);
		return;
	}
	if (state != SIPS_CONNECTED)
		return;

	status_callback("Disconnected");

	sip_uac_hangup(&ua);
}

static void
callback_clear_button_pressed()
{
	int state;

	state = sip_dialog_get_state(ua.dialog);
	if (state == SIPS_IDLE) {
		gtk_entry_set_text(GTK_ENTRY(dialed_number), "");
		status_callback("Clear dialed digits");
		return;
	}
	sip_user_agent_clear(&ua);
	sip_dialog_init(ua.registration);

	gtk_button_set_label(GTK_BUTTON(dial_button), "Dial");
	gtk_button_set_label(GTK_BUTTON(hangup_button), "Hangup");

	status_callback("Client reset");
}

static void
callback_digit_button_pressed(char *label)
{
	char s[128];
	char *text;

	text = (char *) gtk_entry_get_text(GTK_ENTRY(dialed_number));

	memset(s, 0, 128);
	strcpy(s, text);
	strcat(s, label);

	gtk_entry_set_text(GTK_ENTRY(dialed_number), s);
}

static void
callback_digit_dtmf(char *label)
{
	int val = label[0] - '0';
	int state;

	state = sip_dialog_get_state(ua.dialog);

	if (val >= 0 && val <= 9 && state == SIPS_IDLE) {
		callback_digit_button_pressed(label);
		return;
	}
	if (state != SIPS_CONNECTED)
		return;

	/* Send a DTMF digit into a connected call */
	if (label[0] == '*') {
		dtmf_send(&ua, DTMF_2833_EVENT_STAR);
		return;
	}
	if (label[0] == '#') {
		dtmf_send(&ua, DTMF_2833_EVENT_POUND);
		return;
	}
	switch (val) {
	case 0:
		dtmf_send(&ua, DTMF_2833_EVENT_0);
		break;
	case 1:
		dtmf_send(&ua, DTMF_2833_EVENT_1);
		break;
	case 2:
		dtmf_send(&ua, DTMF_2833_EVENT_2);
		break;
	case 3:
		dtmf_send(&ua, DTMF_2833_EVENT_3);
		break;
	case 4:
		dtmf_send(&ua, DTMF_2833_EVENT_4);
		break;
	case 5:
		dtmf_send(&ua, DTMF_2833_EVENT_5);
		break;
	case 6:
		dtmf_send(&ua, DTMF_2833_EVENT_6);
		break;
	case 7:
		dtmf_send(&ua, DTMF_2833_EVENT_7);
		break;
	case 8:
		dtmf_send(&ua, DTMF_2833_EVENT_8);
		break;
	case 9:
		dtmf_send(&ua, DTMF_2833_EVENT_9);
		break;
	}
}

gboolean
callback_dialpad_button_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	char *label = (char *) user_data;

	log_msg(LOG_INFO, "[%s] button pressed", label);

	if (strcmp(label, "Dial") == 0)
		callback_dial_button_pressed();

	else if (strcmp(label, "Hangup") == 0)
		callback_hangup_button_pressed();

	else if (strcmp(label, "Clear") == 0)
		callback_clear_button_pressed();

	else if (strcmp(label, "Settings") == 0) {
		gui_settings_dialog_init();
		gtk_dialog_run(GTK_DIALOG(settings_dialog));
		gtk_widget_destroy(settings_dialog);
		settings_dialog = NULL;

	} else
		callback_digit_dtmf(label);

	return TRUE;
}
