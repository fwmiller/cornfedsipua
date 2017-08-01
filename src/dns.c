#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "lex.h"
#include "log.h"
#include "sip.h"

#define RESOLV_CONF		"/etc/resolv.conf"
#define MAX_NAMESERVERS		8

#define HOSTS_FILE		"/etc/hosts"

struct resolver {
	int ns_count;
	char ns[MAX_NAMESERVERS][128];
};

static struct resolver resolv;

static void dns_read_resolv_conf(void);

void
dns_init()
{
	memset(&resolv, 0, sizeof(struct resolver));
	dns_read_resolv_conf();
}

int dns_avail(sip_user_agent_t ua)
{
	return (ua->flags & SUAF_NO_DNS ? 0 : 1);
}

static int
dns_file_read_line(FILE *fd, char *line)
{
	int len, nread;
	char ch;

	if (fd == NULL || line == NULL)
		return (-1);

	for (len = 0;;) {
		nread = fread(&ch, 1, 1, fd);
		if (nread < 0)
			return (-1);
		if (nread == 0 || ch == '\n')
			break;
		line[len++] = ch;
	}
	return len;
}

static int
dns_hosts_parse_line(char *line, char *host, char *ipaddr, int len)
{
	char s[128];
	int pos = 0;

	if (line == NULL || host == NULL || ipaddr == NULL)
		return (-1);

	memset(ipaddr, 0, len);
	nextarg(line, &pos, " \t", ipaddr);

	for (;;) {
		memset(s, 0, 128);
		nextarg(line, &pos, " \t", s);
		if (strlen(s) == 0)
			break;

		if (strcmp(s, host) == 0)
			return 0;
	}
	return (-1);
}

static int
dns_read_hosts_file(char *host, char *ipaddr, int len)
{
	FILE *fd;
	char line[128];

	if (host == NULL || ipaddr == NULL)
		return (-1);

	fd = fopen(HOSTS_FILE, "r");
	if (fd == NULL)
		return (-1);

	for (;;) {
		int result;

		memset(line, 0, 128);
		result = dns_file_read_line(fd, line);
		if (result < 0 || (result == 0 && feof(fd)))
			break;

		if (line[0] == '#')
			continue;

		result = dns_hosts_parse_line(line, host, ipaddr, len);
		if (result == 0) {
			fclose(fd);
			return 0;
		}
	}
	fclose(fd);
	return (-1);
}

int
dns_gethostbyname(char *host, char *ipaddr, int len)
{
	struct hostent *entry;
	int i;

	if (host == NULL || ipaddr == NULL)
		return (-1);

	entry = gethostbyname(host);
	if (entry == NULL)
		return dns_read_hosts_file(host, ipaddr, len);

	for (i = 0; entry->h_addr_list[i] != NULL; i++) {
		struct in_addr in;

		in = *((struct in_addr *) entry->h_addr_list[i]);
		if (i == 0)
			strncpy(ipaddr, inet_ntoa(in), len - 1);
	}
	if (i == 0)
		return (-1);

	return 0;
}

static int
dns_file_parse_line(char *line, char *nameserver)
{
	if (line == NULL || nameserver == NULL)
		return (-1);

	if (strncmp(line, "nameserver", 10) == 0) {
		int pos = 10;

		nextarg(line, &pos, " ", nameserver);
		return 0;
	}
	return (-1);
}

static void
dns_read_resolv_conf()
{
	FILE *fd;
	char line[128];
	int i;

	printf("\n");

	fd = fopen(RESOLV_CONF, "r");
	if (fd == NULL) {
		printf("Open %s failed (%s)\n",
		       RESOLV_CONF, strerror(errno));
		return;
	}
	for (;;) {
		int result;

		if (resolv.ns_count == MAX_NAMESERVERS)
			break;

		memset(line, 0, 128);
		result = dns_file_read_line(fd, line);
		if (result < 0) {
			printf("Read %s failed (%s)\n",
				RESOLV_CONF, strerror(errno));
			fclose(fd);
			return;
		}
		if (result == 0 && feof(fd))
			break;

		result = dns_file_parse_line(line,
					     resolv.ns[resolv.ns_count]);
		if (result < 0)
			memset(resolv.ns[resolv.ns_count], 0, 128);
		else
			resolv.ns_count++;
	}
	for (i = 0; i < resolv.ns_count; i++)
		printf("nameserver: [%s]\n", resolv.ns[i]);

	fclose(fd);
}
