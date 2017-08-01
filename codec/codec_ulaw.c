/*
 * Craig Reese: IDA/Supercomputing Research Center
 * Joe Campbell: Department of Defense
 * 29 September 1989
 *
 * References:
 * 1) CCITT Recommendation G.711  (very difficult to follow)
 * 2) "A New Digital Technique for Implementation of Any
 *     Continuous PCM Companding Law," Villeret, Michel,
 *     et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
 *     1973, pg. 11.12-11.17
 * 3) MIL-STD-188-113,"Interoperability and Performance Standards
 *     for Analog-to_Digital Conversion Techniques,"
 *     17 February 1987
 *
 */

#include <string.h>
#include "codec_ulaw.h"

#define ZEROTRAP		       /* Turn on trap per MIL-STD-188-113 */
#define BIAS		0x84	       /* Define bias for 16-bit samples */
#define CLIP		32635

static unsigned char __lin2ulaw[65536];
static short __ulaw2lin[256];

static unsigned char
linear2ulaw(int sample)
{
	static int exp_lut[256] = {
		0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
	};
	int sign, exponent, mantissa;
	unsigned char ulawbyte;

	/* Convert sample to sign-magnitude */
	sign = (sample >> 8) & 0x80;   /* Set aside sign */
	if (sign != 0)
		sample = -sample;      /* Get magnitude */
	if (sample > CLIP)
		sample = CLIP;	       /* Clip magnitude */

	/* Convert from 16-bit linear to ulaw */
	sample = sample + BIAS;
	exponent = exp_lut[(sample >> 7) & 0xff];
	mantissa = (sample >> (exponent + 3)) & 0x0f;
	ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
	if (ulawbyte == 0)
		ulawbyte = 0x02;       /* optional CCITT trap */
#endif
	return ulawbyte;
}

static int
ulaw2linear(unsigned char ulawbyte)
{
	static int exp_lut[8] =
	    { 0, 132, 396, 924, 1980, 4092, 8316, 16764 };
	int sign, exponent, mantissa, sample;

	ulawbyte = ~ulawbyte;
	sign = (ulawbyte & 0x80);
	exponent = (ulawbyte >> 4) & 0x07;
	mantissa = ulawbyte & 0x0f;
	sample = exp_lut[exponent] + (mantissa << (exponent + 3));
	if (sign != 0)
		sample = -sample;
	return sample;
}

void
ulaw_init()
{
	int i;

	memset(__lin2ulaw, 0, sizeof(__lin2ulaw));
	memset(__ulaw2lin, 0, sizeof(__ulaw2lin));

	for (i = -32768; i < 32768; i++)
		__lin2ulaw[(unsigned short) i] = linear2ulaw(i);

	for (i = 0; i < 256; i++)
		__ulaw2lin[i] = ulaw2linear((unsigned char) i);
}

static unsigned char
lin2ulaw(short sample)
{
	return __lin2ulaw[(unsigned short) sample];
}

static short
ulaw2lin(unsigned char ulawbyte)
{
	return __ulaw2lin[ulawbyte];
}

int
ulaw_encode(short *sample, int samplelen, char *frame, int *framelen)
{
	int i;

	for (i = 0; i < samplelen;) {
		frame[i] = lin2ulaw(sample[i]);
		i++;
		if (*framelen < samplelen && i == *framelen)
			return (-1);
	} 
	*framelen = i;
	return 0;
}
 
int
ulaw_decode(char *frame, int framelen, short *sample, int *samplelen)
{
	int i;
 
	for (i = 0; i < framelen;) {
		sample[i] = ulaw2lin(frame[i]);
		i++;
		if (*samplelen < framelen && i == *samplelen)
			return (-1);
	} 
	*samplelen = i;
	return 0;
}
