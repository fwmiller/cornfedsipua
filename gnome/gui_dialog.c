#include "gui.h"

static GtkWidget *
gui_new_table(GtkWidget *parent, int rows, int cols)
{
	GtkWidget *table;

	table = gtk_table_new(rows, cols, FALSE);
	gtk_widget_show(table);
	gtk_container_set_border_width(
		GTK_CONTAINER(table), 2 * BORDER_WIDTH);
	gtk_table_set_row_spacings(GTK_TABLE(table), BORDER_WIDTH);
	gtk_table_set_col_spacings(GTK_TABLE(table), BORDER_WIDTH);
	gtk_container_add(GTK_CONTAINER(parent), table);

	return table;
}

static GtkWidget *
gui_new_cell_label(GtkWidget *table, gchar *text, int col, int row,
		   gfloat horiz_align)
{
	GtkWidget *label;
	char s[128];

	label = gtk_label_new(NULL);
	gtk_widget_show(label);
	memset(s, 0, 128);
	sprintf(s, "%s", text);
	gtk_label_set_markup(GTK_LABEL(label), s);
	gtk_misc_set_alignment(GTK_MISC(label), horiz_align, 0.5);
	gtk_table_attach(
		GTK_TABLE(table), label,
		col, col + 1, row, row + 1,
		(GtkAttachOptions) GTK_FILL,
		(GtkAttachOptions) GTK_FILL, 0, 0);

	return label;
}

static GtkWidget *
gui_new_cell_entry(GtkWidget *table, gchar *text, int col, int row)
{
	GtkWidget *entry;

	entry = gtk_entry_new();
	gtk_widget_show(entry);
	gtk_entry_set_text(GTK_ENTRY(entry), text);
	gtk_table_attach(
		GTK_TABLE(table), entry,
		col, col + 1, row, row + 1,
		(GtkAttachOptions) GTK_EXPAND | GTK_FILL,
		(GtkAttachOptions) GTK_FILL, 0, 0);

	return entry;
}

provider_dialog_t
gui_change_provider_dialog_init(GtkWidget *parent)
{
	provider_dialog_t provider_dialog;
	GtkWidget *table;
	GtkObject *adjustment;

	provider_dialog = malloc(sizeof(struct provider_dialog));
	if (provider_dialog == NULL)
		return NULL;

	provider_dialog->dialog = gtk_dialog_new_with_buttons(
		"Change Service Provider",
		GTK_WINDOW(parent),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);

	table = gui_new_table(
		(GTK_DIALOG(provider_dialog->dialog))->vbox, 5, 2);

	gui_new_cell_label(table, "Host", 0, 0, 1.0);
	gui_new_cell_label(table, "Port", 0, 1, 1.0);
	gui_new_cell_label(table, "User", 0, 2, 1.0);
	gui_new_cell_label(table, "Password", 0, 3, 1.0);
	gui_new_cell_label(table, "Confirm", 0, 4, 1.0);

	/* Host entry */
	if (strlen(ua.reg_uri.endpoint.domain) > 0)
		provider_dialog->host =
			gui_new_cell_entry(
				table, ua.reg_uri.endpoint.domain, 1, 0);
	else
		provider_dialog->host =
			gui_new_cell_entry(
				table, ua.reg_uri.endpoint.host, 1, 0);

	/* Port entry */
	adjustment = gtk_adjustment_new(
		ua.reg_uri.endpoint.port, 0, 65535, 1, 10, 10);
	provider_dialog->port =
		gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_widget_show(provider_dialog->port);
	gtk_table_attach(
		GTK_TABLE(table), provider_dialog->port, 1, 2, 1, 2,
		(GtkAttachOptions) GTK_EXPAND | GTK_FILL,
		(GtkAttachOptions) GTK_FILL, 0, 0);

	/* User entry */
	provider_dialog->user =
		gui_new_cell_entry(table, ua.reg_uri.user, 1, 2);

	/* Password and Confirm entries */
	provider_dialog->password =
		gui_new_cell_entry(table, ua.reg_uri.passwd, 1, 3);
	gtk_entry_set_visibility(
		GTK_ENTRY(provider_dialog->password), FALSE);
	gtk_entry_set_invisible_char(
		GTK_ENTRY(provider_dialog->password), '*');
	provider_dialog->confirm =
		gui_new_cell_entry(table, ua.reg_uri.passwd, 1, 4);
	gtk_entry_set_visibility(
		GTK_ENTRY(provider_dialog->confirm), FALSE);
	gtk_entry_set_invisible_char(
		GTK_ENTRY(provider_dialog->confirm), '*');

	return provider_dialog;
}

