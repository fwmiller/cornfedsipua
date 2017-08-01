#include <stdlib.h>
#include "codec_alaw.h"
#if _CODEC_G729
#include "codec_g729.h"
#endif
#include "codec_ulaw.h"
#include "config.h"
#include "gui.h"
#include "history.h"
#include "sip.h"

struct config config;
struct sip_user_agent ua;
int reg_expires = (-1);
struct timeval reg_end;

int
main(int argc, char **argv)
{
	pthread_t thread;
	int result;

	printf("Cornfed SIP User Agent\n");
	printf("Version %d.%d.%d\n",
		CORNFEDSIPUA_VERSION, MAJOR_RELEASE, MINOR_RELEASE);
	printf("Copyright (C) 2004-2008 Cornfed Systems\n");
	printf("Written by Frank W. Miller\n\n");

	config_init(&config);
	config_check_noconfig(argc, argv, &config);
	if (!(config.noconfig))
		config_check_config_file(argc, argv, &config);
	config_check_cli_args(argc, argv, &config);
	config_dump(&config);

	ulaw_init();
	alaw_init();
#if _CODEC_G729
	g729_init();
#endif
	sip_dialog_stack_init();
	sip_via_stack_init();
	sip_user_agent_init(&ua);

	config_apply(&config, &ua);

	ua.uac_canceled = uac_canceled;
	ua.uac_connect = uac_connect;
	ua.uac_completed = uac_completed;
	ua.uac_register_prompt_for_user = uac_register_prompt_for_user;
	ua.uac_register_failed = uac_register_failed;
	ua.uac_timeout = uac_timeout;
	ua.uas_request_uri_lookup = uas_request_uri_lookup;
	ua.uas_cancel = uas_cancel;
	ua.uas_ringback = uas_ringback;
	ua.uas_completed = uas_completed;
	ua.uas_connect = uas_connect;
	ua.uas_hangup = uas_hangup;
	ua.reg_get_interval = reg_get_interval;
	ua.reg_get_expires = reg_get_expires;
	ua.reg_set_expires = reg_set_expires;
	ua.reg_get_auth_user = reg_get_auth_user;
	ua.reg_thread_func = reg_thread_func;
	ua.ua_get_rtt = ua_get_rtt;
	ua.ua_set_rtt = ua_set_rtt;
	ua.ua_history_update = ua_history_update;
	ua.ua_history_clear = ua_history_clear;

	reg_set_expires(NULL, (-1));

	result = sip_init(&ua);
	if (result < 0) {
		if (strcmp(ua.if_name, IF_NAME_LOCAL) == 0)
			exit(-1);

		memset(ua.if_name, 0, BUFSIZE);
		strcpy(ua.if_name, IF_NAME_LOCAL);

		result = sip_init(&ua);
		if (result < 0)
			exit(-1);
	}
	stun_init(&ua, config.stun_server);
	rtp_init(&ua);

	result = pthread_create(&thread, NULL, sip_thread, &ua);
	if (result != 0) {
		printf("Create SIP thread failed (%s)\n", strerror(result));
		exit(-1);
	}
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);

	history_init();
	history_read_file();

	/* Attempt initial registration */
	if (strlen(ua.reg_uri.user) > 0 &&
	    strlen(ua.reg_uri.passwd) > 0 &&
	    sip_uri_isset(&(ua.reg_uri))) {
		reg_set_expires(NULL, REGISTER_INTERVAL);
		sip_uac_register(&ua);
		sip_timer_start(&(ua.registration->timers),
				SIPT_REGISTER_RETR, 1000 * SIP_TIMER_1);
	}
	gdk_threads_enter();
	gui_init(WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_main();
	gdk_threads_leave();

	history_write_file();

	if (!(config.noconfig))
		config_file_write(&ua);

	exit(0);
}
