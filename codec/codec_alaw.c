#include <string.h>
#include "codec_alaw.h"

static short __alaw2lin[256];
static unsigned char __lin2alaw[65536];

static inline unsigned char
linear2alaw (short int linear)
{
	int mask, pcm, seg;
	static int seg_end[8] = {
		0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff
	};
                                                                                
	pcm = linear;
	if (pcm >= 0) {
		/* Sign (7th) bit = 1 */
		mask = 0x55 | 0x80;
	} else {
		/* Sign bit = 0 */
		mask = 0x55;
		pcm = -pcm;
	}
	/* Convert scaled magnitude to segment number */
	for (seg = 0;  seg < 8;  seg++) {
		if (pcm <= seg_end[seg])
			break;
	}
	/* Combine sign, segment, and quantization bits */
	return  ((seg << 4) |
		 ((pcm >> ((seg) ? (seg + 3) : 4)) & 0x0f)) ^ mask;

}

static inline short
alaw2linear(unsigned char alaw)
{
	int i, seg;

	alaw ^= 0x55;
	i = ((alaw & 0x0f) << 4);
	seg = (((int) alaw & 0x70) >> 4);
	if (seg)
		i = (i + 0x100) << (seg - 1);
	return (short) ((alaw & 0x80) ? i : -i);
}

void
alaw_init()
{
	int i;

	memset(__alaw2lin, 0, sizeof(__lin2alaw));
	memset(__lin2alaw, 0, sizeof(__lin2alaw));

	for (i = 0; i < 256; i++)
		__alaw2lin[i] = alaw2linear(i);

	for (i = -32768; i < 32768; i++)
		__lin2alaw[(unsigned short) i] = linear2alaw(i);
}

static unsigned char
lin2alaw(short sample)
{
	return __lin2alaw[(unsigned short) sample];
}

static short
alaw2lin(unsigned char alawbyte)
{
	return __alaw2lin[alawbyte];
}

int
alaw_encode(short *sample, int samplelen, char *frame, int *framelen)
{
	int i;

	for (i = 0; i < samplelen;) {
		frame[i] = lin2alaw(sample[i]);
		i++;
		if (*framelen < samplelen && i == *framelen)
			return (-1);
	}
	*framelen = i;
	return 0;
}

int
alaw_decode(char *frame, int framelen, short *sample, int *samplelen)
{
	int i;

	for (i = 0; i < framelen;) {
		sample[i] = alaw2lin(frame[i]);
		i++;
		if (*samplelen < framelen && i == *samplelen)
			return (-1);
	}
	*samplelen = i;
	return 0;
}
