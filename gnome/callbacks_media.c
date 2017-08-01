#include "gui.h"
#include "log.h"

static int
validate_media_device(char *device)
{
	char current_device[128];
	int result;

	if (strlen(device) == 0)
		return (-1);

	if (strncmp(device, "/dev/", 5) != 0)
		return (-1);

	memset(current_device, 0, 128);
	strcpy(current_device, ua.soundcard.device);

	result = soundcard_init(&ua, device);
	if (result < 0) {
		soundcard_init(&ua, current_device);
		return (-1);
	}
	return 0;
}

static void
settings_media_device_update()
{
	char s[128];

	sprintf(s, "Device: %s", ua.soundcard.device);
	gtk_label_set_text(GTK_LABEL(settings_media_device), s);
}

gboolean
callback_settings_media_device_change_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	soundcard_device_dialog_t soundcard_device_dialog;
	gint button;

	log_msg(LOG_INFO, "Change Media Device button pressed");

	soundcard_device_dialog =
		gui_change_media_device_dialog_init(settings_dialog);
	button = gtk_dialog_run(
		GTK_DIALOG(soundcard_device_dialog->dialog));
	if (button == GTK_RESPONSE_ACCEPT) {
		char *device;
		int result;

		log_msg(LOG_INFO, "Media Device modified");

		device = (char *) gtk_entry_get_text(
			GTK_ENTRY(soundcard_device_dialog->device));

		result = validate_media_device(device);
		if (result < 0) {
			/* Fail if bad soundcard device specified */
			gui_message(soundcard_device_dialog->dialog,
				    GTK_MESSAGE_ERROR,
				    "Bad soundcard device");
			goto change_media_device_done;
		}
		/* Record new media soundcard device */
		memset(ua.soundcard.device, 0, BUFSIZE);
		strcpy(ua.soundcard.device, device);

		/*
		 * Update media soundcard device information on
		 * Settings dialog
		 */
		settings_media_device_update();
	}

change_media_device_done:
	gtk_widget_destroy(soundcard_device_dialog->dialog);
	free(soundcard_device_dialog);

	return TRUE;
}
