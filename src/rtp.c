#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "codec.h"
#include "log.h"
#include "net.h"
#include "sip.h"
#include "stun.h"

void
rtp_endpoint_init(rtp_endpoint_t rtp)
{
	if (rtp == NULL)
		return;

	memset(rtp->host, 0, BUFSIZE);
	rtp->port = (-1);
}

void
rtp_session_clear(rtp_session_t rtp)
{
	if (rtp == NULL)
		return;

	rtp->flags = 0;
	rtp->rtpfd = (-1);
	rtp_endpoint_init(&(rtp->local));
	rtp_endpoint_init(&(rtp->visible));
	rtp_endpoint_init(&(rtp->remote));
	rtp->seqno = 0;
	rtp->timestamp = 0;
	pthread_mutex_init(&(rtp->mutex), NULL);
	rtp_stats_clear_session(&(rtp->stats));
	rtp->codec = (-1);
	timerclear(&(rtp->last_cn));
}

static void
rtp_recv(sip_user_agent_t ua, char *buf, int len)
{
	rtp_hdr_t rh;
	unsigned char *payload_offset;
	int payload_size, sample_size;
	char lin[BUFSIZE];
	u32_t magic;
#if 0
	struct timeval now;
	struct timezone tz;
#endif
	int i, result;

	if (ua == NULL || buf == NULL)
		return;

	/* Check for a STUN response */
	magic = ntohl(STUN_MAGIC);
	if (*((u32_t *) (buf + 4)) == magic) {
		stun_receive_response(ua, buf, len);
		return;
	}
	rh = (rtp_hdr_t) buf;

	/* Check payload type */
	if (rh->pl.pt != ua->rtp.codec) {
		if (rh->pl.pt != RTP_PAYLOAD_CN &&
		    rh->pl.pt != RTP_PAYLOAD_CN_OLD)
			return;

		/* Start comfort noise generation */
		if (!(ua->rtp.flags & RTPF_CN_ENABLED)) {
			struct timezone tz;

			log_msg(LOG_INFO, "Start comfort noise");

			ua->rtp.flags |= RTPF_CN_ENABLED;
			gettimeofday(&(ua->rtp.last_cn), &tz);


			/* Send first comfort noise sample to soundcard */
			memset(lin, 0, BUFSIZE);
			soundcard_write(&(ua->soundcard), lin,
					2 * SOUNDCARD_SAMPLE_SIZE);

		}
		return;
	}
	/* Turn off comfort noise if it is being generated */
	if (ua->rtp.flags & RTPF_CN_ENABLED) {
		log_msg(LOG_INFO, "Stop comfort noise");

		ua->rtp.flags &= ~RTPF_CN_ENABLED;
		timerclear(&(ua->rtp.last_cn));
	}
	rh->seq = ntohs(rh->seq);
	rh->ts = ntohl(rh->ts);
	rh->ssrc = ntohl(rh->ssrc);

	/* Update statistics */
	ua->rtp.stats.total_packets_in++;
	ua->rtp.stats.total_bytes_in += len;
	ua->rtp.stats.session_packets_in++;
	ua->rtp.stats.session_bytes_in += len;
	if (ua->rtp.stats.last_seq != 0xffff) {
		if (ua->rtp.stats.last_seq + 1 != rh->seq) {
			ua->rtp.stats.total_packets_missing_in++;
			ua->rtp.stats.session_packets_missing_in++;
		}
	}
	ua->rtp.stats.last_seq = rh->seq;

	/* Calculate update to accumulated jitter */
#if 0
	/*
	 * This version is based on measured interarrival times of RTP
	 * packets
	 */
	gettimeofday(&now, &tz);
	if (timerisset(&(ua->rtp.stats.last_arrival))) {
		struct timeval interarrival;

		timersub(&now, &(ua->rtp.stats.last_arrival),
			 &interarrival);

		if (timerisset(&(ua->rtp.stats.last_interarrival))) {
			struct timeval delta;
			unsigned long delta_uint;

			if (timercmp(&interarrival,
				     &(ua->rtp.stats.last_interarrival), <))
				timersub(&(ua->rtp.stats.last_interarrival),
					 &interarrival, &delta);
			else
				timersub(&interarrival,
					 &(ua->rtp.stats.last_interarrival),
					 &delta);

			delta_uint = delta.tv_sec * 1000000 + delta.tv_usec;
			ua->rtp.stats.session_jitter_in +=
				delta_uint -
				((ua->rtp.stats.session_jitter_in + 8) >> 4);
		}
		ua->rtp.stats.last_interarrival.tv_sec =
			interarrival.tv_sec;
		ua->rtp.stats.last_interarrival.tv_usec =
			interarrival.tv_usec;
	}
	ua->rtp.stats.last_arrival.tv_sec = now.tv_sec;
	ua->rtp.stats.last_arrival.tv_usec = now.tv_usec;
#endif
	/*
	 * This version is based on the timestamp value included in each
	 * RTP packet
	 */
	if (ua->rtp.stats.last_ts > 0) {
		unsigned int interarrival;

		if (rh->ts > ua->rtp.stats.last_ts)
			interarrival = rh->ts - ua->rtp.stats.last_ts;
		else
			interarrival =
				0xffffffff - ua->rtp.stats.last_ts + rh->ts;

		if (ua->rtp.stats.last_interarrival_ts > 0) {
			unsigned long delta;

			if (interarrival <
			    ua->rtp.stats.last_interarrival_ts)
				delta = ua->rtp.stats.last_interarrival_ts -
					interarrival;
			else
				delta = interarrival -
					ua->rtp.stats.last_interarrival_ts;

			ua->rtp.stats.session_jitter_in +=
				delta -
				((ua->rtp.stats.session_jitter_in + 8) >> 4);
		}
		ua->rtp.stats.last_interarrival_ts = interarrival;
	}
	ua->rtp.stats.last_ts = rh->ts;

	/* Convert from codec to signed 16-bit linear */
	payload_size = len - (sizeof(struct rtp_hdr) + (rh->ver.cc << 2));
	payload_offset = ((unsigned char *) buf) + (len - payload_size);
	sample_size = BUFSIZE;
	result = codec_decode(ua->rtp.codec, (char *) payload_offset,
			      payload_size, (short *) lin, &sample_size);
	if (result < 0)
		return;

	/* Send converted payload to sound card */
	soundcard_write(&(ua->soundcard), lin, 2 * sample_size);


	/*
	 * If recording, convert the signed 16-bit linear samples to
	 * unsigned 8-bit linear and write them to the recording file
	 */
	if (ua->flags & SUAF_RECORDING) {
		unsigned char buf[BUFSIZE];
		int result;

		for (i = 0; i < payload_size; i++)
			buf[i] = ((unsigned char)
				(((short *) lin)[i] >> 8)) + 0x80;

		result = write(ua->recordfd, buf, payload_size);
		if (result >= 0)
			ua->record_cnt += payload_size;
	}
}

