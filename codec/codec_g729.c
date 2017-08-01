#include "codec_g729.h"
#include "g729a.h"

static struct cod_state enc_state;
static struct dec_state dec_state;

void
g729_init()
{
	g729_init_coder(&enc_state);
	g729_init_decoder(&dec_state);
}

int
g729_encode(short *sample, int samplelen, char *frame, int *framelen)
{
	int len, i;

	for (i = 0; i * G729_SAMPLE_SIZE < samplelen; i++) {
		g729_coder(&enc_state, &(sample[i * G729_SAMPLE_SIZE]),
			   &(frame[i * G729_FRAME_SIZE]), &len);
	}
	*framelen = i * G729_FRAME_SIZE;
	return 0;
}

int
g729_decode(char *frame, int framelen, short *sample, int *samplelen)
{
	int i;

	for (i = 0; i * G729_FRAME_SIZE < framelen; i++) {
		g729_decoder(&dec_state, &(sample[i * G729_SAMPLE_SIZE]),
			     &(frame[i * G729_FRAME_SIZE]),
			     G729_FRAME_SIZE);
	}
	*samplelen = i * G729_SAMPLE_SIZE;
	return 0;
}
