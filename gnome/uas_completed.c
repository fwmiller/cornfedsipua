#include "gui.h"

void
uas_completed()
{
	gtk_button_set_label(GTK_BUTTON(dial_button), "Dial");
	gtk_button_set_label(GTK_BUTTON(hangup_button), "Hangup");
}
