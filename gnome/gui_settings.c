#include "gui.h"

GtkWidget *settings_dialog = NULL;
GtkWidget *settings_provider_host;
GtkWidget *settings_provider_port;
GtkWidget *settings_provider_user;
GtkWidget *settings_provider_password;
GtkWidget *settings_stun_server;
GtkWidget *settings_outbound_proxy_host;
GtkWidget *settings_outbound_proxy_port;
GtkWidget *settings_media_device;
GtkWidget *settings_ringtone_device;
GtkWidget *settings_ringtone_file;

static GtkWidget *
gui_settings_frame_init(GtkWidget *parent, char *title)
{
	GtkWidget *frame, *label, *vbox;
	char s[128];

	frame = gtk_frame_new(NULL);
	gtk_widget_show(frame);
	label = gtk_label_new(NULL);
	gtk_widget_show(label);
	memset(s, 0, 128);
	sprintf(s, "<b>%s</b>", title);
	gtk_label_set_markup(GTK_LABEL(label), s);
	gtk_frame_set_label_widget(GTK_FRAME(frame), label);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(parent), frame);
#if 0
        gtk_box_pack_start(
		GTK_BOX(parent), frame, FALSE, FALSE, BORDER_WIDTH);
#endif
	vbox = gtk_vbox_new(FALSE, BORDER_WIDTH);
	gtk_container_set_border_width(
		GTK_CONTAINER(vbox), 2 * BORDER_WIDTH);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return vbox;
}

static GtkWidget *
gui_settings_line(GtkWidget *parent, char *line)
{
	GtkWidget *align, *label;

	align = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(parent), align);

	label = gtk_label_new(line);
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(align), label);

	return label;
}

static GtkWidget *
gui_settings_buttons_hbox_init(GtkWidget *parent)
{
	GtkWidget *align, *hbox;

	align = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(parent), align);

	hbox = gtk_hbox_new(FALSE, BORDER_WIDTH);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(align), hbox);

	return hbox;
}

static GtkWidget *
gui_settings_button_init(GtkWidget *parent, char *text)
{
	GtkWidget *button;

	button = gtk_button_new_with_label(text);
	gtk_widget_show(button);
	gtk_container_add(GTK_CONTAINER(parent), button);

	return button;
}

static void
gui_settings_provider_frame_init(GtkWidget *parent)
{
	GtkWidget *vbox, *hbox, *button;
	char s[128];

	vbox = gui_settings_frame_init(parent, "Service Provider");

	memset(s, 0, 128);
	sprintf(s, "Host: ");
	if (strlen(ua.reg_uri.endpoint.domain) > 0)
		sprintf(s + strlen(s), "%s", ua.reg_uri.endpoint.domain);
	else if (strlen(ua.reg_uri.endpoint.host) > 0)
		sprintf(s + strlen(s), "%s", ua.reg_uri.endpoint.host);
	else
		sprintf(s + strlen(s), "none");
	settings_provider_host = gui_settings_line(vbox, s);

	memset(s, 0, 128);
	sprintf(s, "Port: %d", ua.reg_uri.endpoint.port);
	settings_provider_port = gui_settings_line(vbox, s);

	memset(s, 0, 128);
	sprintf(s, "User: ");
	if (strlen(ua.reg_uri.user) > 0)
		sprintf(s + strlen(s), "%s", ua.reg_uri.user);
	else
		sprintf(s + strlen(s), "none");
	settings_provider_user = gui_settings_line(vbox, s);

	memset(s, 0, 128);
	sprintf(s, "Password: ");
	if (strlen(ua.reg_uri.passwd) > 0) {
		int i, len;

		for (len = strlen(ua.reg_uri.passwd), i = 0; i < len; i++)
			sprintf(s + strlen(s), "%s", "*");
	} else
		sprintf(s + strlen(s), "none");
	settings_provider_password = gui_settings_line(vbox, s);

	hbox = gui_settings_buttons_hbox_init(vbox);
	button = gui_settings_button_init(hbox, "Change");
	g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_settings_provider_change_event, NULL);
}

