#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "codec.h"
#include "log.h"
#include "sip.h"



#if 0
static void
wav_dump_hdr(wav_hdr_t wav_hdr)
{
	char s[128];
	int i;

	if (wav_hdr == NULL)
		return;

	memset(s, 0, 128);
	sprintf(s, "chunk_id [");
	for (i = 0; i < 4; i++)
		sprintf(s + strlen(s), "%c", wav_hdr->chunk_id[i]);
	sprintf(s + strlen(s), "]");
	log_msg(LOG_INFO, s);

	log_msg(LOG_INFO, "chunk_size %u", wav_hdr->chunk_size);

	memset(s, 0, 128);
	sprintf(s, "format [");
	for (i = 0; i < 4; i++)
		sprintf(s + strlen(s), "%c", wav_hdr->format[i]);
	sprintf(s + strlen(s), "]");
	log_msg(LOG_INFO, s);

	memset(s, 0, 128);
	sprintf(s, "subchunk1_id [");
	for (i = 0; i < 4; i++)
		sprintf(s + strlen(s), "%c", wav_hdr->subchunk1_id[i]);
	sprintf(s + strlen(s), "]");
	log_msg(LOG_INFO, s);

	log_msg(LOG_INFO, "subchunk1_size %u", wav_hdr->subchunk1_size);

	if (wav_hdr->audio_format == 1)
		log_msg(LOG_INFO, "audio_format PCM");
	else if (wav_hdr->audio_format == 0x55)
		log_msg(LOG_INFO, "audio_format MP3");
	else
		log_msg(LOG_INFO, "audio_format 0x%04x",
			wav_hdr->audio_format);

	log_msg(LOG_INFO, "num_channels %u", wav_hdr->num_channels);
	log_msg(LOG_INFO, "sample_rate %u", wav_hdr->sample_rate);
	log_msg(LOG_INFO, "byte_rate %u", wav_hdr->byte_rate);
	log_msg(LOG_INFO, "block_align %u", wav_hdr->block_align);
	log_msg(LOG_INFO, "bits_per_sample %u", wav_hdr->bits_per_sample);

	memset(s, 0, 128);
	sprintf(s, "subchunk2_id [");
	for (i = 0; i < 4; i++)
		sprintf(s + strlen(s), "%c", wav_hdr->subchunk2_id[i]);
	sprintf(s + strlen(s), "]");
	log_msg(LOG_INFO, s);

	log_msg(LOG_INFO, "subchunk2_size %u", wav_hdr->subchunk2_size);
}
#endif

static void
wav_dump_rec(wav_rec_t rec, int pos)
{
	if (rec == NULL)
		return;

	log_msg(LOG_INFO, "wav_rec %d len %d frame_size %d pos %d",
		pos, rec->len, rec->frame_size, rec->pos);
}

void
wav_dump(sip_user_agent_t ua)
{
	wav_rec_t rec;
	int i;

	if (ua == NULL)
		return;

	pthread_mutex_lock(&(ua->wavs.mutex));

	for (rec = ua->wavs.wav_recs, i = 1;
	     rec != NULL;
	     rec = rec->next, i++) {
		if (i == 1)
			printf("\n");
		wav_dump_rec(rec, i);
	}
	pthread_mutex_unlock(&(ua->wavs.mutex));

	if (i > 1)
		printf("\n");
}

char *
wav_load(const char *filename, int *len)
{
	wav_hdr_t wav_hdr;
	char buf[sizeof(struct wav_hdr)];
	char *data;
	int fd, result;

	if (filename == NULL || len == NULL)
		return NULL;

	log_msg(LOG_INFO, "Load [%s] .wav file", filename);

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		log_msg(LOG_INFO, "Open %s failed (%s)",
			filename, strerror(fd));
		*len = 0;
		return NULL;
	}
	memset(buf, 0, sizeof(struct wav_hdr));
	result = read(fd, buf, sizeof(struct wav_hdr));
	if (result < 0) {
		log_msg(LOG_INFO, "Read %s failed (%s)",
			filename, strerror(fd));
		close(fd);
		*len = 0;
		return NULL;
	}
	wav_hdr = (wav_hdr_t) buf;
#if 0
	wav_dump_hdr(wav_hdr);
#endif
	if (strncmp(wav_hdr->chunk_id, "RIFF", 4) != 0 ||
	    strncmp(wav_hdr->format, "WAVE", 4) != 0 ||
	    strncmp(wav_hdr->subchunk1_id, "fmt ", 4) != 0 ||
	    wav_hdr->audio_format != 1 ||
	    wav_hdr->num_channels != 1 ||
	    wav_hdr->byte_rate != 8000 ||
	    wav_hdr->bits_per_sample != 8 ||
	    strncmp(wav_hdr->subchunk2_id, "data", 4) != 0) {
		close(fd);
		*len = 0;
		return NULL;
	}
	data = malloc(wav_hdr->subchunk2_size);
	memset(data, 0, wav_hdr->subchunk2_size);
	result = read(fd, data, wav_hdr->subchunk2_size);
	if (result < 0) {
		log_msg(LOG_WARNING, "Read %s data failed (%s)",
			filename, strerror(fd));
		close(fd);
		*len = 0;
		return NULL;
	}
	close(fd);
	*len = wav_hdr->subchunk2_size;
	return data;
}

