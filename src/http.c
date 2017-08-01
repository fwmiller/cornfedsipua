#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "dns.h"
#include "http.h"
#include "log.h"

#define IAS_IDLE			0
#define IAS_FIRST			1
#define IAS_SECOND			2
#define IAS_THIRD			3
#define IAS_FOURTH			4

#define HTTP_WWW_MYIPADDRESS_COM	"www.myipaddress.com"
#define HTTP_WWW_SHOWMYIP_COM		"http://www.showmyip.com/simple"
#define HTTP_RESP_BUFSIZE		(8 * BUFSIZE)
#define HTTP_REQUEST "GET / HTTP/1.0\r\n\r\n"
#define HTTP_LOG_REQUEST "GET /cgi-bin/cornfedsipua.pl?%s HTTP/1.0\r\n\r\n"

int
find_ip_address(char *s, int len, char **addr, int *addrlen)
{
	int state = IAS_IDLE;
	int cnt = 0;
	int i;

	if (s == NULL || addr == NULL || addrlen == NULL)
		return (-1);

	*addr = NULL;
	*addrlen = 0;
	for (i = 0; i < len; i++) {
		if (isdigit(s[i])) {
			if (state == IAS_IDLE) {
				*addr = s + i;
				*addrlen = 1;
				cnt = 1;
				state = IAS_FIRST;
				continue;
			}
			/* state > IAS_IDLE */
			if (++cnt > 3) {
				*addr = NULL;
				*addrlen = 0;
				cnt = 0;
				state = IAS_IDLE;
				continue;
			}
			(*addrlen)++;
			continue;

		} else if (state == IAS_FOURTH)
			return 0;

		else if (s[i] == '.') {
			(*addrlen)++;
			cnt = 0;
			state++;

		} else if (state != IAS_IDLE)
			state = IAS_IDLE;
	}
	return (-1);
}

int
http_get_myipaddr(sip_user_agent_t ua)
{
	char host[128];
	struct in_addr in;
	struct sockaddr_in addr;
	char resp[HTTP_RESP_BUFSIZE];
	char *s;
	int fd, len, pos, result;

	if (ua == NULL || !dns_avail(ua))
		return (-1);

	result = dns_gethostbyname(HTTP_WWW_SHOWMYIP_COM, host, 128);
	if (result < 0)
		return (-1);

	log_msg(LOG_INFO, "%s: [%s]", HTTP_WWW_SHOWMYIP_COM, host);
	inet_aton(host, &in);

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		log_msg(LOG_ERROR, "HTTP socket failed (%s)",
			strerror(errno));
		return (-1);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = in.s_addr;
	addr.sin_port = htons(80);
	result = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
	if (result < 0) {
		log_msg(LOG_ERROR, "HTTP connect failed (%s)",
			strerror(errno));
		close(fd);
		return (-1);
	}
	len = write(fd, HTTP_REQUEST, strlen(HTTP_REQUEST));
	if (len < 0) {
		log_msg(LOG_ERROR, "HTTP write request failed (%s)",
			strerror(errno));
		close(fd);
		return (-1);
	}
	memset(resp, 0, HTTP_RESP_BUFSIZE);
	for (pos = 0; pos < 8 * BUFSIZE - 1; pos++) {
		len = read(fd, &(resp[pos]), 1);
		if (len < 0) {
			log_msg(LOG_ERROR, "HTTP read failed (%s)",
				strerror(errno));
			close(fd);
			return (-1);
		}
		if (len == 0)
			break;
	}
	close(fd);

	memset(ua->visible_endpoint.host, 0, BUFSIZE);
	result = find_ip_address(resp, 8 * BUFSIZE, &s, &len);
	if (result == 0)
		strncpy(ua->visible_endpoint.host, s, len);
	log_msg(LOG_INFO, "Visible IP address: [%s]",
		ua->visible_endpoint.host);
	return 0;
}

int
http_log(sip_user_agent_t ua, char *msg)
{
	char host[128];
	struct in_addr in;
	struct sockaddr_in addr;
	char buf[BUFSIZE], resp[HTTP_RESP_BUFSIZE];
	int fd, len, pos, result;

	if (ua == NULL || msg == NULL || !dns_avail(ua))
		return (-1);

	result = dns_gethostbyname("www.cornfed.com", host, 128);
	if (result < 0)
		return (-1);

	inet_aton(host, &in);

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return (-1);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = in.s_addr;
	addr.sin_port = htons(80);
	result = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
	if (result < 0) {
		close(fd);
		return (-1);
	}
	memset(buf, 0, BUFSIZE);
	sprintf(buf, HTTP_LOG_REQUEST, msg);
	len = write(fd, buf, strlen(buf));
	if (len < 0) {
		close(fd);
		return (-1);
	}
	memset(resp, 0, HTTP_RESP_BUFSIZE);
	for (pos = 0; pos < 8 * BUFSIZE - 1; pos++) {
		len = read(fd, &(resp[pos]), 1);
		if (len < 0) {
			close(fd);
			return (-1);
		}
		if (len == 0)
			break;
	}
	close(fd);
	return 0;
}
