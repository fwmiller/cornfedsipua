#ifndef __ULAW_H
#define __ULAW_H

#define ULAW_SAMPLE_SIZE	160

void ulaw_init(void);
int ulaw_encode(short *sample, int samplelen, char *frame, int *framelen);
int ulaw_decode(char *frame, int framelen, short *sample, int *samplelen);

#endif