static void
gui_settings_local_endpoint_frame_init(GtkWidget *parent)
{
	GtkWidget *vbox;
	char s[128];

	vbox = gui_settings_frame_init(parent, "Local Endpoint");

	memset(s, 0, 128);
	sprintf(s, "Host: %s", ua.local_endpoint.host);
	gui_settings_line(vbox, s);

	memset(s, 0, 128);
	sprintf(s, "SIP port: %d", ua.local_endpoint.port);
	gui_settings_line(vbox, s);

	memset(s, 0, 128);
	sprintf(s, "RTP port: %d", ua.rtp_port);
	gui_settings_line(vbox, s);
}

static void
gui_settings_visible_endpoint_frame_init(GtkWidget *parent)
{
	GtkWidget *vbox;
	char s[128];

	vbox = gui_settings_frame_init(parent, "Visible Endpoint");

	memset(s, 0, 128);
	sprintf(s, "Host: ");
	if (strlen(ua.visible_endpoint.host) > 0)
		sprintf(s + strlen(s), "%s", ua.visible_endpoint.host);
	else
		sprintf(s + strlen(s), "none");
	gui_settings_line(vbox, s);

	memset(s, 0, 128);
	sprintf(s, "SIP port: ");
	if (ua.visible_endpoint.port >= 0)
		sprintf(s + strlen(s), "%d", ua.visible_endpoint.port);
	else
		sprintf(s + strlen(s), "none");
	gui_settings_line(vbox, s);

	memset(s, 0, 128);
	sprintf(s, "RTP port: ");
	if (ua.rtp.visible.port >= 0)
		sprintf(s + strlen(s), "%d", ua.rtp.visible.port);
	else
		sprintf(s + strlen(s), "none");
	gui_settings_line(vbox, s);
}

static void
gui_settings_stun_server_frame_init(GtkWidget *parent)
{
	GtkWidget *vbox, *hbox, *button;
	char s[128];

	vbox = gui_settings_frame_init(parent, "STUN");

	memset(s, 0, 128);
	sprintf(s, "Server: ");
	if (strlen(ua.stun_server.domain) > 0)
		sprintf(s + strlen(s), "%s", ua.stun_server.domain);
	else if (strlen(ua.stun_server.host) > 0)
		sprintf(s + strlen(s), "%s", ua.stun_server.host);
	else
		sprintf(s + strlen(s), "none");
	settings_stun_server = gui_settings_line(vbox, s);

	hbox = gui_settings_buttons_hbox_init(vbox);

	button = gui_settings_button_init(hbox, "Clear");
	g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_settings_stun_server_clear_event, NULL);

	button = gui_settings_button_init(hbox, "Set");
	g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_settings_stun_server_set_event, NULL);
}

static void
gui_settings_outbound_proxy_frame_init(GtkWidget *parent)
{
	GtkWidget *vbox, *hbox, *button;
	char s[128];

	vbox = gui_settings_frame_init(parent, "Outbound Proxy");

	memset(s, 0, 128);
	sprintf(s, "Host: ");
	if (strlen(ua.outbound_proxy.domain) > 0)
		sprintf(s + strlen(s), "%s", ua.outbound_proxy.domain);
	else if (strlen(ua.outbound_proxy.host) > 0)
		sprintf(s + strlen(s), "%s", ua.outbound_proxy.host);
	else
		sprintf(s + strlen(s), "none");
	settings_outbound_proxy_host = gui_settings_line(vbox, s);

	memset(s, 0, 128);
	sprintf(s, "Port: ");
	if (ua.outbound_proxy.port >= 0)
		sprintf(s + strlen(s), "%d", ua.outbound_proxy.port);
	else
		sprintf(s + strlen(s), "none");
	settings_outbound_proxy_port = gui_settings_line(vbox, s);

	hbox = gui_settings_buttons_hbox_init(vbox);

	button = gui_settings_button_init(hbox, "Clear");
	g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_settings_outbound_proxy_clear_event,
		NULL);

	button = gui_settings_button_init(hbox, "Set");
	g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_settings_outbound_proxy_set_event,
		NULL);
}

