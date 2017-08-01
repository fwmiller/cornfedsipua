#ifndef __ALAW_H
#define __ALAW_H

#define ALAW_SAMPLE_SIZE	160

void alaw_init(void);
int alaw_encode(short *sample, int samplelen, char *frame, int *framelen);
int alaw_decode(char *frame, int framelen, short *sample, int *samplelen);

#endif
