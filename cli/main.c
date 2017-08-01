#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "codec_alaw.h"
#if _CODEC_G729
#include "codec_g729.h"
#endif
#include "codec_ulaw.h"
#include "config.h"
#include "cornfedsipua.h"
#include "dns.h"
#include "history.h"
#include "http.h"
#include "stun.h"

struct config config;
struct sip_user_agent ua;

int
main(int argc, char **argv)
{
	pthread_t thread;
	int result;

	about();
	help_init();
	/* dns_init(); */

	config_init(&config);
	config_check_noconfig(argc, argv, &config);
	if (!(config.noconfig))
	{
		config_check_config_file(argc, argv, &config);
	}
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

	reg_set_expires(NULL, -1);

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
	history_init();
	history_read_file();

	/* Main thread implements a command line interface */
	cli_cmd_history_init();
	cli();

	history_write_file();

	if (!(config.noconfig))
		config_file_write(&ua);

	/* XXX Not reached */
	exit(0);
}
