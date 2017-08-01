#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cornfedsipua.h"
#include "config.h"
#include "dns.h"
#include "http.h"
#include "lex.h"
#include "log.h"

void
config_check_noconfig(int argc, char **argv, config_t config)
{
	int i;

	if ((argc > 0 && argv == NULL) || config == NULL)
		return;

	/* Look for a -noconfig file argument */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-noconfig") == 0) {
			config->noconfig = 1;
			break;
		}
	}
}

void
config_check_config_file(int argc, char **argv, config_t config)
{
	int i, result;

	if ((argc > 0 && argv == NULL) || config == NULL)
		return;

	/*
	 * If a -config argument exists, then read the specified
	 * config file.  Otherwise, try to read the default config
	 * file
	 */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-config") == 0) {
			result = config_file_read(argv[++i], config);
			if (result < 0) {
				printf("Read specified config file failed\n");
				exit(-1);
			}
			break;
		}
	}
	if (i == argc || argc == 0) {
		result = config_file_read(NULL, config);
		if (result < 0) {
			printf("Read config file failed\n");
			exit(-1);
		}
	}
}

void
config_check_cli_args(int argc, char **argv, config_t config)
{
	int i;

	if ((argc > 0 && argv == NULL) || config == NULL)
		return;

	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "-intf") == 0) {
			if (++i == argc) {
				printf("Missing network interface name\n");
				exit(-1);
			}
			memset(config->if_name, 0, BUFSIZE);
			strcpy(config->if_name, argv[i]);

		} else if (strcmp(argv[i], "-sport") == 0) {
			if (++i == argc) {
				printf("Missing sip port\n");
				exit(-1);
			}
			config->sip_port = atoi(argv[i]);

		} else if (strcmp(argv[i], "-rport") == 0) {
			if (++i == argc) {
				printf("Missing rtp port\n");
				exit(-1);
			}
			config->rtp_port = atoi(argv[i]);

		} else if (strcmp(argv[i], "-stun") == 0) {
			if (++i == argc) {
				printf("Missing STUN server host\n");
				exit(-1);
			}
			memset(config->stun_server, 0, BUFSIZE);
			strcpy(config->stun_server, argv[i]);

		} else if (strcmp(argv[i], "-soundcard") == 0) {
			if (++i == argc) {
				printf("Missing soundcard device name\n");
				exit(-1);
			}
			memset(config->soundcard_device, 0, BUFSIZE);
			strcpy(config->soundcard_device, argv[i]);

		} else if (strcmp(argv[i], "-ringdevice") == 0) {
			if (++i == argc) {
				printf("Missing ringtone device name\n");
				exit(-1);
			}
			memset(config->ringtone_device, 0, BUFSIZE);
			strcpy(config->ringtone_device, argv[i]);

		} else if (strcmp(argv[i], "-ringfile") == 0) {
			if (++i == argc) {
				printf("Missing ringtone .wav file name\n");
				exit(-1);
			}
			memset(config->ringtone_file, 0, BUFSIZE);
			strcpy(config->ringtone_file, argv[i]);

		} else if (strcmp(argv[i], "-dns") == 0)
			config->nodns = 0;

		else if (strcmp(argv[i], "-nodns") == 0)
			config->nodns = 1;

		else if (strcmp(argv[i], "-config") == 0)
			i++;

		else if (strcmp(argv[i], "-noconfig") == 0)
			continue;

		else {
			printf("Unrecognized command line argument\n");
			exit(-1);
		}
}

