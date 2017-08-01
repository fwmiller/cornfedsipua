#ifndef __CLI_H
#define __CLI_H

#include "sip.h"

extern struct sip_user_agent ua;
extern int reg_interval;
extern int reg_buffer;
extern int reg_norereg;
extern int reg_expires;
extern struct timeval reg_end;

void about(void);
void cli(void);
void cli_about(char *cmdline);
void cli_answer(char *cmdline);
void cli_debug(char *cmdline);
void cli_dial(char *cmdline);
void cli_dialog(char *cmdline);
void cli_dnd(char *cmdline);
void cli_dns(char *cmdline);
int cli_dtmf_scan(char *s);
void cli_dtmf_send(char *s);
void cli_hangup(char *cmdline);
void cli_help(char *cmdline);
void cli_history(char *cmdline);
void cli_local(char *cmdline);
void cli_log(char *cmdline);
void cli_nat(char *cmdline);
void cli_outbound_proxy(char *cmdline);
void cli_play_wav(char *cmdline);
void cli_record_wav(char *cmdline);
void cli_refuse(char *cmdline);
void cli_register(char *cmdline);
void cli_remote(char *cmdline);
void cli_reset(char *cmdline);
void cli_ringtone(char *cmdline);
void cli_rtp(char *cmdline);
void cli_soundcard(char *cmdline);
void cli_wav(char *cmdline);
void cli_volume(char *cmdline);

void cli_cmd_history_init(void);
void cli_cmd_history_dump(void);
void cli_cmd_history_push(char *cmd);
void cli_cmd_history_substitute(int cmd, char *cmdline);

int cli_set_host(char *cmdline, char *s, int *pos, sip_uri_t uri);

void uac_canceled(void);
void uac_connect(void);
void uac_completed(void);
int uac_register_prompt_for_user(char *user, int len);
void uac_register_failed(char *host);
void uac_timeout(void);
int uas_request_uri_lookup(char *host);
void uas_cancel(sip_dialog_t dialog);
void uas_ringback(sip_dialog_t dialog);
void uas_completed(void);
void uas_connect(sip_dialog_t dialog);
void uas_hangup(sip_dialog_t dialog);

int reg_get_interval(void);
int reg_get_expires(char *host);
void reg_set_expires(char *host, int expires);
void reg_get_auth_user(char *host, char *auth_user);
void reg_thread_func(void);

void ua_get_rtt(char *host, struct timeval *rtt);
void ua_set_rtt(char *host, struct timeval *rtt);

void ua_history_update(char *s);
void ua_history_clear(void);

void help_init(void);

#endif
