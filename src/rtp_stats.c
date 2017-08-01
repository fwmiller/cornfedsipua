#include <string.h>
#include "sip.h"

void
rtp_stats_clear_all(rtp_stats_t rtp_stats)
{
	if (rtp_stats == NULL)
		return;

	memset(rtp_stats, 0, sizeof(struct rtp_stats));
	rtp_stats->last_seq = 0xffff;
}

void
rtp_stats_clear_session(rtp_stats_t rtp_stats)
{
	if (rtp_stats == NULL)
		return;

	rtp_stats->session_connect_duration = 0;
	rtp_stats->session_packets_in = 0;
	rtp_stats->session_bytes_in = 0;
	rtp_stats->session_packets_missing_in = 0;
	rtp_stats->session_packets_out = 0;
	rtp_stats->session_bytes_out = 0;
	rtp_stats->session_jitter_in = 0;

	rtp_stats->last_seq = 0xffff;
	rtp_stats->session_start = 0;
#if 0
	timerclear(&(rtp_stats->last_arrival));
	timerclear(&(rtp_stats->last_interarrival));
#endif
	rtp_stats->last_ts = 0;
	rtp_stats->last_interarrival_ts = 0;
}

void
rtp_stats_start_session(rtp_stats_t rtp_stats)
{
	if (rtp_stats == NULL)
		return;

	rtp_stats_clear_session(rtp_stats);
	rtp_stats->session_start = time(NULL);
}

void
rtp_stats_stop_session(rtp_stats_t rtp_stats)
{
	time_t dur;

	if (rtp_stats == NULL || rtp_stats->session_start == 0)
		return;

	dur = time(NULL) - rtp_stats->session_start;
	rtp_stats->session_connect_duration = dur;
	rtp_stats->total_connect_duration += dur;
	rtp_stats->session_start = 0;
}