void
config_dump(config_t config)
{
	char s[128];

	if (config == NULL)
		return;

	printf("\n");
	printf("cornfeddir: [%s]\n", config->cornfeddir);
	printf("configfile: [%s]\n", config->configfile);
	printf("debug: %s\n", (config->debug ? "yes" : "no"));

	memset(s, 0, 128);
	sprintf(s, "log_level: ");
	switch (config->log_level) {
	case LOG_ERROR:
		strcat(s, "LOG_ERROR\n");
		break;
	case LOG_WARNING:
		strcat(s, "LOG_WARNING\n");
		break;
	case LOG_CONNECTION:
		strcat(s, "LOG_CONNECTION\n");
		break;
	case LOG_EVENT:
		strcat(s, "LOG_EVENT\n");
		break;
	case LOG_INFO:
		strcat(s, "LOG_INFO\n");
		break;
	default:
		strcat(s, "ERROR\n");
	}
	printf(s);

	printf("nat: %s\n", (config->nat ? "yes" : "no"));
	printf("if_name: [%s]\n", config->if_name);
	printf("sip_port: %d\n", config->sip_port);
	printf("rtp_port: %d\n", config->rtp_port);
	printf("stun_server_host: [%s]\n", config->stun_server);

	memset(s, 0, 128);
	sprintf(s, "remote_uri: [");
	sip_uri_gen(&(config->remote_uri), s + strlen(s));
	strcat(s, "]\n");
	printf(s);

	memset(s, 0, 128);
	sprintf(s, "reg_uri: [");
	sip_uri_gen(&(config->reg_uri), s + strlen(s));
	strcat(s, "]\n");
	printf(s);

	printf("outbound_proxy.host: [%s]\n", config->outbound_proxy.host);
	printf("outbound_proxy.port: %d\n", config->outbound_proxy.port);
	printf("soundcard_device: [%s]\n", config->soundcard_device);
	printf("ringtone_device: [%s]\n", config->ringtone_device);
	printf("ringtone_file: [%s]\n", config->ringtone_file);
	printf("noconfig: %s\n", (config->noconfig ? "yes" : "no"));
	printf("dns: %s\n", (config->nodns ? "no" : "yes"));
	printf("\n");
}

void
config_init(config_t config)
{
	if (config == NULL)
		return;

	memset(config->cornfeddir, 0, BUFSIZE);
	memset(config->configfile, 0, BUFSIZE);
	config->debug = 1;
	config->log_level = LOG_INFO;
	config->nat = 1;
	memset(config->if_name, 0, BUFSIZE);
	config->sip_port = SIP_DEFAULT_PORT;
	config->rtp_port = RTP_DEFAULT_PORT;
	memset(config->stun_server, 0, BUFSIZE);
	sip_uri_init(&(config->remote_uri));
	sip_uri_init(&(config->reg_uri));
	ipendpoint_init(&(config->outbound_proxy));
	memset(config->soundcard_device, 0, BUFSIZE);
	strcpy(config->soundcard_device, SOUNDCARD_DEVICE);
	memset(config->ringtone_device, 0, BUFSIZE);
	strcpy(config->ringtone_device, RINGTONE_DEVICE);
	memset(config->ringtone_file, 0, BUFSIZE);
	strcpy(config->ringtone_file, RINGTONE_FILE);
	config->noconfig = 0;
	config->nodns = 0;
}

