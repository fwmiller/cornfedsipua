#ifndef __RTP_H
#define __RTP_H

#include <pthread.h>
#include <time.h>
#include "cornfedsipua.h"
#include "sip.h"

#define RTP_PAYLOAD_G711_ULAW	0
#define RTP_PAYLOAD_G721	2
#define RTP_PAYLOAD_GSM		3
#define RTP_PAYLOAD_G723	4
#define RTP_PAYLOAD_G711_ALAW	8
#define RTP_PAYLOAD_G722	9
#define RTP_PAYLOAD_CN		13
#define RTP_PAYLOAD_G728	15
#define RTP_PAYLOAD_G729	18
#define RTP_PAYLOAD_CN_OLD	19
#define RTP_PAYLOAD_DYNAMIC	96
#define RTP_PAYLOAD_DTMF_2833	101

#define RTP_JITTER_WINDOW_SIZE	100

/* RTP session flags */
#define RTPF_CN_ENABLED		0x01

struct rtp_hdr {
	/* Bit ordering is Intel x86 specific */
	struct {
		u8_t cc:4;	       /* CSRC count */
		u8_t x:1;	       /* Extension */
		u8_t p:1;	       /* Padding */
		u8_t v:2;	       /* Version */
	} ver;

	/* Bit ordering is Intel x86 specific */
	struct {
		u8_t pt:7;	       /* Payload type */
		u8_t m:1;	       /* Marker */
	} pl;

	u16_t seq;
	u32_t ts;		       /* Timestamp */
	u32_t ssrc;		       /* SSRC */
	u32_t csrc[0];		       /* CSRCs */
} __attribute__ ((packed));

struct rtp_endpoint {
	char host[BUFSIZE];
	int port;
};

struct rtp_stats {
	/* Client lifetime measurements */
	time_t total_connect_duration;
	unsigned long total_packets_in;
	unsigned long total_bytes_in;
	unsigned long total_packets_missing_in;
	unsigned long total_packets_out;
	unsigned long total_bytes_out;

	/* Current session measurements */
	time_t session_connect_duration;
	unsigned long session_packets_in;
	unsigned long session_bytes_in;
	unsigned long session_packets_missing_in;
	unsigned long session_packets_out;
	unsigned long session_bytes_out;
	unsigned long session_jitter_in; /* microseconds */

	/* Raw data */
	u16_t last_seq;
	time_t session_start;
#if 0
	struct timeval last_arrival;
	struct timeval last_interarrival;
#endif
	unsigned long last_ts;
	unsigned long last_interarrival_ts;
};

struct rtp_session {
	int flags;
	int rtpfd;
	struct rtp_endpoint local;
	struct rtp_endpoint visible;
	struct rtp_endpoint remote;
	unsigned short seqno;
	unsigned long timestamp;
	pthread_mutex_t mutex;
	struct rtp_stats stats;
	int codec;
	struct timeval last_cn;
};

typedef struct rtp_hdr *rtp_hdr_t;
typedef struct rtp_endpoint *rtp_endpoint_t;
typedef struct rtp_stats *rtp_stats_t;
typedef struct rtp_session *rtp_session_t;

void rtp_endpoint_init(rtp_endpoint_t rtp);
void rtp_stats_clear_all(rtp_stats_t rtp_stats);
void rtp_stats_clear_session(rtp_stats_t rtp_stats);
void rtp_stats_start_session(rtp_stats_t rtp_stats);
void rtp_stats_stop_session(rtp_stats_t rtp_stats);
void rtp_session_clear(rtp_session_t rtp);

void rtp_lock(rtp_session_t rtp);
int rtp_trylock(rtp_session_t rtp);
void rtp_unlock(rtp_session_t rtp);

void rtp_thread_func(sip_user_agent_t ua);
int rtp_init_port(rtp_session_t rtp, int port);
void rtp_init(sip_user_agent_t ua);
int rtp_send(rtp_session_t rtp, char *buf, int len, int payload_type);

char *rtp_get_local_host(sip_user_agent_t ua, char *remote_host);
int rtp_get_local_port(sip_user_agent_t ua, char *remote_host);

#endif
