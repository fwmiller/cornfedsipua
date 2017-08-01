#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dns.h"
#include "http.h"
#include "lex.h"
#include "log.h"
#include "net.h"
#include "sip.h"

static unsigned int seed;

static void
sip_thread_func(sip_user_agent_t ua)
{
	fd_set rfds;
	struct timeval timeout;

	char buf[BUFSIZE];
	int fd, len, result;

	if (ua == NULL)
		return;

	fd = ua->sipfd;
	if (fd < 0)
		return;

	/* Check for an incoming SIP message */
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	timerclear(&timeout);
	result = select(fd + 1, &rfds, NULL, NULL, &timeout);
	if (result <= 0)
		return;

	/* SIP message waiting */
	len = net_recv(fd, buf, BUFSIZE);
	if (len <= 0)
		return;

	sip_recv(ua, buf, len);
}

void *
sip_thread(void *arg)
{
	sip_user_agent_t ua = (sip_user_agent_t) arg;
	struct timeval last, now, dur;
	struct timezone tz;
	int period = 5000;

	if (ua == NULL)
		return NULL;

	gettimeofday(&last, &tz);
	for (;;) {
		stun_thread_func(ua);
		sip_thread_func(ua);
		rtp_thread_func(ua);
		soundcard_thread_func(ua);
		sip_timer_thread_func(ua);
		sip_ringing_thread_func(ua);
		if (ua->reg_thread_func != NULL)
			(ua->reg_thread_func)();

		gettimeofday(&now, &tz);
		timersub(&now, &last, &dur);
		if (dur.tv_sec < 1 && dur.tv_usec < period)
			usleep(period - dur.tv_usec);

		dur.tv_sec = 0;
		dur.tv_usec = period;
		timeradd(&last, &dur, &last);
	}
}

int
sip_init_port(sip_user_agent_t ua, int port)
{
	if (ua == NULL)
		return (-1);

	if (ua->sipfd >= 0) {
		int result = close(ua->sipfd);

		if (result < 0) {
			log_msg(LOG_ERROR, "Close SIP port %d failed (%s)",
				ua->local_endpoint.port,
				strerror(errno));
			return (-1);
		}
	}
	log_msg(LOG_INFO, "SIP local port %d", port);

	ua->sipfd = net_init(port);
	if (ua->sipfd < 0) {
		log_msg(LOG_ERROR, "Initialize SIP port failed");
		return (-1);
	}
	return 0;
}

int
sip_init(sip_user_agent_t ua)
{
	int result;

	if (ua == NULL)
		return (-1);

	/*
	 * Seed random number generator for various branch and tag
	 * generation routines
	 */
	seed = (unsigned int) time(NULL);
	srand(seed);

	/* Initialize SIP socket */
	result = sip_init_port(ua, ua->local_endpoint.port);
	if (result < 0)
		return (-1);

	/* Get local IP address */
	
	result = net_get_ifaddr(ua);
	if (result < 0)
		return (-1);

	/* Initialize SIP timers */
	sip_timer_init(ua->dialog);

	return 0;
}

int
sip_send(sip_user_agent_t ua, char *host, int port, char *buf, int len)
{
	int sent;

	if (ua == NULL) {
		log_msg(LOG_ERROR, "Bad user agent for SIP message send");
		return (-1);
	}
	if (host == NULL) {
		log_msg(LOG_ERROR, "Bad host address for SIP message send");
		return (-1);
	}
	if (buf == NULL) {
		log_msg(LOG_ERROR, "Bad buffer address for SIP message send");
		return (-1);
	}

	if (strlen(ua->outbound_proxy.host) > 0 &&
	    ua->outbound_proxy.port >= 0 &&
	    ua->outbound_proxy.port < 65536) {
		log_msg(LOG_INFO, "Send to outbound proxy [%s] port %d",
			ua->outbound_proxy.host, ua->outbound_proxy.port);
		host = ua->outbound_proxy.host;
		port = ua->outbound_proxy.port;
	}
	if (strlen(host) == 0) {
		log_msg(LOG_ERROR,
			"Bad host address length for SIP message send");
		return (-1);
	}
	if (port < 0 || port > 65535) {
		log_msg(LOG_ERROR,
			"Bad UDP port value for SIP message send");
		return (-1);
	}
	sent = net_send(ua->sipfd, host, port, buf, len);

	if (sent > 0) {
		char line[BUFSIZE];
		int i, j;

		log_msg(LOG_INFO, "");
		log_msg(LOG_INFO, "==> Sent %d bytes to %s:%d",
			sent, host, port);
		memset(line, 0, BUFSIZE);
	
		for (i = 0, j = 0; i < len; i++) {
			if (buf[i] == '\n') {		
				log_msg(LOG_INFO, line);
				memset(line, 0, BUFSIZE);

				j = 0;
				continue;
			}
			if (isprint(buf[i]))
				line[j++] = buf[i];
		}
		
	}

	return sent;
}

