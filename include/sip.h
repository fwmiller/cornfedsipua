#ifndef __CORNFED_SIP_UA_H
#define __CORNFED_SIP_UA_H

#include <sys/time.h>
#include "ipendpoint.h"

/* Max routes in the route set */
#define MAX_ROUTES		32

/* SIP URI Prefix */
#define SIPU_NULL		0
#define SIPU_SIP		1
#define SIPU_SIPS		2

/* SIP Method */
#define SIPM_UNDEF		0
#define SIPM_INVITE		1
#define SIPM_ACK		2
#define SIPM_OPTIONS		3
#define SIPM_BYE		4
#define SIPM_CANCEL		5
#define SIPM_REGISTER		6

/* SIP User Agent flags */
#define SUAF_DEBUG		0x01
#define SUAF_NO_DNS		0x02
#define SUAF_LOCAL_NAT		0x04
#define SUAF_RINGING		0x08
#define SUAF_RECORDING		0x10

/* SIP Dialog State */
#define SIPS_IDLE		0
#define SIPS_BUSY		1
#define SIPS_UAC_REGISTERING	2
#define SIPS_UAC_CALLING	3
#define SIPS_UAC_PROCEEDING	4
#define SIPS_UAC_CANCELING	5
#define SIPS_UAC_TRYING		6
#define SIPS_UAS_PROCEEDING	7
#define SIPS_UAS_COMPLETED	8
#define SIPS_UAS_ACK_WAIT	9
#define SIPS_CONNECTED		10

#define SIP_TIMER_1		500	/* milliseconds */

/* SIP Timers */
#define SIPT_NULL		0
#define SIPT_INVITE_RETR	1
#define SIPT_RESPONSE_RETR	2
#define SIPT_BYE_RETR		3
#define SIPT_CANCEL_RETR	4
#define SIPT_REGISTER_RETR	5

/* SIP Authorization Types */
#define SIPA_NULL		0
#define SIPA_PROXY		1
#define SIPA_WWW		2

/* SIP URI Transport Types */
#define SIPUT_UDP		0
#define SIPUT_TCP		1
#define SIPUT_TLS		2

typedef struct sip_uri *sip_uri_t;
typedef struct sip_via *sip_via_t;
typedef struct sip_route_set *sip_route_set_t;
typedef struct sip_timers *sip_timers_t;
typedef struct sip_dialog *sip_dialog_t;
typedef struct sip_user_agent *sip_user_agent_t;

#include "lws2sws.h"
#include "msglines.h"
#include "ringtone.h"
#include "rtp.h"

#include "soundcard.h"

#include "stun.h"
#include "wav.h"

struct sip_uri {
	int prefix;
	char user[BUFSIZE];
	char passwd[BUFSIZE];
	struct ipendpoint endpoint;
	int transport;
};

struct sip_via {
	sip_via_t prev, next;
	struct ipendpoint endpoint;
	char branch[BUFSIZE];
	char rport[BUFSIZE];
	char received[BUFSIZE];
	int transport;
};

struct sip_route_set {
	char buf[BUFSIZE];
	int routes;
	int current_route;
	int pos[MAX_ROUTES];
	int len[MAX_ROUTES];
};

struct sip_timers {
	int timer;
	struct timeval a_interval;
	struct timeval a_end;
	struct timeval b_interval;
	struct timeval b_end;
};

struct sip_dialog {
	sip_dialog_t prev, next;
	int state;
	struct timeval rtt;
	struct timeval rtt_start;
	struct sip_timers timers;
	struct sip_route_set route_set;
	sip_via_t via_hdrs;
	char call_id[BUFSIZE];
	char local_tag[BUFSIZE];
	char remote_tag[BUFSIZE];
	int local_seq;
	int remote_seq;
	struct sip_uri local_uri;
	struct sip_uri remote_uri;
	struct sip_uri reg_uri;
	struct sip_uri remote_target;
	char last_resp[BUFSIZE];
	int auth_type;
	char auth_nonce[BUFSIZE];
	char auth_realm[BUFSIZE];
	char auth_user[BUFSIZE];
	char auth_response[BUFSIZE];
	char authorization[BUFSIZE];
};

