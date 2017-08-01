#ifndef __HISTORY_H
#define __HISTORY_H

#include <time.h>
#include "sip.h"

#define HIST_EVENT_NULL		0
#define HIST_EVENT_INITIATED	1
#define HIST_EVENT_CANCELED	2
#define HIST_EVENT_RECEIVED	3
#define HIST_EVENT_CONNECTED	4
#define HIST_EVENT_HANGUP	5

#define HIST_ENDPOINT_SIZE	128

struct history_event {
	struct history_event *next;
	time_t time;
	int event;
	char endpoint[HIST_ENDPOINT_SIZE];
};

typedef struct history_event *history_event_t;

int history_init(void);
void history_dump_event(history_event_t e, char *s);
void history_dump(void);
int history_read_file(void);
int history_write_file(void);
void history_add_event(sip_user_agent_t ua, int event, char *endpoint);
history_event_t history_first_event(void);
history_event_t history_next_event(history_event_t e);
void history_clear(sip_user_agent_t ua);

#endif
