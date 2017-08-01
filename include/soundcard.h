#ifndef __SOUNDCARD_H
#define __SOUNDCARD_H

#include <sys/soundcard.h>
#include "sip.h"

#define SOUNDCARD_SAMPLING_RATE	8000
#define SOUNDCARD_SAMPLE_SIZE	160

struct soundcard {
	int no_soundcard;
	char device[BUFSIZE];
	int fd;
	int frame_size;
	int ibufsize;
	char *ibuf;
	int icnt;
	int ihead;
	int itail;
};

typedef struct soundcard *soundcard_t;

int soundcard_setup(int fd);
int soundcard_thread_func(sip_user_agent_t ua);
int soundcard_init(sip_user_agent_t ua, char *soundcard_device);
void soundcard_write(soundcard_t soundcard, char *buf, int len);

int soundcard_status(soundcard_t soundcard,
		     struct audio_buf_info *iinfo,
		     struct audio_buf_info *oinfo);

void soundcard_flush(soundcard_t soundcard);

#endif