struct sip_user_agent {
	int flags;
	char cornfeddir[BUFSIZE];
	char configfile[BUFSIZE];
	char if_name[BUFSIZE];
	int sipfd;
	char local_netmask[BUFSIZE];
	struct ipendpoint local_endpoint;
	struct ipendpoint stun_server;
	struct ipendpoint visible_endpoint;
	struct ipendpoint outbound_proxy;
	struct sip_uri remote_uri;
	struct sip_uri reg_uri;
	struct timeval rtt;
	struct timeval stun_keepalive_interval;
	struct timeval stun_keepalive_end;

	/* Session dialog(s) */
	sip_dialog_t dialog;

	/* Registration dialog */
	sip_dialog_t registration;

	/* Client callbacks */
	void (*uac_canceled) (void);
	void (*uac_connect) (void);
	void (*uac_completed) (void);
	int (*uac_register_prompt_for_user) (char *user, int len);
	void (*uac_register_failed) (char *host);
	void (*uac_timeout) (void);
	int (*uas_request_uri_lookup) (char *host);
	void (*uas_cancel) (sip_dialog_t dialog);
	void (*uas_ringback) (sip_dialog_t dialog);
	void (*uas_completed) (void);
	void (*uas_connect) (sip_dialog_t dialog);
	void (*uas_hangup) (sip_dialog_t dialog);
	int (*reg_get_interval) (void);
	int (*reg_get_expires) (char *host);
	void (*reg_set_expires) (char *host, int expires);
	void (*reg_get_auth_user) (char *host, char *auth_user);
	void (*reg_thread_func) (void);
	void (*ua_get_rtt) (char *host, struct timeval *rtt);
	void (*ua_set_rtt) (char *host, struct timeval *rtt);
	void (*ua_history_update) (char *s);
	void (*ua_history_clear) (void);

	/* Ringing */
	struct timeval ringing_interval;

	/* RTP session */
	int rtp_port;
	struct rtp_session rtp;

	/* Media sound card device */
	struct soundcard soundcard;


	/* Ringtone sound card device */
	struct ringtone ringtone;

	/* Recording */
	char record_file[BUFSIZE];
	int recordfd;
	unsigned long record_cnt;

	/* .wav files */
	struct wav_rec_list wavs;
	struct timeval wav_end;
};

/* sip.c */
void *sip_thread(void *arg);
int sip_init_port(sip_user_agent_t ua, int port);
int sip_init(sip_user_agent_t ua);
int sip_send(sip_user_agent_t ua, char *host, int port, char *buf, int len);
int sip_get_send_host_port(sip_dialog_t dialog, char *host,
			   int *port, int dns);
char *sip_get_local_host(sip_user_agent_t ua, char *remote_host);
int sip_get_local_port(sip_user_agent_t ua, char *remote_host);

/* sip_dialog.c */
int sip_dialog_get_state(sip_dialog_t dialog);
void sip_dialog_set_state(sip_dialog_t dialog, int state);
void sip_dialog_stack_init(void);
void sip_dialog_push(sip_dialog_t dialog);
sip_dialog_t sip_dialog_pop(void);
void sip_dialog_init(sip_dialog_t dialog);
sip_dialog_t sip_dialog_list_find_remote(sip_uri_t uri, sip_dialog_t list);
void sip_dialog_list_insert_head(sip_dialog_t dialog, sip_dialog_t *list);
void sip_dialog_list_insert_tail(sip_dialog_t dialog, sip_dialog_t *list);
void sip_dialog_list_remove(sip_dialog_t dialog, sip_dialog_t *list);
void sip_dialog_rtt_init(sip_dialog_t dialog);
void sip_dialog_rtt_start(sip_dialog_t dialog);
void sip_dialog_rtt_stop(sip_dialog_t dialog);

/* sip_gen.c */
void sip_gen_branch(char *branch);
void sip_gen_call_id(char *call_id);
void sip_gen_tag(char *tag);
void sip_gen_register(sip_user_agent_t ua, char *buf, int len,
		      int contact, int expires);
void sip_gen_invite(sip_user_agent_t ua, sip_dialog_t dialog,
		    char *buf, int len);
void sip_gen_ack(sip_user_agent_t ua, sip_dialog_t dialog,
		 char *buf, int len);
char *sip_get_reason_phrase(int code);
void sip_gen_response(sip_user_agent_t ua, sip_dialog_t dialog, char *buf,
		      int len, int code, int cseqno, char *method, int sdp);
