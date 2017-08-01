#include <ctype.h>
#include <string.h>
#include "ipendpoint.h"

void
ipendpoint_init(ipendpoint_t endpoint)
{
	if (endpoint == NULL)
		return;

	memset(endpoint->domain, 0, BUFSIZE);
	memset(endpoint->host, 0, BUFSIZE);
	endpoint->port = (-1);
}

static int
ipendpoint_valid_domain(ipendpoint_t endpoint)
{
	int i, len;

	/*
	 * XXX This routine needs to be tightend up
	 */
	len = strlen(endpoint->domain);
	if (len == 0)
		return 0;

	for (i = 0; i < len; i++)
		if (isalnum(endpoint->domain[i]) ||
		    endpoint->domain[i] == '.')
			return 1;

	return 0;
}

static int
ipendpoint_valid_ipaddr(ipendpoint_t endpoint)
{
	int i, len;

	/*
	 * XXX This routine needs to be tightend up
	 */
	len = strlen(endpoint->host);
	if (len == 0)
		return 0;

	for (i = 0; i < len; i++)
		if (isdigit(endpoint->host[i]) ||
		    endpoint->host[i] == '.')
			return 1;

	return 0;
}

int
ipendpoint_isset(ipendpoint_t endpoint)
{
	if (endpoint == NULL)
		return 0;

	if ((ipendpoint_valid_domain(endpoint) ||
	     ipendpoint_valid_ipaddr(endpoint)) &&
	    (endpoint->port > 0 && endpoint->port < 0x10000))
		return 1;

	return 0;
}

int
ipendpoint_cmp(ipendpoint_t endpoint1, ipendpoint_t endpoint2)
{
	int len1, len2;

	if (endpoint1 == NULL || endpoint2 == NULL)
		return (-1);

	len1 = strlen(endpoint1->domain);
	len2 = strlen(endpoint2->domain);

	if (len1 > 0 && len2 > 0) {
		if (strcmp(endpoint1->domain, endpoint2->domain) != 0)
			return (-1);

	} else if (len1 > 0 || len2 > 0)
		return (-1);

	else if (strcmp(endpoint1->host, endpoint2->host) != 0)
		return (-1);

	if (endpoint1->port != endpoint2->port)
		return (-1);

	return 0;
}
