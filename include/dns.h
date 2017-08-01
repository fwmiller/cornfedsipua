#ifndef __DNS_H
#define __DNS_H

#include "sip.h"

void dns_init(void);
int dns_avail(sip_user_agent_t ua);
int dns_gethostbyname(char *host, char *ipaddr, int len);

#endif