static void
rtp_check_cn(sip_user_agent_t ua)
{
	if (ua == NULL)
		return;

	/* Check whether comfort noise is enabled */
	if (ua->rtp.flags & RTPF_CN_ENABLED) {
		struct timeval now, dur;
		struct timezone tz;

		gettimeofday(&now, &tz);
		timersub(&now, &(ua->rtp.last_cn), &dur);
		if (dur.tv_sec > 0 || dur.tv_usec > 20000) {
			

			char lin[BUFSIZE];
			/* Send comfort noise sample to soundcard */
			memset(lin, 0, BUFSIZE);
			soundcard_write(&(ua->soundcard), lin,
					2 * SOUNDCARD_SAMPLE_SIZE);

			dur.tv_sec = 0;
			dur.tv_usec = 20000;
			timeradd(&(ua->rtp.last_cn), &dur,
				 &(ua->rtp.last_cn));
		}
	}
}

void
rtp_thread_func(sip_user_agent_t ua)
{
	fd_set rfds;
	struct timeval timeout;

	char buf[BUFSIZE];
	int fd, len, result;

	if (ua == NULL)
		return;

	fd = ua->rtp.rtpfd;
	if (fd < 0)
		return;

	/* Check for an incoming RTP packet */
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	timerclear(&timeout);
	result = select(fd + 1, &rfds, NULL, NULL, &timeout);
	if (result == 0) {
		/* Check whether comfort noise is enabled */
		rtp_check_cn(ua);
		return;

	} else if (result > 0) {
		len = net_recv(fd, buf, BUFSIZE);
		if (len <= 0)
			return;

		rtp_recv(ua, buf, len);
		return;
	}
}