void
config_apply(config_t config, sip_user_agent_t ua)
{
	char *ipaddr;
	int ipaddrlen, result;

	if (config == NULL || ua == NULL)
		return;

	strcpy(ua->cornfeddir, config->cornfeddir);
	strcpy(ua->configfile, config->configfile);

	if (config->debug)
		ua->flags |= SUAF_DEBUG;
	else
		ua->flags &= ~SUAF_DEBUG;

	if (config->log_level >= LOG_ERROR &&
	    config->log_level <= LOG_INFO)
		log_set_level(config->log_level);

	if (config->nat)
		ua->flags |= SUAF_LOCAL_NAT;
	else
		ua->flags &= ~SUAF_LOCAL_NAT;

	if (strlen(config->if_name) == 0)
		strcpy(ua->if_name, IF_NAME);
	else
		strcpy(ua->if_name, config->if_name);

	ua->local_endpoint.port = config->sip_port;
	ua->rtp_port = config->rtp_port;

	result = find_ip_address(config->stun_server, BUFSIZE,
				 &ipaddr, &ipaddrlen);
	if (result == 0)
		strncpy(ua->stun_server.host, ipaddr, ipaddrlen);

	else if (dns_avail(ua)) {
		char ipaddr[128];

		result = dns_gethostbyname(config->stun_server, ipaddr, 128);
		if (result >= 0) {
			strcpy(ua->stun_server.domain, config->stun_server);
			strcpy(ua->stun_server.host, ipaddr);
		}
	}
	memcpy(&(ua->remote_uri), &(config->remote_uri),
	       sizeof(struct sip_uri));
	memcpy(&(ua->reg_uri), &(config->reg_uri), sizeof(struct sip_uri));

	strcpy(ua->outbound_proxy.host, config->outbound_proxy.host);
	ua->outbound_proxy.port = config->outbound_proxy.port;


	if (strlen(config->soundcard_device) > 0) {
		result = soundcard_init(ua, config->soundcard_device);
		if (result < 0) {
			log_msg(LOG_INFO,
				"Could not initialize soundcard device [%s]",
				config->soundcard_device);
			log_msg(LOG_INFO,
				"Trying default soundcard [%s]",
				SOUNDCARD_DEVICE);

			result = soundcard_init(ua, SOUNDCARD_DEVICE);
		}
	} else
		result = soundcard_init(ua, SOUNDCARD_DEVICE);
	if (result < 0)
		log_msg(LOG_INFO,
			"Could not initialize soundcard device");


	if (strlen(config->ringtone_device) > 0) {
		result = ringtone_device_init(ua, config->ringtone_device);
		if (result < 0) {
			log_msg(LOG_INFO,
				"Could not initialize ringtone device");
			log_msg(LOG_INFO,
				"Trying default ringtone device [%s]",
				RINGTONE_DEVICE);
			result = ringtone_device_init(ua, RINGTONE_DEVICE);
		}
	} else
		result = ringtone_device_init(ua, RINGTONE_DEVICE);
	if (result < 0)
		log_msg(LOG_INFO, "Could not initialize ringtone device");

	if (strlen(config->ringtone_file) > 0) {
		result = ringtone_file_init(
			&(ua->ringtone),
			(const char *) config->ringtone_file);
		if (result < 0) {
			log_msg(LOG_INFO,
				"Could not initialize ringtone file");
			log_msg(LOG_INFO,
				"Trying default ringtone file [%s]",
				RINGTONE_FILE);
			result = ringtone_file_init(
				&(ua->ringtone),
				(const char *) RINGTONE_FILE);
		}
	} else
		result = ringtone_file_init(
			&(ua->ringtone),
			(const char *) RINGTONE_FILE);
	if (result < 0)
		log_msg(LOG_INFO, "Could not initialize ringtone file");

	if (config->nodns)
		ua->flags |= SUAF_NO_DNS;
}

static int
config_file_write_line(int fd, char *line)
{
	int result;

	if (fd < 0 || line == NULL)
		return (-1);

	result = write(fd, line, strlen(line));
	if (result < 0) {
		log_msg(LOG_ERROR, "Write config file failed");
		return (-1);
	}
	return 0;
}