stun_server_dialog_t
gui_set_stun_server_dialog_init(GtkWidget *parent)
{
	stun_server_dialog_t stun_server_dialog;
	GtkWidget *table;

	stun_server_dialog = malloc(sizeof(struct stun_server_dialog));
	if (stun_server_dialog == NULL)
		return NULL;
	
	stun_server_dialog->dialog = gtk_dialog_new_with_buttons(
		"Set STUN Server",
		GTK_WINDOW(parent),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);

	table = gui_new_table(
		(GTK_DIALOG(stun_server_dialog->dialog))->vbox, 1, 2);

	gui_new_cell_label(table, "Server", 0, 0, 1.0);

	if (strlen(ua.stun_server.domain) > 0)
		stun_server_dialog->server =
			gui_new_cell_entry(
				table, ua.stun_server.domain, 1, 0);
	else
		stun_server_dialog->server =
			gui_new_cell_entry(
				table, ua.stun_server.host, 1, 0);

	return stun_server_dialog;
}

outbound_proxy_dialog_t
gui_set_outbound_proxy_dialog_init(GtkWidget *parent)
{
	outbound_proxy_dialog_t outbound_proxy_dialog;
	GtkWidget *table;
	GtkObject *adjustment;

	outbound_proxy_dialog = malloc(sizeof(struct outbound_proxy_dialog));
	if (outbound_proxy_dialog == NULL)
		return NULL;
	
	outbound_proxy_dialog->dialog = gtk_dialog_new_with_buttons(
		"Set Outbound Proxy",
		GTK_WINDOW(parent),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);

	table = gui_new_table(
		(GTK_DIALOG(outbound_proxy_dialog->dialog))->vbox, 2, 2);

	gui_new_cell_label(table, "Host", 0, 0, 1.0);
	gui_new_cell_label(table, "Port", 0, 1, 1.0);

	/* Host entry */
	if (strlen(ua.outbound_proxy.domain) > 0)
		outbound_proxy_dialog->host =
			gui_new_cell_entry(
				table, ua.outbound_proxy.domain, 1, 0);
	else
		outbound_proxy_dialog->host =
			gui_new_cell_entry(
				table, ua.outbound_proxy.host, 1, 0);

	/* Port entry */
	if (ua.outbound_proxy.port < 0)
		adjustment = gtk_adjustment_new(
			SIP_DEFAULT_PORT, 0, 65535, 1, 10, 10);
	else
		adjustment = gtk_adjustment_new(
			ua.outbound_proxy.port, 0, 65535, 1, 10, 10);

	outbound_proxy_dialog->port =
		gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_widget_show(outbound_proxy_dialog->port);
	gtk_table_attach(
		GTK_TABLE(table), outbound_proxy_dialog->port,
		1, 2, 1, 2,
		(GtkAttachOptions) GTK_EXPAND | GTK_FILL,
		(GtkAttachOptions) GTK_FILL, 0, 0);

	return outbound_proxy_dialog;
}

soundcard_device_dialog_t
gui_change_media_device_dialog_init(GtkWidget *parent)
{
	soundcard_device_dialog_t soundcard_device_dialog;
	GtkWidget *table;

	soundcard_device_dialog =
		malloc(sizeof(struct soundcard_device_dialog));
	if (soundcard_device_dialog == NULL)
		return NULL;
	
	soundcard_device_dialog->dialog = gtk_dialog_new_with_buttons(
		"Change Media Device",
		GTK_WINDOW(parent),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);

	table = gui_new_table(
		(GTK_DIALOG(soundcard_device_dialog->dialog))->vbox, 1, 2);

	gui_new_cell_label(table, "Device", 0, 0, 1.0);
	soundcard_device_dialog->device =
		gui_new_cell_entry(table, ua.soundcard.device, 1, 0);

	return soundcard_device_dialog;
}

soundcard_device_dialog_t
gui_change_ringtone_device_dialog_init(GtkWidget *parent)
{
	soundcard_device_dialog_t soundcard_device_dialog;
	GtkWidget *table;

	soundcard_device_dialog =
		malloc(sizeof(struct soundcard_device_dialog));
	if (soundcard_device_dialog == NULL)
		return NULL;
	
	soundcard_device_dialog->dialog = gtk_dialog_new_with_buttons(
		"Change Ringtone Device",
		GTK_WINDOW(parent),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);

	table = gui_new_table(
		(GTK_DIALOG(soundcard_device_dialog->dialog))->vbox, 1, 2);

	gui_new_cell_label(table, "Device", 0, 0, 1.0);
	soundcard_device_dialog->device =
		gui_new_cell_entry(table, ua.ringtone.device, 1, 0);

	return soundcard_device_dialog;
}

void
gui_change_ringtone_file_dialog_init()
{
	GtkWidget *file_selector;

	file_selector = gtk_file_selection_new("Choose Ringtone .wav File");

	g_signal_connect(GTK_FILE_SELECTION(file_selector)->ok_button,
		"clicked", G_CALLBACK(callback_ringtone_set_file),
		file_selector);

	g_signal_connect_swapped(
		GTK_FILE_SELECTION(file_selector)->ok_button,
		"clicked", G_CALLBACK(gtk_widget_destroy), file_selector);
	g_signal_connect_swapped(
		GTK_FILE_SELECTION(file_selector)->cancel_button,
		"clicked", G_CALLBACK(gtk_widget_destroy), file_selector);

	gtk_dialog_run(GTK_DIALOG(file_selector));
}
