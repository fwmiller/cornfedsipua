#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "log.h"
#include "sip.h"
#include "net.h"

#define IFC_BUFSIZE	1024

static struct ifconf ifc;

int
net_get_ifaddr(sip_user_agent_t ua)
{
	int i, ifrs, result;

	if (ua == NULL)
		return (-1);

	/*
	 * XXX This needs to be made fancier.  Per Stevens, Volume 1,
	 * this buffer size needs to be re-malloc'd until the returned
	 * length stops increasing to make sure we get the entire list
	 * of interfaces.
	 */
	ifc.ifc_len = IFC_BUFSIZE;
	if (ifc.ifc_buf == NULL)
		ifc.ifc_buf = malloc(IFC_BUFSIZE);
	if (ifc.ifc_buf == NULL) {
		log_msg(LOG_ERROR, "No memory for ifconf buffer");
		return (-1);
	}
	memset(ifc.ifc_buf, 0, IFC_BUFSIZE);
	result = ioctl(ua->sipfd, SIOCGIFCONF, &ifc);
	if (result < 0) {
		log_msg(LOG_ERROR, "SIOCGIFCONF failed (%s)",
			strerror(errno));
		return (-1);
	}
	ifrs = ifc.ifc_len / sizeof(struct ifreq);
	for (i = 0; i < ifrs; i++) {
		struct ifreq *ifr = &(ifc.ifc_req[i]);
		struct sockaddr_in *addr;
		struct in_addr in;
		char *s;

		addr = (struct sockaddr_in *) &(ifr->ifr_addr);
		if (addr->sin_family != AF_INET)
			continue;

		if (strcmp(ifr->ifr_name, ua->if_name) == 0) {
			in.s_addr = addr->sin_addr.s_addr;
			s = inet_ntoa(in);
			memset(ua->local_endpoint.host, 0, BUFSIZE);
			strcpy(ua->local_endpoint.host, s);
			log_msg(LOG_INFO, "%s IP address: [%s]",
				ua->if_name, s);

			/* Get subnet mask */
			result = ioctl(ua->sipfd, SIOCGIFNETMASK, ifr);
			if (result == 0) {
				addr = (struct sockaddr_in *)
				    &(ifr->ifr_addr);
				in.s_addr = addr->sin_addr.s_addr;
				s = inet_ntoa(in);
				memset(ua->local_netmask, 0, BUFSIZE);
				strcpy(ua->local_netmask, s);
				log_msg(LOG_INFO, "%s subnet mask: [%s]",
					ua->if_name, s);
				return 0;
			}
			log_msg(LOG_ERROR,
				"SIOCGIFNETMASK failed (%s)",
				strerror(errno));
			return (-1);
		}
	}
	return (-1);
}

int
net_init(int port)
{
	struct sockaddr_in addr;
	int fd, result;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0) {
		log_msg(LOG_ERROR, "Create socket failed (%s)",
			strerror(errno));
		return (-1);
	}
	
	result = fcntl(fd, F_SETFL, O_NONBLOCK);
	if (result < 0) {
		log_msg(LOG_ERROR, "Set socket to non-blocking failed (%s)",
			strerror(errno));
		return (-1);
	}
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	result = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
	if (result < 0) {
		log_msg(LOG_ERROR, "Bind socket failed (%s)",
			strerror(errno));
		close(fd);
		return (-1);
	}
	return fd;
}

int
net_send(int fd, char *host, int port, char *buf, int len)
{
	struct in_addr in;
	struct sockaddr_in addr;
	int sent;

	if (fd < 0 || host == NULL || buf == NULL)
		return (-1);

	/*
	 * This routine assumes an IP address is specified in the host
	 * parameter
	 */
	inet_aton(host, &in);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = in.s_addr;
	addr.sin_port = htons((unsigned short) port);
	sent = sendto(fd, buf, len, 0, (struct sockaddr *) &addr,
		      sizeof(addr));
	if (sent < 0)
		log_msg(LOG_ERROR, "Socket send failed (%s)", strerror(errno));

	return sent;
}

int
net_recv(int fd, char *buf, int len)
{
	struct sockaddr addr;
	int addrlen;

	if (fd < 0 || buf == NULL)
		return (-1);

	addrlen = sizeof(addr);
	return recvfrom(fd, buf, len, 0, &addr, (socklen_t *) &addrlen);
}