int
rtp_init_port(rtp_session_t rtp, int port)
{
	if (rtp == NULL)
		return (-1);

	if (rtp->rtpfd >= 0) {
		int result = close(rtp->rtpfd);

		if (result < 0) {
			log_msg(LOG_ERROR, "Close RTP port %d failed (%s)",
				rtp->local.port, strerror(errno));
			return (-1);
		}
	}
	rtp->rtpfd = net_init(port);
	if (rtp->rtpfd < 0) {
		log_msg(LOG_ERROR, "Initialize RTP port failed");
		return (-1);
	}
	return 0;
}

void
rtp_init(sip_user_agent_t ua)
{
	int result;

	if (ua == NULL)
		return;

	/* Initialize local RTP endpoint */
	rtp_endpoint_init(&(ua->rtp.local));
	strcpy(ua->rtp.local.host, ua->local_endpoint.host);
	ua->rtp.local.port = ua->rtp_port;

	/* Initialize remote RTP endpoint */
	rtp_endpoint_init(&(ua->rtp.remote));

	/* Initialize RTP socket */
	result = rtp_init_port(&(ua->rtp), ua->rtp.local.port);
	if (result < 0)
		exit(-1);
}

int
rtp_send(rtp_session_t rtp, char *buf, int len, int payload_type)
{
	char packet[BUFSIZE];
	rtp_hdr_t rh;
	
	if (rtp == NULL || buf == NULL)
		return (-1);

		

	rh = (rtp_hdr_t) packet;
	rh->ver.cc = 0;
	rh->ver.x = 0;
	rh->ver.p = 0;
	rh->ver.v = 2;		       /* RFC 3550 - Version 2 */
	rh->pl.pt = payload_type;
	rh->pl.m = 0;
	rh->seq = htons((rtp->seqno)++);
	rh->ts = htonl(rtp->timestamp);
	rh->ssrc = 0;
	memcpy(packet + sizeof(struct rtp_hdr), buf, len);

	/* XXX Update timestamp based on assumed 8 KHz sample clock */
	rtp->timestamp += SOUNDCARD_SAMPLE_SIZE;


	/* Update statistics */
	rtp->stats.total_packets_out++;
	rtp->stats.total_bytes_out += len;
	rtp->stats.session_packets_out++;
	rtp->stats.session_bytes_out += len;

	return net_send(rtp->rtpfd, rtp->remote.host, rtp->remote.port,
			packet, sizeof(struct rtp_hdr) + len);
}

char *
rtp_get_local_host(sip_user_agent_t ua, char *remote_host)
{
	if (ua == NULL || remote_host == NULL)
		return NULL;

	if (!(ua->flags & SUAF_LOCAL_NAT))
		return ua->rtp.local.host;

	if (strlen(ua->rtp.visible.host) > 0) {
		struct in_addr lhost, mask, rhost, vhost;

		inet_aton(ua->rtp.local.host, &lhost);
		inet_aton(ua->local_netmask, &mask);
		inet_aton(ua->rtp.visible.host, &vhost);

		if ((vhost.s_addr & mask.s_addr) ==
		    (lhost.s_addr & mask.s_addr))
			return ua->rtp.local.host;

		if (remote_host != NULL && strlen(remote_host) > 0) {
			inet_aton(remote_host, &rhost);
			if ((rhost.s_addr & mask.s_addr) !=
			    (lhost.s_addr & mask.s_addr))
				return ua->rtp.visible.host;
		}
	}
	return ua->rtp.local.host;
}

int
rtp_get_local_port(sip_user_agent_t ua, char *remote_host)
{
	if (ua == NULL || remote_host == NULL)
		return (-1);

	if (!(ua->flags & SUAF_LOCAL_NAT))
		return ua->rtp.local.port;

	if (strlen(ua->rtp.visible.host) > 0) {
		struct in_addr lhost, mask, rhost, vhost;

		inet_aton(ua->rtp.local.host, &lhost);
		inet_aton(ua->local_netmask, &mask);
		inet_aton(ua->rtp.visible.host, &vhost);

		if ((vhost.s_addr & mask.s_addr) ==
		    (lhost.s_addr & mask.s_addr))
			return ua->rtp.local.port;

		if (remote_host != NULL && strlen(remote_host) > 0) {
			inet_aton(remote_host, &rhost);
			if ((rhost.s_addr & mask.s_addr) !=
			    (lhost.s_addr & mask.s_addr))
				return ua->rtp.visible.port;
		}
	}
	return ua->rtp.local.port;
}
