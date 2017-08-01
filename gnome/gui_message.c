#include "gui.h"

void
gui_message(GtkWidget *parent, GtkMessageType type, char *msg)
{
	GtkWidget *dialog;
	gint button;

	dialog = gtk_message_dialog_new(
		GTK_WINDOW(parent),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		type,
		GTK_BUTTONS_OK,
		msg);

	button = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
}
