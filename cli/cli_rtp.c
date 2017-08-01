#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "lex.h"

static void
cli_rtp_dump_state()
{
	printf("\n");
	printf("local host   : [%s]\n", ua.rtp.local.host);
	printf("local port   : %d\n", ua.rtp.local.port);
	printf("visible host : [%s]\n", ua.rtp.visible.host);
	printf("visible port : %d\n", ua.rtp.visible.port);
	printf("remote host  : [%s]\n", ua.rtp.remote.host);
	printf("remote port  : %d\n", ua.rtp.remote.port);
	printf("codec        : ");
	if (ua.rtp.codec == RTP_PAYLOAD_G711_ULAW)
		printf("G.711 mu-law\n");
	else if (ua.rtp.codec == RTP_PAYLOAD_G711_ALAW)
		printf("G.711 a-law\n");
	else if (ua.rtp.codec == RTP_PAYLOAD_G729)
		printf("G.729\n");
	else
		printf("%d\n", ua.rtp.codec);
	printf("\n");
}

void
cli_rtp(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 3;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "stats") == 0) {
		time_t dur;

		if (ua.rtp.stats.session_start > 0) {
			dur = time(NULL) - ua.rtp.stats.session_start;
			ua.rtp.stats.session_connect_duration = dur;
		}
		printf("\n");

#if 0
		if (ua.rtp.stats.session_start == 0)
			dur = ua.rtp.stats.total_connect_duration;
		else
			dur = ua.rtp.stats.total_connect_duration +
			      ua.rtp.stats.session_connect_duration;
		printf("total_connect : %u\n", (unsigned int) dur);

		printf("total_throughput_rx : %u\n",
		       (dur == 0 ? 0 : (unsigned int)
			((ua.rtp.stats.total_bytes_in * 8) / dur)));
		printf("total_packets_rx : %u\n",
		       (unsigned int) ua.rtp.stats.total_packets_in);
		printf("total_bytes_rx : %u\n",
		       (unsigned int) ua.rtp.stats.total_bytes_in);
		printf("total_loss_rx : %u\n",
		       (unsigned int)
			ua.rtp.stats.total_packets_missing_in);
		printf("total_throughput_tx : %u\n",
		       (dur == 0 ? 0 : (unsigned int)
			((ua.rtp.stats.total_bytes_out * 8) / dur)));
		printf("total_packets_tx : %u\n",
		       (unsigned int) ua.rtp.stats.total_packets_out);
		printf("total_bytes_tx : %u\n",
		       (unsigned int) ua.rtp.stats.total_bytes_out);

		dur = ua.rtp.stats.session_connect_duration;
		printf("connect : %u\n", (unsigned int) dur);

		printf("throughput_rx : %u\n",
		       (dur == 0 ? 0 : (unsigned int)
			((ua.rtp.stats.session_bytes_in * 8) / dur)));
		printf("packets_rx : %u\n",
		       (unsigned int) ua.rtp.stats.session_packets_in);
		printf("bytes_rx : %u\n",
		       (unsigned int) ua.rtp.stats.session_bytes_in);
		printf("loss_rx : %u\n",
		       (unsigned int)
			ua.rtp.stats.session_packets_missing_in);
		printf("jitter_rx : %u\n",
		       (unsigned int) ua.rtp.stats.session_jitter_in);
		printf("throughput_tx : %u\n",
		       (dur == 0 ? 0 : (unsigned int)
			((ua.rtp.stats.session_bytes_out * 8) / dur)));
		printf("packets_tx : %u\n",
		       (unsigned int) ua.rtp.stats.session_packets_out);
		printf("bytes_tx : %u\n",
		       (unsigned int) ua.rtp.stats.session_bytes_out);
#endif
		if (ua.rtp.stats.session_start == 0)
			dur = ua.rtp.stats.total_connect_duration;
		else
			dur = ua.rtp.stats.total_connect_duration +
			      ua.rtp.stats.session_connect_duration;
		printf("total connect duration        : %u min %u sec\n",
		       (unsigned int) dur / 60, (unsigned int) dur % 60);

		printf("total throughput in           : %u bits per sec\n",
		       (dur == 0 ? 0 : (unsigned int)
			((ua.rtp.stats.total_bytes_in * 8) / dur)));
		printf("total packets in              : %u\n",
		       (unsigned int) ua.rtp.stats.total_packets_in);
		printf("total bytes in                : %u\n",
		       (unsigned int) ua.rtp.stats.total_bytes_in);
		printf("total packets missing in      : %u\n",
		       (unsigned int)
			ua.rtp.stats.total_packets_missing_in);
		printf("total throughput out          : %u bits per sec\n",
		       (dur == 0 ? 0 : (unsigned int)
			((ua.rtp.stats.total_bytes_out * 8) / dur)));
		printf("total packets out             : %u\n",
		       (unsigned int) ua.rtp.stats.total_packets_out);
		printf("total bytes out               : %u\n",
		       (unsigned int) ua.rtp.stats.total_bytes_out);

		dur = ua.rtp.stats.session_connect_duration;
		printf("session connect duration      : %u min %u sec\n",
		       (unsigned int) dur / 60, (unsigned int) dur % 60);

		printf("session throughput in         : %u bits per sec\n",
		       (dur == 0 ? 0 : (unsigned int)
			((ua.rtp.stats.session_bytes_in * 8) / dur)));
		printf("session packets in            : %u\n",
		       (unsigned int) ua.rtp.stats.session_packets_in);
		printf("session bytes in              : %u\n",
		       (unsigned int) ua.rtp.stats.session_bytes_in);
		printf("session packets missing in    : %u\n",
		       (unsigned int)
			ua.rtp.stats.session_packets_missing_in);
#if 0
		printf("session accumulated jitter in : %u usec\n",
		       (unsigned int) ua.rtp.stats.session_jitter_in);
#endif
		printf("session accumulated jitter in : %u\n",
		       (unsigned int) ua.rtp.stats.session_jitter_in);
		printf("session throughput out        : %u bits per sec\n",
		       (dur == 0 ? 0 : (unsigned int)
			((ua.rtp.stats.session_bytes_out * 8) / dur)));
		printf("session packets out           : %u\n",
		       (unsigned int) ua.rtp.stats.session_packets_out);
		printf("session bytes out             : %u\n",
		       (unsigned int) ua.rtp.stats.session_bytes_out);

		printf("\n");

	} else if (strlen(s) == 0)
		cli_rtp_dump_state();

	else
		cli_help("help rtp");
}
