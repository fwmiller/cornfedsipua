#include "dns.h"
#include "gui.h"
#include "http.h"
#include "log.h"

static int
validate_provider_host(char *host, int hostlen, char *domain, char *ipaddr)
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

static int
validate_provider_password(char *password, char *confirm)
{
	if (strcmp(password, confirm) == 0)
		return 0;

	return (-1);
}

static void
settings_provider_update()
{
	char s[128];

	memset(s, 0, 128);
	sprintf(s, "Host: ");
	if (strlen(ua.reg_uri.endpoint.domain) > 0)
		sprintf(s + strlen(s), "%s", ua.reg_uri.endpoint.domain);
	else if (strlen(ua.reg_uri.endpoint.host) > 0)
		sprintf(s + strlen(s), "%s", ua.reg_uri.endpoint.host);
	else
		sprintf(s + strlen(s), "none");
	gtk_label_set_text(GTK_LABEL(settings_provider_host), s);

	memset(s, 0, 128);
	sprintf(s, "Port: %d", ua.reg_uri.endpoint.port);
	gtk_label_set_text(GTK_LABEL(settings_provider_port), s);

	memset(s, 0, 128);
	sprintf(s, "User: ");
	if (strlen(ua.reg_uri.user) > 0)
		sprintf(s + strlen(s), "%s", ua.reg_uri.user);
	else
		sprintf(s + strlen(s), "none");
	gtk_label_set_text(GTK_LABEL(settings_provider_user), s);

	memset(s, 0, 128);
	sprintf(s, "Password: ");
	if (strlen(ua.reg_uri.passwd) > 0) {
		int i, len;

		for (len = strlen(ua.reg_uri.passwd), i = 0; i < len; i++)
			sprintf(s + strlen(s), "%s", "*");
	} else
		sprintf(s + strlen(s), "none");
	gtk_label_set_text(GTK_LABEL(settings_provider_password), s);
}

gboolean
callback_settings_provider_change_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	provider_dialog_t provider_dialog;
	gint button;

	log_msg(LOG_INFO, "Change Service Provider button pressed");

	provider_dialog = gui_change_provider_dialog_init(settings_dialog);
	button = gtk_dialog_run(GTK_DIALOG(provider_dialog->dialog));
	if (button == GTK_RESPONSE_ACCEPT) {
		char domain[128], ipaddr[128];
		char *confirm, *host, *password, *port, *user;
		int result, state;

		log_msg(LOG_INFO, "Service Provider information modified");

		host = (char *) gtk_entry_get_text(
			GTK_ENTRY(provider_dialog->host));

		memset(domain, 0, 128);
		memset(ipaddr, 0, 128);
		result = validate_provider_host(
			host, strlen(host), domain, ipaddr);
		if (result < 0) {
			/* Fail if bad host address specified */
			gui_message(provider_dialog->dialog,
				    GTK_MESSAGE_ERROR,
				    "Bad host address");
			goto change_service_provider_done;
		}
		port = (char *) gtk_entry_get_text(
			GTK_ENTRY(provider_dialog->port));

		user = (char *) gtk_entry_get_text(
			GTK_ENTRY(provider_dialog->user));
		if (strlen(user) == 0) {
			/* Warning if no user specified */
			gui_message(provider_dialog->dialog,
				    GTK_MESSAGE_WARNING,
				    "No user specified");
		}
		password = (char *) gtk_entry_get_text(
			GTK_ENTRY(provider_dialog->password));

		confirm = (char *) gtk_entry_get_text(
			GTK_ENTRY(provider_dialog->confirm));

		result = validate_provider_password(password, confirm);
		if (result < 0) {
			/* Fail if password and confirm do not match */
			gui_message(provider_dialog->dialog, 
				    GTK_MESSAGE_ERROR,
				    "Password and confirm do not match");
			goto change_service_provider_done;
		}
		if (strlen(password) == 0) {
			/* Warning if no password specified */
			gui_message(provider_dialog->dialog, 
				    GTK_MESSAGE_WARNING,
				    "No password specified");
		}
		/* Record new Service Provider */
		sip_uri_init(&(ua.reg_uri));
		ua.reg_uri.prefix = SIPU_SIP;
		strcpy(ua.reg_uri.user, user);
		strcpy(ua.reg_uri.passwd, password);
		strcpy(ua.reg_uri.endpoint.domain, domain);
		strcpy(ua.reg_uri.endpoint.host, ipaddr);
		ua.reg_uri.endpoint.port = atoi(port);

		/* Update Service Provider information on Settings dialog */
		settings_provider_update();

		/* Start new registration */
		state = sip_dialog_get_state(ua.registration);
		if (state == SIPS_IDLE) {
#if 0
			reg_set_expires(NULL, REGISTER_INTERVAL);
#endif
			sip_uac_register(&ua);
			sip_timer_start(&(ua.registration->timers),
					SIPT_REGISTER_RETR,
					1000 * SIP_TIMER_1);
		}
	}

change_service_provider_done:
	gtk_widget_destroy(provider_dialog->dialog);
	free(provider_dialog);

	return TRUE;
}
