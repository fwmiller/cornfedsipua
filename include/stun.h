#ifndef __STUN_H
#define __STUN_H

#include "sip.h"

#define STUN_PORT	3478
#define STUN_MAGIC	0x2112A442

struct stun_hdr {
	u16_t type;
	u16_t length;
	u32_t magic;
	u8_t tid[12];
} __attribute__((packed));

typedef struct stun_hdr *stun_hdr_t;

void stun_gen_tid(u8_t *tid);
void stun_gen_binding_request(u8_t *tid, u8_t *msg);
u8_t *stun_find_attr(u8_t *msg, int len, u16_t type);

int
stun_parse_binding_response(u8_t *msg,
			    char *mapped_address,
			    int *mapped_port);

void stun_dump_msg(char *action, u8_t *hdr);
void stun_init(sip_user_agent_t ua, char *host);
void stun_receive_response(sip_user_agent_t ua, char *msg, int len);
void stun_thread_func(sip_user_agent_t ua);

#endif
