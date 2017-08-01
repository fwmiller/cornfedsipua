#include "dns.h"
#include "gui.h"
#include "http.h"
#include "log.h"

static int
validate_stun_server(char *server, int serverlen, char *domain, char *ipaddr)
{
	char *ipaddrstr;
	int ipaddrlen, result;

	result = find_ip_address(
		server, serverlen, &ipaddrstr, &ipaddrlen);
	if (result == 0) {
		strncpy(ipaddr, ipaddrstr, ipaddrlen);
		return 0;

	} else if (dns_avail(&ua)) {
		char ipaddrstr[128];

		result = dns_gethostbyname(server, ipaddrstr, 128);
		if (result >= 0) {
			strcpy(domain, server);
			strcpy(ipaddr, ipaddrstr);
			return 0;
		}
	}
	return (-1);
}

static void
settings_stun_server_update()
{
	char s[128];

	memset(s, 0, 128);
	sprintf(s, "Server: ");
	if (strlen(ua.stun_server.domain) > 0)
		sprintf(s + strlen(s), "%s", ua.stun_server.domain);
	else if (strlen(ua.stun_server.host) > 0)
		sprintf(s + strlen(s), "%s", ua.stun_server.host);
	else
		sprintf(s + strlen(s), "none");
	gtk_label_set_text(GTK_LABEL(settings_stun_server), s);
}

gboolean
callback_settings_stun_server_clear_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	log_msg(LOG_INFO, "Clear STUN Server button pressed");

	ipendpoint_init(&(ua.stun_server));

	/* Update STUN Server information on Settings dialog */
	settings_stun_server_update();

	return TRUE;
}

gboolean
callback_settings_stun_server_set_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	stun_server_dialog_t stun_server_dialog;
	gint button;

	log_msg(LOG_INFO, "Set STUN Server button pressed");

	stun_server_dialog =
		gui_set_stun_server_dialog_init(settings_dialog);
	button = gtk_dialog_run(GTK_DIALOG(stun_server_dialog->dialog));
	if (button == GTK_RESPONSE_ACCEPT) {
		struct timeval now, dur;
		struct timezone tz;
		char domain[128], ipaddr[128];
		char *server;
		int result;

		log_msg(LOG_INFO, "STUN Server modified");

		server = (char *) gtk_entry_get_text(
			GTK_ENTRY(stun_server_dialog->server));

		memset(domain, 0, 128);
		memset(ipaddr, 0, 128);
		result = validate_stun_server(
			server, strlen(server), domain, ipaddr);
		if (result < 0) {
			/* Fail if bad server address specified */
			gui_message(stun_server_dialog->dialog,
				    GTK_MESSAGE_ERROR,
				    "Bad server address");
			goto change_stun_server_done;
		}
		/* Record new STUN Server */
		ipendpoint_init(&(ua.stun_server));
		strcpy(ua.stun_server.domain, domain);
		strcpy(ua.stun_server.host, ipaddr);

		/* Update STUN Server information on Settings dialog */
		settings_stun_server_update();

		/* Set initial STUN keepalive timeout */
		dur.tv_sec = 2;
		dur.tv_usec = 0;
		gettimeofday(&now, &tz);
		timeradd(&now, &dur, &(ua.stun_keepalive_end));
	}

change_stun_server_done:
	gtk_widget_destroy(stun_server_dialog->dialog);
	free(stun_server_dialog);

	return TRUE;
}