static int
config_file_create(char *configfile)
{
	char s[128];
	int fd;

	if (configfile == NULL)
		return (-1);

	/* Create default config file */                                    
	fd = open(configfile, O_WRONLY | O_CREAT, 0600);
	if (fd < 0) {
		log_msg(LOG_ERROR, "Could not create config file");
		return (-1);
	}
	/* Write default config file lines */
	config_file_write_line(fd, "#\n");
	config_file_write_line(fd, 
		"# Do not edit -- file generated automagically\n");
	config_file_write_line(fd, "#\n");
	config_file_write_line(fd, "debug=on\n");
	config_file_write_line(fd, "log=info\n");
	config_file_write_line(fd, "nat=on\n");

	memset(s, 0, 128);
	sprintf(s, "if_name=%s\n", IF_NAME);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "sip_port=%d\n", SIP_DEFAULT_PORT);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "rtp_port=%d\n", RTP_DEFAULT_PORT);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "stun_server=%s\n", STUN_SERVER);
	config_file_write_line(fd, s);

	config_file_write_line(fd, "remote_uri=sip:613@fwd.pulver.com\n");
	config_file_write_line(fd,
		"register_uri=sip:276140@fwd.pulver.com\n");

	config_file_write_line(fd, "outbound_proxy_host=\n");
	config_file_write_line(fd, "outbound_proxy_port=\n");

	memset(s, 0, 128);
	sprintf(s, "soundcard=%s\n", SOUNDCARD_DEVICE);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "ringtone_device=%s\n", RINGTONE_DEVICE);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "ringtone_file=%s\n", RINGTONE_FILE);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "dns=yes\n");
	config_file_write_line(fd, s);

	close(fd);
	return 0;
}

static int
config_file_read_line(int fd, char *line)
{
	int len, nread;
	char ch;

	if (fd < 0 || line == NULL)
		return (-1);

	for (len = 0;;) {
		nread = read(fd, &ch, 1);
		if (nread < 0)
			return (-1);
		if (nread == 0)
			break;

		if (ch == '\n')
			break;

		line[len++] = ch;
	}
	return len;
}

static void
config_file_parse_line(char *line, config_t config)
{
	char lval[128], rval[128];
	int pos = 0;

	if (line == NULL || config == NULL)
		return;

	/* Skip comment lines and zero-length lines */
	if (line[0] == '#' || strlen(line) == 0)
		return;

	/* Get lval */
	memset(lval, 0, 128);
	nextarg(line, &pos, "=", lval);
	if (line[pos++] != '=')
		return;

	/* Get rval */
	memset(rval, 0, 128);
	nextarg(line, &pos, "", rval);

	/* Skip lines with a zero-length rval */
	if (strlen(rval) == 0)
		return;

	if (strcasecmp(lval, "debug") == 0) {
		if (strcasecmp(rval, "on") == 0)
			config->debug = 1;
		else if (strcasecmp(rval, "off") == 0)
			config->debug = 0;
		else
			log_msg(LOG_ERROR, "Bad rval in config.");

	} else if (strcasecmp(lval, "log") == 0) {
		if (strcasecmp(rval, "error") == 0)
			config->log_level = LOG_ERROR;
		else if (strcasecmp(rval, "warning") == 0)
			config->log_level = LOG_WARNING;
		else if (strcasecmp(rval, "connection") == 0)
			config->log_level = LOG_CONNECTION;
		else if (strcasecmp(rval, "event") == 0)
			config->log_level = LOG_EVENT;
		else if (strcasecmp(rval, "info") == 0)
			config->log_level = LOG_INFO;

	} else if (strcasecmp(lval, "nat") == 0) {
		if (strcasecmp(rval, "on") == 0)
			config->nat = 1;
		else if (strcasecmp(rval, "off") == 0)
			config->nat = 0;

	} else if (strcasecmp(lval, "if_name") == 0) {
		if (strlen(rval) > 0)
			strcpy(config->if_name, rval);

	} else if (strcasecmp(lval, "sip_port") == 0) {
		int port = atoi(rval);

		if (port >= 0 && port < 0x10000)
			config->sip_port = port;

	} else if (strcasecmp(lval, "rtp_port") == 0) {
		int port = atoi(rval);

		if (port >= 0 && port < 0x10000)
			config->rtp_port = port;

	} else if (strcasecmp(lval, "stun_server") == 0) {
		if (strlen(rval) > 0)
			strcpy(config->stun_server, rval);

	} else if (strcasecmp(lval, "remote_uri") == 0) {
		log_msg(LOG_INFO, "Remote URI: [%s]", rval);
		sip_uri_parse(rval, &(config->remote_uri),
			      (config->nodns ? 0 : 1));

	} else if (strcasecmp(lval, "register_uri") == 0) {
		log_msg(LOG_INFO, "Register URI: [%s]", rval);
		sip_uri_parse(rval, &(config->reg_uri),
			      (config->nodns ? 0 : 1));

	} else if (strcasecmp(lval, "outbound_proxy_host") == 0) {
		char *ipaddr;
		int ipaddrlen, result;

		result = find_ip_address(rval, 128, &ipaddr, &ipaddrlen);
		if (result == 0)
			strncpy(config->outbound_proxy.host, ipaddr,
				ipaddrlen);

	} else if (strcasecmp(lval, "outbound_proxy_port") == 0) {
		int port = atoi(rval);

		if (port >= 0 && port < 0x10000)
			config->outbound_proxy.port = port;

	} else if (strcasecmp(lval, "soundcard") == 0) {
		if (strlen(rval) > 0)
			strcpy(config->soundcard_device, rval);

	} else if (strcasecmp(lval, "ringtone_device") == 0) {
		if (strlen(rval) > 0)
			strcpy(config->ringtone_device, rval);

	} else if (strcasecmp(lval, "ringtone_file") == 0) {
		if (strlen(rval) > 0)
			strcpy(config->ringtone_file, rval);

	} else if (strcasecmp(lval, "dns") == 0) {
		if (strcmp(rval, "no") == 0)
			config->nodns = 1;
	}
}

