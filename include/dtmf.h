#ifndef __DTMF_H
#define __DTMF_H

#include "sip.h"

#define DTMF_2833_EVENT_0	0
#define DTMF_2833_EVENT_1	1
#define DTMF_2833_EVENT_2	2
#define DTMF_2833_EVENT_3	3
#define DTMF_2833_EVENT_4	4
#define DTMF_2833_EVENT_5	5
#define DTMF_2833_EVENT_6	6
#define DTMF_2833_EVENT_7	7
#define DTMF_2833_EVENT_8	8
#define DTMF_2833_EVENT_9	9
#define DTMF_2833_EVENT_STAR	10
#define DTMF_2833_EVENT_POUND	11
#define DTMF_2833_EVENT_A	12
#define DTMF_2833_EVENT_B	13
#define DTMF_2833_EVENT_C	14
#define DTMF_2833_EVENT_D	15
#define DTMF_2833_EVENT_FLASH	16

struct dtmf_2833_payload {
	u8_t ev;			/* Event */
	/* Bit ordering is Intel x86 specific */
	struct {
		u8_t vol:1;		/* Volume */
		u8_t r1:1;		/* Reserved */
		u8_t e:1;		/* End of event */
		u8_t r2:5;		/* Reserved */
	} __attribute__ ((packed)) flags;
	u16_t dur;			/* Duration */
} __attribute__ ((packed));

typedef struct dtmf_2833_payload *dtmf_2833_payload_t;

void dtmf_send(sip_user_agent_t ua, int digit);

#endif
