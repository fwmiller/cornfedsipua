#include "gui.h"
#include "log.h"

static int
validate_ringtone_device(char *device)
{
	char current_device[128];
	int result;

	if (strlen(device) == 0)
		return (-1);

	if (strncmp(device, "/dev/", 5) != 0)
		return (-1);

	memset(current_device, 0, 128);
	strcpy(current_device, ua.ringtone.device);

	result = soundcard_init(&ua, device);
	if (result < 0) {
		soundcard_init(&ua, current_device);
		return (-1);
	}
	return 0;
}

static void
settings_ringtone_device_update()
{
	char s[128];

	sprintf(s, "Device: %s", ua.ringtone.device);
	gtk_label_set_text(GTK_LABEL(settings_ringtone_device), s);
}

gboolean
callback_settings_ringtone_device_change_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	soundcard_device_dialog_t soundcard_device_dialog;
	gint button;

	log_msg(LOG_INFO, "Change Ringtone Device button pressed");

	soundcard_device_dialog =
		gui_change_ringtone_device_dialog_init(settings_dialog);
	button = gtk_dialog_run(
		GTK_DIALOG(soundcard_device_dialog->dialog));
	if (button == GTK_RESPONSE_ACCEPT) {
		char *device;
		int result;

		log_msg(LOG_INFO, "Ringtone Device modified");

		device = (char *) gtk_entry_get_text(
			GTK_ENTRY(soundcard_device_dialog->device));

		result = validate_ringtone_device(device);
		if (result < 0) {
			/* Fail if bad soundcard device specified */
			gui_message(soundcard_device_dialog->dialog,
				    GTK_MESSAGE_ERROR,
				    "Bad soundcard device");
			goto change_ringtone_device_done;
		}
		/* Record new ringtone soundcard device */
		memset(ua.ringtone.device, 0, BUFSIZE);
		strcpy(ua.ringtone.device, device);

		/*
		 * Update ringtone soundcard device information on
		 * Settings dialog
		 */
		settings_ringtone_device_update();
	}

change_ringtone_device_done:
	gtk_widget_destroy(soundcard_device_dialog->dialog);
	free(soundcard_device_dialog);

	return TRUE;
}

gboolean
callback_settings_ringtone_file_change_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	log_msg(LOG_INFO, "Change Ringtone File button pressed");

	gui_change_ringtone_file_dialog_init();

	return TRUE;
}

void
callback_ringtone_set_file(GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector = GTK_WIDGET(user_data);
	const gchar *filename;
	char current_ringtone_file[BUFSIZE];
	char s[BUFSIZE];
	int result, state;

	state = sip_dialog_get_state(ua.dialog);
	if (state != SIPS_IDLE)
		return;

	filename = gtk_file_selection_get_filename(
		GTK_FILE_SELECTION(file_selector));

	memset(current_ringtone_file, 0, BUFSIZE);
	strcpy(current_ringtone_file, ua.ringtone.file);

	result = ringtone_file_init(&(ua.ringtone), (const char *) filename);
	if (result < 0) {
		ringtone_file_init(&(ua.ringtone),
				   (const char *) current_ringtone_file);
		return;
	}
	memset(ua.ringtone.file, 0, BUFSIZE);
	strcpy(ua.ringtone.file, filename);

	memset(s, 0, 128);
	sprintf(s, "File: ");
	if (strlen(ua.ringtone.file) > 0)
		sprintf(s + strlen(s), "%s", ua.ringtone.file);
	else
		sprintf(s + strlen(s), "none");
	gtk_label_set_text(GTK_LABEL(settings_ringtone_file), s);

	log_msg(LOG_INFO, "Ringtone file [%s]", ua.ringtone.file);
}

