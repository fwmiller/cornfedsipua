#ifndef __HTTP_H
#define __HTTP_H

#include "sip.h"


int find_ip_address(char *s, int len, char **addr, int *addrlen);
int http_get_myipaddr(sip_user_agent_t ua);
int http_log(sip_user_agent_t ua, char *msg);




#endif
