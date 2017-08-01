#ifndef __SDP_H
#define __SDP_H

#include "sip.h"

#define SDP_MAX_CODECS	16

struct sdp_codec {
	int codec;
};

typedef struct sdp_codec sdp_codec_list_t[SDP_MAX_CODECS];

int sdp_parse(sip_user_agent_t ua, msglines_t msglines,
	      sdp_codec_list_t codec_list);
int sdp_choose_codec(sdp_codec_list_t codec_list);

#endif