void sip_gen_bye(sip_user_agent_t ua, char *buf, int len);
void sip_gen_proxy_authorization(sip_user_agent_t ua, sip_dialog_t dialog,
				 char *buf, int len, char *realm, char *nonce,
				 char *user, char *response);
void sip_gen_www_authorization(sip_user_agent_t ua, sip_dialog_t dialog,
			       char *buf, int len, char *realm, char *nonce,
			       char *user, char *response);
void sip_gen_cancel(sip_user_agent_t ua, sip_dialog_t dialog,
		    char *buf, int len);

/* sip_parse.c */
int sip_parse_to_hdr(msglines_t msglines, sip_uri_t uri,
		     char *tag, int taglen, int dns);
int sip_parse_from_hdr(msglines_t msglines, sip_uri_t uri,
		       char *tag, int taglen, int dns);
int sip_parse_callid(msglines_t msglines, char *callid, int callidlen);
int sip_parse_cseq(msglines_t msglines, char *method, int methodlen);
int sip_parse_contact(msglines_t msglines, sip_uri_t uri, int dns);
int sip_parse_route_set(msglines_t msglines, sip_dialog_t dialog);
int sip_parse_via_hdrs(msglines_t msglines, sip_dialog_t dialog, int dns);

/* sip_recv.c */
void sip_recv(sip_user_agent_t ua, char *buf, int len);

/* sip_recv_request.c */
void sip_recv_request(sip_user_agent_t ua, msglines_t msglines);

/* sip_recv_response.c */
void sip_recv_response(sip_user_agent_t ua, msglines_t msglines);

/* sip_ringing.c */
void sip_ringing_thread_func(sip_user_agent_t ua);
void sip_ringing_start(sip_user_agent_t ua);
void sip_ringing_stop(sip_user_agent_t ua);
void sip_ringing_answer(sip_user_agent_t ua);

/* sip_route_set.c */
void sip_route_set_init(sip_route_set_t route_set);
void sip_route_set_append(char *route, sip_route_set_t route_set);
int sip_route_set_first(sip_route_set_t route_set, char *route);
int sip_route_set_next(sip_route_set_t route_set, char *route);

/* sip_timers.c */
void sip_timer_thread_func(sip_user_agent_t ua);
void sip_timer_init(sip_dialog_t dialog);
void sip_timer_start(sip_timers_t timers, int timer, int duration);
void sip_timer_cancel(sip_timers_t timers);

/* sip_uac_authenticate.c */
int sip_uac_authenticate(sip_user_agent_t ua, sip_dialog_t dialog,
			 msglines_t msglines, char *method, char *user);

/* sip_uac.c */
sip_dialog_t sip_uac_check_response(sip_user_agent_t ua, msglines_t msglines);
int sip_check_route_line(char *line, sip_route_set_t route_set);
int sip_uac_check_route_set(sip_dialog_t dialog, msglines_t msglines);
void sip_uac_check_remote_target(sip_dialog_t dialog,
				 msglines_t msglines, int dns);
void sip_uac_teardown(sip_user_agent_t ua, int code);

/* sip_uac_cancel.c */
int sip_uac_cancel(sip_user_agent_t ua);
void sip_uac_retransmit_cancel(sip_user_agent_t ua);
void sip_uac_canceling(sip_user_agent_t ua);

/* sip_uac_completed.c */
void sip_uac_completed(sip_user_agent_t ua, sip_dialog_t dialog,
		       msglines_t msglines, int code);

/* sip_uac_connect.c */
void sip_uac_connect(sip_user_agent_t ua, sip_dialog_t dialog,
		     msglines_t msglines);

/* sip_uac_hangup.c */
int sip_uac_hangup(sip_user_agent_t ua);
void sip_uac_retransmit_bye(sip_user_agent_t ua);

/* sip_uac_invite.c */
int sip_uac_invite(sip_user_agent_t ua, sip_uri_t to,
		   sip_uri_t from, sip_uri_t reg);
void sip_uac_retransmit_invite(sip_user_agent_t ua, sip_dialog_t dialog);

/* sip_uac_proceeding.c */
void sip_uac_proceeding(sip_user_agent_t ua, sip_dialog_t dialog,
			msglines_t msglines, int code);

