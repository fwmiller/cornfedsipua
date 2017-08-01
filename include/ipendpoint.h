#ifndef __IP_ENDPOINT_H
#define __IP_ENDPOINT_H

#include "cornfedsipua.h"

struct ipendpoint {
	char domain[BUFSIZE];
	char host[BUFSIZE];
	int port;
};

typedef struct ipendpoint *ipendpoint_t;

void ipendpoint_init(ipendpoint_t endpoint);
int ipendpoint_isset(ipendpoint_t endpoint);
int ipendpoint_cmp(ipendpoint_t endpoint1, ipendpoint_t endpoint2);

#endif
