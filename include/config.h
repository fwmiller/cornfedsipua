#ifndef __CONFIG_H
#define __CONFIG_H

#include "sip.h"

struct config {
	char cornfeddir[BUFSIZE];
	char configfile[BUFSIZE];
	int debug;
	int log_level;
	int nat;
	char if_name[BUFSIZE];
	int sip_port;
	int rtp_port;
	char stun_server[BUFSIZE];
	struct sip_uri remote_uri;
	struct sip_uri reg_uri;
	struct ipendpoint outbound_proxy;
	char soundcard_device[BUFSIZE];
	char ringtone_device[BUFSIZE];
	char ringtone_file[BUFSIZE];
	int noconfig;
	int nodns;
};

typedef struct config *config_t;

void config_check_noconfig(int argc, char **argv, config_t config);
void config_check_config_file(int argc, char **argv, config_t config);
void config_check_cli_args(int argc, char **argv, config_t config);
void config_dump(config_t config);
void config_init(config_t config);
void config_apply(config_t config, sip_user_agent_t ua);
int config_file_read(char *configfile, config_t config);
int config_file_write(sip_user_agent_t ua);


#endif
