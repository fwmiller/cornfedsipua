#ifndef __GUI_H
#define __GUI_H

#include <gnome.h>
#include "sip.h"

#define BORDER_WIDTH	3

struct provider_dialog {
	GtkWidget *dialog;
	GtkWidget *host;
	GtkWidget *port;
	GtkWidget *user;
	GtkWidget *password;
	GtkWidget *confirm;
};

struct stun_server_dialog {
	GtkWidget *dialog;
	GtkWidget *server;
};

struct outbound_proxy_dialog {
	GtkWidget *dialog;
	GtkWidget *host;
	GtkWidget *port;
};

struct soundcard_device_dialog {
	GtkWidget *dialog;
	GtkWidget *device;
};

typedef struct provider_dialog *provider_dialog_t;
typedef struct stun_server_dialog *stun_server_dialog_t;
typedef struct outbound_proxy_dialog *outbound_proxy_dialog_t;
typedef struct soundcard_device_dialog *soundcard_device_dialog_t;

extern struct sip_user_agent ua;
extern int reg_expires;
extern struct timeval reg_end;

extern GtkWidget *main_window;
extern GtkWidget *dialed_number;
extern GtkWidget *dial_button;
extern GtkWidget *hangup_button;
extern GtkWidget *status_bar;
extern guint status_bar_context_id;
extern GtkWidget *settings_dialog;
extern GtkWidget *settings_provider_host;
extern GtkWidget *settings_provider_port;
extern GtkWidget *settings_provider_user;
extern GtkWidget *settings_provider_password;
extern GtkWidget *settings_stun_server;
extern GtkWidget *settings_outbound_proxy_host;
extern GtkWidget *settings_outbound_proxy_port;
extern GtkWidget *settings_media_device;
extern GtkWidget *settings_ringtone_device;
extern GtkWidget *settings_ringtone_file;

void status_callback(char *msg);
void status(char *msg);

void gui_init();
void gui_message(GtkWidget *parent, GtkMessageType type, char *msg);
void gui_settings_dialog_init();
provider_dialog_t gui_change_provider_dialog_init(GtkWidget *parent);
stun_server_dialog_t gui_set_stun_server_dialog_init(GtkWidget *parent);

outbound_proxy_dialog_t
	gui_set_outbound_proxy_dialog_init(GtkWidget *parent);

soundcard_device_dialog_t
	gui_change_media_device_dialog_init(GtkWidget *parent);

soundcard_device_dialog_t
	gui_change_ringtone_device_dialog_init(GtkWidget *parent);

void gui_change_ringtone_file_dialog_init();

gboolean callback_delete_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_dialpad_button_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_settings_provider_change_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_settings_stun_server_clear_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_settings_stun_server_set_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_settings_outbound_proxy_clear_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_settings_outbound_proxy_set_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_settings_media_device_change_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_settings_ringtone_device_change_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean callback_settings_ringtone_file_change_event(
	GtkWidget *widget, GdkEvent *event, gpointer user_data);

void callback_ringtone_set_file(GtkWidget *widget, gpointer user_data);

void uac_canceled();
void uac_connect();
void uac_completed();
int uac_register_prompt_for_user(char *user, int len);
void uac_register_failed(char *host);
void uac_timeout();
int uas_request_uri_lookup(char *host);
void uas_cancel(sip_dialog_t dialog);
void uas_ringback(sip_dialog_t dialog);
void uas_completed();
void uas_connect(sip_dialog_t dialog);
void uas_hangup(sip_dialog_t dialog);
int reg_get_interval();
int reg_get_expires(char *host);
void reg_set_expires(char *host, int expires);
void reg_get_auth_user(char *host, char *auth_user);
void reg_thread_func();
void ua_get_rtt(char *host, struct timeval *rtt);
void ua_set_rtt(char *host, struct timeval *rtt);
void ua_history_update(char *s);
void ua_history_clear();

#endif