/* sip_uac_register.c */
void sip_uac_register(sip_user_agent_t ua);
void sip_uac_retransmit_register(sip_user_agent_t ua);
void sip_uac_bad_register(sip_user_agent_t ua, msglines_t msglines, int code);
void sip_uac_unregister(sip_user_agent_t ua);

/* sip_uac_registered.c */
void sip_uac_registered(sip_user_agent_t ua, msglines_t msglines);

/* sip_uas_answer.c */
int sip_uas_answer(sip_user_agent_t ua);

/* sip_uas.c */
int sip_uas_check_mandatory_ack_hdrs(msglines_t msglines);
int sip_uas_check_mandatory_bye_hdrs(msglines_t msglines);
int sip_uas_check_mandatory_cancel_hdrs(msglines_t msglines);
int sip_uas_check_mandatory_invite_hdrs(msglines_t msglines);
int sip_uas_check_mandatory_options_hdrs(msglines_t msglines);
int sip_uas_match_dialog(sip_user_agent_t ua, msglines_t msglines);
int sip_uas_check_route_set(sip_dialog_t dialog, msglines_t msglines);
int sip_uas_check_via_hdrs(sip_user_agent_t ua, sip_dialog_t dialog,
			   msglines_t msglines);
int sip_uas_check_invite(sip_user_agent_t ua, msglines_t msglines);
int sip_uas_check_reinvite(sip_user_agent_t ua, msglines_t msglines);
int sip_uas_check_ack(sip_user_agent_t ua, msglines_t msglines);
int sip_uas_check_bye(sip_user_agent_t ua, msglines_t msglines);
int sip_uas_check_require(sip_user_agent_t ua, msglines_t msglines,
			  char *method);
int sip_uas_check_supported(sip_user_agent_t ua, msglines_t msglines);

/* sip_uas_cancel.c */
void sip_uas_cancel(sip_user_agent_t ua, msglines_t msglines);

/* sip_uas_hangup.c */
void sip_uas_hangup(sip_user_agent_t ua, msglines_t msglines);

/* sip_uas_invite.c */
void sip_uas_invite(sip_user_agent_t ua, msglines_t msglines);
void sip_uas_reinvite(sip_user_agent_t ua, msglines_t msglines);
void sip_uas_retransmit_response(sip_user_agent_t ua);

/* sip_uas_options.c */
void sip_uas_options(sip_user_agent_t ua, msglines_t msglines);

/* sip_uas_refuse.c */
void sip_uas_refuse(sip_user_agent_t ua);

/* sip_uas_response.c */
void sip_uas_method_not_allowed(sip_user_agent_t ua, msglines_t msglines);
void sip_uas_busy_here(sip_user_agent_t ua, msglines_t msglines);
void sip_uas_request_pending(sip_user_agent_t ua, msglines_t msglines);
void sip_uas_server_error(sip_user_agent_t ua, msglines_t msglines);
void sip_uas_not_acceptable(sip_user_agent_t ua, msglines_t msglines);
void sip_uas_call_leg_does_not_exist(sip_user_agent_t ua,
				     msglines_t msglines);

/* sip_uri.c */
void sip_uri_init(sip_uri_t uri);
int sip_uri_isset(sip_uri_t uri);
void sip_uri_parse(char *s, sip_uri_t uri, int dns);
void sip_uri_gen(sip_uri_t uri, char *s);
int sip_uri_cmp(sip_uri_t uri1, sip_uri_t uri2);

/* sip_user_agent.c */
void sip_user_agent_init(sip_user_agent_t ua);
void sip_user_agent_clear(sip_user_agent_t ua);
void sip_user_agent_clear_registration(sip_user_agent_t ua);
void sip_user_agent_clear_do_not_disturb(sip_user_agent_t ua);
void sip_user_agent_set_do_not_disturb(sip_user_agent_t ua);

/* sip_via.c */
void sip_via_stack_init(void);
void sip_via_push(sip_via_t via);
sip_via_t sip_via_pop(void);
void sip_via_init(sip_via_t via);
int sip_via_parse(char *line, sip_via_t via, int dns);
void sip_via_list_insert_tail(sip_via_t via, sip_via_t *list);
void sip_free_via_hdrs(sip_via_t *via_hdrs);

#endif
