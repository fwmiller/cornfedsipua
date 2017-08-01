#ifndef __NET_H
#define __NET_H

#include "sip.h"

int net_init(int port);
int net_get_ifaddr(sip_user_agent_t ua);
int net_send(int fd, char *host, int port, char *buf, int len);
int net_recv(int fd, char *buf, int len);

#endif