int
sip_get_send_host_port(sip_dialog_t dialog, char *host, int *port, int dns)
{
	if (dialog == NULL || host == NULL || port == NULL)
		return (-1);

	if (dialog->route_set.routes > 0) {
		struct sip_uri uri;
		char s[BUFSIZE];

		memset(s, 0, BUFSIZE);
		memcpy(s, dialog->route_set.buf, dialog->route_set.len[0]);
		sip_uri_init(&uri);
		sip_uri_parse(s, &uri, dns);

		/* Verify that URI host is an IP address */
		if (strlen(uri.endpoint.host) == 0)
			return (-1);

		strcpy(host, uri.endpoint.host);
		*port = uri.endpoint.port;

	} else if (strlen(dialog->remote_target.endpoint.host) > 0) {
		/* Verify that remote target host is an IP address */
		if (strlen(dialog->remote_target.endpoint.host) == 0)
			return (-1);

		strcpy(host, dialog->remote_target.endpoint.host);
		*port = dialog->remote_target.endpoint.port;

	} else {
		/* Verify that remote URI host is an IP address */
		if (strlen(dialog->remote_uri.endpoint.host) == 0)
			return (-1);

		strcpy(host, dialog->remote_uri.endpoint.host);
		*port = dialog->remote_uri.endpoint.port;
	}
	return 0;
}

char *
sip_get_local_host(sip_user_agent_t ua, char *remote_host)
{
	if (ua == NULL || remote_host == NULL)
		return NULL;

	if (!(ua->flags & SUAF_LOCAL_NAT))
		return ua->local_endpoint.host;

	if (strlen(ua->visible_endpoint.host) > 0) {
		struct in_addr lhost, mask, rhost, vhost;

		inet_aton(ua->local_endpoint.host, &lhost);
		inet_aton(ua->local_netmask, &mask);
		inet_aton(ua->visible_endpoint.host, &vhost);

		if ((vhost.s_addr & mask.s_addr) ==
		    (lhost.s_addr & mask.s_addr))
			return ua->local_endpoint.host;

		if (remote_host != NULL && strlen(remote_host) > 0) {
			inet_aton(remote_host, &rhost);
			if ((rhost.s_addr & mask.s_addr) !=
			    (lhost.s_addr & mask.s_addr))
				return ua->visible_endpoint.host;
		}
	}
	return ua->local_endpoint.host;
}

int
sip_get_local_port(sip_user_agent_t ua, char *remote_host)
{
	if (ua == NULL || remote_host == NULL)
		return (-1);

	if (!(ua->flags & SUAF_LOCAL_NAT))
		return ua->local_endpoint.port;

	if (strlen(ua->visible_endpoint.host) > 0) {
		struct in_addr lhost, mask, rhost, vhost;

		inet_aton(ua->local_endpoint.host, &lhost);
		inet_aton(ua->local_netmask, &mask);
		inet_aton(ua->visible_endpoint.host, &vhost);

		if ((vhost.s_addr & mask.s_addr) ==
		    (lhost.s_addr & mask.s_addr))
			return ua->local_endpoint.port;

		if (remote_host != NULL && strlen(remote_host) > 0) {
			inet_aton(remote_host, &rhost);
			if ((rhost.s_addr & mask.s_addr) !=
			    (lhost.s_addr & mask.s_addr))
				return ua->visible_endpoint.port;
		}
	}
	return ua->local_endpoint.port;
}
