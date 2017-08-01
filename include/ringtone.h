#ifndef __RINGTONE_H
#define __RINGTONE_H

#include "sip.h"

struct ringtone {
	int no_ringtone;
	char file[BUFSIZE];
	short *wdata;
	int wdata_len;
	char device[BUFSIZE];
	int fd;
};

typedef struct ringtone *ringtone_t;

int ringtone_file_init(ringtone_t ringtone, const char *ringtone_file);
int ringtone_device_init(sip_user_agent_t ua, char *ringtone_device);
void ringtone_play(sip_user_agent_t ua);

#endif