int
wav_play(sip_user_agent_t ua, char *filename)
{
	char *data, *cdata;
	short *wdata;
	int i, len = 0, result, state;

	if (ua == NULL || filename == NULL)
		return (-1);

	/*
	 * If there is a call connected, play the .wav file into the
	 * outbound RTP stream.  Load .wav file into memory.  If this
	 * function succeeds, the returned buffer needs to be freed
	 */
	data = wav_load((const char *) filename, &len);
	if (data == NULL) {
		log_msg(LOG_WARNING, "Load %s failed", filename);
		return (-1);
	}
	log_msg(LOG_INFO, "Play %s length %d bytes", filename, len);

	/*
	 * XXX Assume the .wav file is 8-bit sampled at 8 KHz.  Convert
	 * from unsigned 8-bit to signed 16-bit linear.
	 */
	wdata = malloc(2 * len);
	if (wdata == NULL) {
		free(data);
		return (-1);
	}
	for (i = 0; i < len; i++)
		wdata[i] = (short) ((data[i] - 0x0080) << 8);

	state = sip_dialog_get_state(ua->dialog);
	if (state == SIPS_IDLE) {
		/*
		 * If there is no call connected, play the .wav file
		 * out to the soundcard
		 */

		soundcard_write(&(ua->soundcard), (char *) wdata, 2 * len);

		free(wdata);
		free(data);
		return 0;
	}
	/* Convert signed 16-bit linear samples */
	cdata = malloc(len);
	if (cdata == NULL) {
		free(wdata);
		free(data);
		return (-1);
	}
	result = codec_encode(ua->rtp.codec, wdata, len, cdata, &len);
	if (result < 0) {
		free(wdata);
		free(data);
		return (-1);
	}

	wav_start(ua, cdata, len, SOUNDCARD_SAMPLE_SIZE);

	free(wdata);
	free(data);
	return 0;
}

void
wav_start(sip_user_agent_t ua, char *cdata, int len, int frame_size)
{
	wav_rec_t new_rec, rec;
	struct timezone tz;

	if (ua == NULL || cdata == NULL)
		return;

	new_rec = malloc(sizeof(struct wav_rec));
	if (new_rec == NULL)
		return;
	memset(new_rec, 0, sizeof(struct wav_rec));
	new_rec->cdata = cdata;
	new_rec->len = len;
	new_rec->frame_size = frame_size;
	new_rec->pos = 0;

	pthread_mutex_lock(&(ua->wavs.mutex));

	if (ua->wavs.wav_recs == NULL)
		ua->wavs.wav_recs = new_rec;

	else {
		for (rec = ua->wavs.wav_recs;
		     rec->next != NULL;
		     rec = rec->next);
		rec->next = new_rec;
	}
	pthread_mutex_unlock(&(ua->wavs.mutex));

	if (!timerisset(&(ua->wav_end)))
		gettimeofday(&(ua->wav_end), &tz);
}

void
wav_send(sip_user_agent_t ua)
{
	struct timeval dur, now;
	struct timezone tz;
	int len;

	if (ua == NULL)
		return;

	/*
	 * Write a chunk of the .wav file to the RTP stream every
	 * 20 milliseconds
	 */
	gettimeofday(&now, &tz);
	if (timercmp(&now, &(ua->wav_end), <))
		return;

	pthread_mutex_lock(&(ua->wavs.mutex));

	/*
	 * Send at most frame_size bytes of data from the .wav
	 * file to the soundcard
	 */
	len = ua->wavs.wav_recs->len - ua->wavs.wav_recs->pos;
	if (len > ua->wavs.wav_recs->frame_size)
		len = ua->wavs.wav_recs->frame_size;

	/* Generate and send an RTP packet */
	rtp_send(&(ua->rtp),
		 ua->wavs.wav_recs->cdata + ua->wavs.wav_recs->pos, len,
		 ua->rtp.codec);

	/* Mark .wav data consumed */
	ua->wavs.wav_recs->pos += len;
	if (ua->wavs.wav_recs->pos >= ua->wavs.wav_recs->len) {
		wav_rec_t rec;

		/* Free up .wav file since we've played all of it */
		rec = ua->wavs.wav_recs;
		free(rec->cdata);
		ua->wavs.wav_recs = rec->next;
		free(rec);
	}
	pthread_mutex_unlock(&(ua->wavs.mutex));

	if (ua->wavs.wav_recs == NULL) {
		timerclear(&(ua->wav_end));
		return;
	}
	dur.tv_sec = 0;
	dur.tv_usec = 20000;
	timeradd(&(ua->wav_end), &dur, &(ua->wav_end));
}

void
wav_rec_list_init(wav_rec_list_t wavs)
{
	if (wavs == NULL)
		return;

	wavs->wav_recs = NULL;
	pthread_mutex_init(&(wavs->mutex), NULL);
}

static int
wav_rec_list_clear(wav_rec_list_t wavs)
{
	wav_rec_t rec;
	int i;

	if (wavs == NULL)
		return (-1);

	for (rec = wavs->wav_recs, i = 0;
	     rec != NULL;
	     rec = wavs->wav_recs, i++) {
		wavs->wav_recs = rec->next;

		wav_dump_rec(rec, i);

		free(rec->cdata);
		free(rec);
	}
	return i;
}

void
wav_rec_list_flush(sip_user_agent_t ua)
{
	int n;

	if (ua == NULL)
		return;

	pthread_mutex_lock(&(ua->wavs.mutex));
	n = wav_rec_list_clear(&(ua->wavs));
	pthread_mutex_unlock(&(ua->wavs.mutex));
	if (n > 0)
		timerclear(&(ua->wav_end));
}
