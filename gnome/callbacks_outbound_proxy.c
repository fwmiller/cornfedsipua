#include "dns.h"
#include "gui.h"
#include "http.h"
#include "log.h"

static int
validate_outbound_proxy(char *host, int hostlen, char *domain, char *ipaddr)
{
	char *ipaddrstr;
	int ipaddrlen, result;

	result = find_ip_address(
		host, hostlen, &ipaddrstr, &ipaddrlen);
	if (result == 0) {
		strncpy(ipaddr, ipaddrstr, ipaddrlen);
		return 0;

	} else if (dns_avail(&ua)) {
		char ipaddrstr[128];

		result = dns_gethostbyname(host, ipaddrstr, 128);
		if (result >= 0) {
			strcpy(domain, host);
			strcpy(ipaddr, ipaddrstr);
			return 0;
		}
	}
	return (-1);
}

static void
settings_outbound_proxy_update()
{
	char s[128];

	memset(s, 0, 128);
	sprintf(s, "Host: ");
	if (strlen(ua.outbound_proxy.domain) > 0)
		sprintf(s + strlen(s), "%s", ua.outbound_proxy.domain);
	else if (strlen(ua.outbound_proxy.host) > 0)
		sprintf(s + strlen(s), "%s", ua.outbound_proxy.host);
	else
		sprintf(s + strlen(s), "none");
	gtk_label_set_text(GTK_LABEL(settings_outbound_proxy_host), s);

	memset(s, 0, 128);
	sprintf(s, "Port: ");
	if (ua.outbound_proxy.port >= 0)
		sprintf(s + strlen(s), "%d", ua.outbound_proxy.port);
	else
		sprintf(s + strlen(s), "none");
	gtk_label_set_text(GTK_LABEL(settings_outbound_proxy_port), s);
}

gboolean
callback_settings_outbound_proxy_clear_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	log_msg(LOG_INFO, "Clear Outbound Proxy button pressed");

	ipendpoint_init(&(ua.outbound_proxy));

	/* Update Outbound Proxy information on Settings Dialog */
	settings_outbound_proxy_update();

	return TRUE;
}

gboolean
callback_settings_outbound_proxy_set_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	outbound_proxy_dialog_t outbound_proxy_dialog;
	gint button;

	log_msg(LOG_INFO, "Set Outbound Proxy button pressed");

	outbound_proxy_dialog =
		gui_set_outbound_proxy_dialog_init(settings_dialog);
	button = gtk_dialog_run(GTK_DIALOG(outbound_proxy_dialog->dialog));
	if (button == GTK_RESPONSE_ACCEPT) {
		char domain[128], ipaddr[128];
		char *host, *port;
		int result;

		log_msg(LOG_INFO, "Outbound Proxy modified");

		host = (char *) gtk_entry_get_text(
			GTK_ENTRY(outbound_proxy_dialog->host));
		port = (char *) gtk_entry_get_text(
			GTK_ENTRY(outbound_proxy_dialog->port));

		memset(domain, 0, 128);
		memset(ipaddr, 0, 128);
		result = validate_outbound_proxy(
			host, strlen(host), domain, ipaddr);
		if (result < 0) {
			/*
			 * Fail if bad outbound proxy host address
			 * specified
			 */
			gui_message(outbound_proxy_dialog->dialog,
				GTK_MESSAGE_ERROR,
				"Bad host address");
				goto change_outbound_proxy_done;
		}
		/* Record new Outbound Proxy */
		ipendpoint_init(&(ua.outbound_proxy));
		strcpy(ua.outbound_proxy.domain, domain);
		strcpy(ua.outbound_proxy.host, ipaddr);
		ua.outbound_proxy.port = atoi(port);

		/* Update Outbound Proxy information on Settings Dialog */
		settings_outbound_proxy_update();
	}

change_outbound_proxy_done:
	gtk_widget_destroy(outbound_proxy_dialog->dialog);
	free(outbound_proxy_dialog);

	return TRUE;
}
