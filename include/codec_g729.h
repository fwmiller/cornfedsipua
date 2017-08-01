#ifndef __G729_H
#define __G729_H

#define G729_SAMPLE_SIZE	80
#define G729_FRAME_SIZE		10

void g729_init(void);
int g729_encode(short *sample, int samplelen, char *frame, int *framelen);
int g729_decode(char *frame, int framelen, short *sample, int *samplelen);

#endif
