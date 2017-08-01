#ifndef __CODEC_H
#define __CODEC_H

int codec_encode(int type, short *sample, int sample_size,
		 char *frame, int *frame_size);

int codec_decode(int type, char *frame, int frame_size,
		 short *sample, int *sample_size);

#endif