int
config_file_read(char *configfile, config_t config)
{
	struct stat buf;
	char line[128];
	int fd, result;

	if (config == NULL)
		return (-1);

	if (configfile == NULL) {
		char *homedir;

		/* Get user home directory */
		homedir = getenv("HOME");
		if (homedir == NULL) {
			log_msg(LOG_ERROR, "Could not get home directory");
			return (-1);
		}
		/* Add .cornfed to home directory */
		strcpy(config->cornfeddir, homedir);
		strcat(config->cornfeddir, CORNFED_DIR);
		log_msg(LOG_INFO, "Cornfed directory: [%s]",
			config->cornfeddir);

		/* Add config file name to cornfed directory */
		strcpy(config->configfile, config->cornfeddir);
		strcat(config->configfile, CONFIG_FILE);
	} else
		strcpy(config->configfile, configfile);

	log_msg(LOG_INFO, "Config file: [%s]", config->configfile);

	/* Check whether config file exists */
	memset(&buf, 0, sizeof(struct stat));
	result = stat(config->configfile, &buf);
	if (result < 0) {
		/* If not, check whether cornfed directory exists */
		if (configfile == NULL) {
			memset(&buf, 0, sizeof(struct stat));
			result = stat(config->cornfeddir, &buf);
			if (result < 0) {
				/* Create cornfed directory */
				result = mkdir(config->cornfeddir, 0777);
				if (result < 0) {
					log_msg(LOG_ERROR,
					"Create cornfed directory failed");
					return (-1);
				}
			}
		}
		/* Create config file */
		result = config_file_create(config->configfile);
		if (result < 0)
			return (-1);
	}
	fd = open(config->configfile, O_RDONLY);
	if (fd < 0) {
		char backupconfigfile[BUFSIZE];

		log_msg(LOG_ERROR, "Open config file failed (%s)",
			strerror(errno));

		memset(backupconfigfile, 0, BUFSIZE);
		strcpy(backupconfigfile, config->configfile);
		strcat(backupconfigfile, "~");

		fd = open(backupconfigfile, O_RDONLY);
		if (fd < 0) {
			log_msg(LOG_ERROR,
				"Open backup config file failed (%s)",
				strerror(errno));
			return (-1);
		}
	}
	/* Read the config file line by line and parse each entry */
	for (;;) {
		memset(line, 0, 128);
		result = config_file_read_line(fd, line);
		if (result < 0) {
			log_msg(LOG_ERROR, "Read config file line failed");
			close(fd);
			return (-1);
		}
		if (result == 0)
			break;

		config_file_parse_line(line, config);
	}
	close(fd);
	return 0;
}

