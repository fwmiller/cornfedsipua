#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "log.h"
#include "sdp.h"

static int
sdp_parse_connection(sip_user_agent_t ua, msglines_t msglines)
{
	char *conn;

	if (ua == NULL || msglines == NULL)
		return (-1);

	/* Look for remote RTP host in offered SDP */
	conn = msglines_find_hdr(msglines, "c=");
	if (conn == NULL) {
		log_msg(LOG_WARNING, "Connection data not found");
		sip_dialog_init(ua->dialog);
		return (-1);

	} else {
		char s[BUFSIZE];
		int pos = 2;

		memset(s, 0, BUFSIZE);
		nextarg(conn, &pos, " ", s);
		memset(s, 0, BUFSIZE);
		nextarg(conn, &pos, " ", s);
		memset(s, 0, BUFSIZE);
		nextarg(conn, &pos, " /", s);
		strcpy(ua->rtp.remote.host, s);
	}
	return 0;
}

static void
sdp_codec_list_clear(sdp_codec_list_t codec_list)
{
	int i;

	if (codec_list == NULL)
		return;

	for (i = 0; i < SDP_MAX_CODECS; i++)
		codec_list[i].codec = (-1);
}

static int
sdp_parse_media(sip_user_agent_t ua, msglines_t msglines,
		sdp_codec_list_t codec_list)
{
	char *media;

	if (ua == NULL || msglines == NULL || codec_list == NULL)
		return (-1);

	/* Look for remote RTP port in offered SDP */
	media = msglines_find_hdr(msglines, "m=");
	if (media == NULL) {
		log_msg(LOG_WARNING, "Media transport address not found");
		sip_dialog_init(ua->dialog);
		rtp_endpoint_init(&(ua->rtp.remote));
		ua->rtp.codec = (-1);
		return (-1);

	} else {
		char s[BUFSIZE];
		int i = 0, pos = 2;

		memset(s, 0, BUFSIZE);
		nextarg(media, &pos, " ", s);
		memset(s, 0, BUFSIZE);
		nextarg(media, &pos, " ", s);
		ua->rtp.remote.port = atoi(s);

		sdp_codec_list_clear(codec_list);

		memset(s, 0, BUFSIZE);
		nextarg(media, &pos, " ", s);
		for (;;) {
			int codec;

			memset(s, 0, BUFSIZE);
			nextarg(media, &pos, " ", s);
			if (strlen(s) == 0)
				break;

			codec = atoi(s);
			if (codec < RTP_PAYLOAD_DYNAMIC) {
				codec_list[i++].codec = codec;
				if (i == SDP_MAX_CODECS)
					break;
			}
		}
	}
	return 0;
}

int
sdp_parse(sip_user_agent_t ua, msglines_t msglines,
	  sdp_codec_list_t codec_list)
{
	int result;

	if (ua == NULL || msglines == NULL || codec_list == NULL)
		return (-1);

	result = sdp_parse_connection(ua, msglines);
	if (result < 0)
		return (-1);

	result = sdp_parse_media(ua, msglines, codec_list);
	if (result < 0)
		return (-1);

	return 0;
}

int
sdp_choose_codec(sdp_codec_list_t codec_list)
{
	int i;

	if (codec_list == NULL)
		return (-1);

	for (i = 0; i < SDP_MAX_CODECS; i++) {
		if (codec_list[i].codec == (-1))
			break;

		if (codec_list[i].codec == RTP_PAYLOAD_G711_ULAW) {
			log_msg(LOG_INFO, "Codec: G.711 mu-law");
			return RTP_PAYLOAD_G711_ULAW;
		}
		if (codec_list[i].codec == RTP_PAYLOAD_G711_ALAW) {
			log_msg(LOG_INFO, "Codec: G.711 a-law");
			return RTP_PAYLOAD_G711_ALAW;
		}
#if _CODEC_G729
		if (codec_list[i].codec == RTP_PAYLOAD_G729) {
			log_msg(LOG_INFO, "Codec: G.729");
			return RTP_PAYLOAD_G729;
		}
#endif
	}
	return (-1);
}