static void
gui_settings_media_device_frame_init(GtkWidget *parent)
{
	GtkWidget *vbox, *hbox, *button;
	char s[128];

	vbox = gui_settings_frame_init(parent, "Media Device");

	memset(s, 0, 128);
	sprintf(s, "Device: %s", ua.soundcard.device);
	settings_media_device = gui_settings_line(vbox, s);

	hbox = gui_settings_buttons_hbox_init(vbox);
	button = gui_settings_button_init(hbox, "Change");
	g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_settings_media_device_change_event,
		NULL);
}

static void
gui_settings_ringtone_device_frame_init(GtkWidget *parent)
{
	GtkWidget *vbox, *hbox, *button;
	char s[128];

	vbox = gui_settings_frame_init(parent, "Ringtone Device");

	memset(s, 0, 128);
	sprintf(s, "Device: %s", ua.ringtone.device);
	settings_ringtone_device = gui_settings_line(vbox, s);

	hbox = gui_settings_buttons_hbox_init(vbox);
	button = gui_settings_button_init(hbox, "Change");
	g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_settings_ringtone_device_change_event,
		NULL);
}

static void
gui_settings_ringtone_file_frame_init(GtkWidget *parent)
{
	GtkWidget *vbox, *hbox, *button;
	char s[128];

	vbox = gui_settings_frame_init(parent, "Ringtone File");

	memset(s, 0, 128);
	sprintf(s, "File: ");
	if (strlen(ua.ringtone.file) > 0)
		sprintf(s + strlen(s), "%s", ua.ringtone.file);
	else
		sprintf(s + strlen(s), "none");
	settings_ringtone_file = gui_settings_line(vbox, s);

	hbox = gui_settings_buttons_hbox_init(vbox);
	button = gui_settings_button_init(hbox, "Change");
	g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_settings_ringtone_file_change_event,
		NULL);
}

void
gui_settings_dialog_init()
{
	GtkWidget *hbox, *align, *vbox, *sep;

	settings_dialog = gtk_dialog_new_with_buttons(
		"Settings",
		GTK_WINDOW(main_window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CLOSE,
		GTK_RESPONSE_ACCEPT,
		NULL);

	hbox = gtk_hbox_new(FALSE, BORDER_WIDTH);
	gtk_container_set_border_width(
		GTK_CONTAINER(hbox), 2 * BORDER_WIDTH);
	gtk_widget_show(hbox);
	gtk_container_add(
		GTK_CONTAINER((GTK_DIALOG(settings_dialog))->vbox), hbox);

	align = gtk_alignment_new(0, 0, 0, 0);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(hbox), align);

	vbox = gtk_vbox_new(FALSE, BORDER_WIDTH);
	gtk_container_set_border_width(
		GTK_CONTAINER(vbox), 2 * BORDER_WIDTH);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(align), vbox);

	gui_settings_provider_frame_init(vbox);
	gui_settings_local_endpoint_frame_init(vbox);
	gui_settings_visible_endpoint_frame_init(vbox);
	gui_settings_stun_server_frame_init(vbox);

	sep = gtk_vseparator_new();
	gtk_widget_show(sep);
	gtk_container_add(GTK_CONTAINER(hbox), sep);

	align = gtk_alignment_new(0, 0, 0, 0);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(hbox), align);

	vbox = gtk_vbox_new(FALSE, BORDER_WIDTH);
	gtk_container_set_border_width(
		GTK_CONTAINER(vbox), 2 * BORDER_WIDTH);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(align), vbox);

	gui_settings_outbound_proxy_frame_init(vbox);
	gui_settings_media_device_frame_init(vbox);
	gui_settings_ringtone_device_frame_init(vbox);
	gui_settings_ringtone_file_frame_init(vbox);
}