int
config_file_write(sip_user_agent_t ua)
{
	char backupconfigfile[BUFSIZE], s[128];
	int fd, result;

	if (ua == NULL)
		return (-1);

	memset(backupconfigfile, 0, BUFSIZE);
	strcpy(backupconfigfile, ua->configfile);
	strcat(backupconfigfile, "~");

	/* Remove existing config file */
	result = rename(ua->configfile, backupconfigfile);
	if (result < 0)
		log_msg(LOG_ERROR, "Rename config file failed");

	/* Create a new config file */
	fd = open(ua->configfile, O_WRONLY | O_CREAT, 0600);
	if (fd < 0) {
		log_msg(LOG_ERROR, "Could not create config file [%s]",
			ua->configfile);
		return (-1);
	}
	/* Write config file lines */
	config_file_write_line(fd, "#\n");
	config_file_write_line(fd, 
		"# Do not edit -- file generated automagically\n");
	config_file_write_line(fd, "#\n");

	if (ua->flags & SUAF_DEBUG)
		config_file_write_line(fd, "debug=on\n");
	else
		config_file_write_line(fd, "debug=off\n");

	switch (log_get_level()) {
	case LOG_ERROR:
		config_file_write_line(fd, "log=error\n");
		break;
	case LOG_WARNING:
		config_file_write_line(fd, "log=warning\n");
		break;
	case LOG_CONNECTION:
		config_file_write_line(fd, "log=connection\n");
		break;
	case LOG_EVENT:
		config_file_write_line(fd, "log=event\n");
		break;
	default:
		config_file_write_line(fd, "log=info\n");
	}
	if (ua->flags & SUAF_LOCAL_NAT)
		config_file_write_line(fd, "nat=on\n");
	else
		config_file_write_line(fd, "nat=off\n");

	memset(s, 0, 128);
	if (strcmp(ua->if_name, IF_NAME_LOCAL) == 0)
		sprintf(s, "if_name=%s\n", IF_NAME);
	else
		sprintf(s, "if_name=%s\n", ua->if_name);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "sip_port=%d\n", ua->local_endpoint.port);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "rtp_port=%d\n", ua->rtp_port);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	if (strlen(ua->stun_server.domain) > 0)
		sprintf(s, "stun_server=%s\n", ua->stun_server.domain);
	else
		sprintf(s, "stun_server=%s\n", ua->stun_server.host);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "remote_uri=");
	sip_uri_gen(&(ua->remote_uri), s + strlen(s));
	sprintf(s + strlen(s), "\n");
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "register_uri=");
	sip_uri_gen(&(ua->reg_uri), s + strlen(s));
	sprintf(s + strlen(s), "\n");
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "outbound_proxy_host=%s\n", ua->outbound_proxy.host);
	config_file_write_line(fd, s);

	if (ua->outbound_proxy.port < 0 || ua->outbound_proxy.port > 65535)
		config_file_write_line(fd, "outbound_proxy_port=\n");
	else {
		memset(s, 0, 128);
		sprintf(s, "outbound_proxy_port=%d\n",
			ua->outbound_proxy.port);
		config_file_write_line(fd, s);
	}
	memset(s, 0, 128);
	sprintf(s, "soundcard=%s\n", ua->soundcard.device);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "ringtone_device=%s\n", ua->ringtone.device);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "ringtone_file=%s\n", ua->ringtone.file);
	config_file_write_line(fd, s);

	memset(s, 0, 128);
	sprintf(s, "dns=%s\n", (dns_avail(ua) ? "yes" : "no"));
	config_file_write_line(fd, s);

	close(fd);
	return 0;
}
