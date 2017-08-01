#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cli.h"
#include "lex.h"
#include "wav.h"

static void
cli_record_dump_state()
{
	printf("\n");
	printf("Record file [%s]\n", ua.record_file);
	printf("Recording length %u bytes\n", (unsigned int) ua.record_cnt);
	printf("\n");
}

void
cli_record_wav(char *cmdline)
{
	char s[BUFSIZE];
	int pos = 0, result;

	/* Skip command word */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	/* Get first argument, if any */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, &pos, " ", s);

	if (strcmp(s, "start") == 0) {
		struct wav_hdr wav_hdr;
		int fd;

		if (ua.flags & SUAF_RECORDING) {
			printf("Recording already in progress\n");
			return;
		}
		/* Remove any existing recording */
		result = unlink(ua.record_file);
		if (result < 0 && errno != ENOENT) {
			printf("Remove %s failed (%s)\n",
			       ua.record_file, strerror(errno));
			return;
		}
		/* XXX Create new record file */
		fd = open(ua.record_file, O_WRONLY | O_CREAT, 0666);
		if (fd < 0) {
			printf("Create %s failed (%s)\n",
			       ua.record_file, strerror(errno));
			return;
		}
		ua.recordfd = fd;

		/*
		 * XXX Insert .wav file header template in new record
		 * file
		 */
		memset(&wav_hdr, 0, sizeof(struct wav_hdr));
		strncpy(wav_hdr.chunk_id, "RIFF", 4);
		strncpy(wav_hdr.format, "WAVE", 4);
		strncpy(wav_hdr.subchunk1_id, "fmt ", 4);
		wav_hdr.subchunk1_size = 16;
		wav_hdr.audio_format = 1;
		wav_hdr.num_channels = 1;
		wav_hdr.sample_rate = 8000;
		wav_hdr.byte_rate = 8000;
		wav_hdr.block_align = 1;
		wav_hdr.bits_per_sample = 8;
		strncpy(wav_hdr.subchunk2_id, "data", 4);

		result = write(ua.recordfd, &wav_hdr,
			       sizeof(struct wav_hdr));
		if (result < 0) {
			printf("Write failed (%s)\n", strerror(errno));
			close(ua.recordfd);
			ua.recordfd = 0;
			return;
		}
		/* Clear recording length */
		ua.record_cnt = 0;

		/* Signal start of recording */
		ua.flags |= SUAF_RECORDING;

		printf("Recording started\n");

	} else if (strcmp(s, "stop") == 0) {
		if (!(ua.flags & SUAF_RECORDING)) {
			printf("Not recording\n");
			return;
		}
		/* Signal end of recording */
		ua.flags &= ~SUAF_RECORDING;

		printf("Recording stopped\n");

		/* XXX Close record file */
		close(ua.recordfd);
		ua.recordfd = (-1);

		/* XXX Update record file to reflect length of recording */
		{
			struct wav_hdr wav_hdr;
			int fd;

			fd = open(ua.record_file, O_RDWR);
			if (fd < 0) {
				printf("Open %s failed (%s)\n",
				       ua.record_file, strerror(errno));
				return;
			}
			memset(&wav_hdr, 0, sizeof(struct wav_hdr));
			result = read(fd, &wav_hdr, sizeof(struct wav_hdr));
			if (result < 0) {
				printf("Read failed (%s)\n",
				       strerror(errno));
				close(fd);
				return;
			}
			wav_hdr.chunk_size = ua.record_cnt +
					     sizeof(struct wav_hdr);
			wav_hdr.subchunk2_size = ua.record_cnt;

			lseek(fd, 0, SEEK_SET);
			result = write(fd, &wav_hdr, sizeof(struct wav_hdr));
			if (result < 0) {
				printf("Write failed (%s)\n",
				       strerror(errno));
				close(fd);
				return;
			}
			close(fd);
		}

	} else if (strcmp(s, "file") == 0) {
		memset(s, 0, BUFSIZE);
		nextarg(cmdline, &pos, " ", s);
		strcpy(ua.record_file, s);

		cli_record_dump_state();

	} else if (strlen(s) == 0)
		cli_record_dump_state();

	else
		cli_help("help record");
}
