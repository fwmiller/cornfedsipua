#include "codec.h"
#include "codec_alaw.h"
#if _CODEC_G729
#include "codec_g729.h"
#endif
#include "codec_ulaw.h"
#include "sip.h"

int
codec_encode(int type, short *sample, int sample_size,
	     char *frame, int *frame_size)
{
	switch (type) {
#if _CODEC_G729
	case RTP_PAYLOAD_G729:
		g729_encode(sample, sample_size, frame, frame_size);
		return 0;
#endif
	case RTP_PAYLOAD_G711_ALAW:
		alaw_encode(sample, sample_size, frame, frame_size);
		return 0;

	case RTP_PAYLOAD_G711_ULAW:
		ulaw_encode(sample, sample_size, frame, frame_size);
		return 0;
	}
	return (-1);
}

int
codec_decode(int type, char *frame, int frame_size,
	     short *sample, int *sample_size)
{
	switch (type) {
#if _CODEC_G729
	case RTP_PAYLOAD_G729:
		g729_decode(frame, frame_size, sample, sample_size);
		return 0;
#endif
	case RTP_PAYLOAD_G711_ALAW:
		alaw_decode(frame, frame_size, sample, sample_size);
		return 0;

	case RTP_PAYLOAD_G711_ULAW:
		ulaw_decode(frame, frame_size, sample, sample_size);
		return 0;
	}
	return (-1);
}
