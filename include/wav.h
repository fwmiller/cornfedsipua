#ifndef __WAV_H
#define __WAV_H

#include "sip.h"

struct wav_hdr {
	char chunk_id[4];
	unsigned long chunk_size;
	char format[4];
	char subchunk1_id[4];
	unsigned long subchunk1_size;
	unsigned short audio_format;
	unsigned short num_channels;
	unsigned long sample_rate;
	unsigned long byte_rate;
	unsigned short block_align;
	unsigned short bits_per_sample;
	char subchunk2_id[4];
	unsigned long subchunk2_size;
};

struct wav_rec {
	struct wav_rec *next;
	char *cdata;
	int len;
	int frame_size;
	int pos;
};

struct wav_rec_list {
	struct wav_rec *wav_recs;
	pthread_mutex_t mutex;
};

typedef struct wav_hdr *wav_hdr_t;
typedef struct wav_rec *wav_rec_t;
typedef struct wav_rec_list *wav_rec_list_t;

void wav_dump(sip_user_agent_t ua);
char *wav_load(const char *filename, int *len);
int wav_play(sip_user_agent_t ua, char *filename);
void wav_start(sip_user_agent_t ua, char *cdata, int len, int sample_size);
void wav_send(sip_user_agent_t ua);
void wav_rec_list_init(wav_rec_list_t wavs);
void wav_rec_list_flush(sip_user_agent_t ua);

#endif
