#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "codec.h"
#include "log.h"
#include "sip.h"
#include "soundcard.h"

/* 8 256-byte fragments */
#define SOUNDCARD_BUFFER_FORMAT 0x00080008

static void
soundcard_clear(sip_user_agent_t ua)
{
	soundcard_t soundcard = &(ua->soundcard);
	ringtone_t ringtone = &(ua->ringtone);

	if (ua == NULL)
		return;

	soundcard->no_soundcard = 1;
	memset(soundcard->device, 0, BUFSIZE);

	if (soundcard->fd > 2)
		close(soundcard->fd);
	soundcard->fd = (-1);
	if (strcmp(soundcard->device, ringtone->device) == 0)
		ringtone->fd = (-1);

	soundcard->frame_size = 2 * SOUNDCARD_SAMPLE_SIZE;
	soundcard->ibufsize = 0;
	if (soundcard->ibuf != NULL) {
		log_msg(LOG_INFO, "Free soundcard input buffer");

		free(soundcard->ibuf);
		soundcard->ibuf = NULL;
	}
	soundcard->icnt = 0;
	soundcard->ihead = 0;
	soundcard->itail = 0;
}

int
soundcard_setup(int fd)
{
	int fmt, result;

	if (fd < 0)
		return (-1);

	/* Set full-duplex operation */
	result = ioctl(fd, SNDCTL_DSP_SETDUPLEX, 0);
	if (result < 0) {
		log_msg(LOG_ERROR,
			"Set full-duplex soundcard operation failed (%s)",
			strerror(errno));
		return (-1);
	}
	/* Set buffer format */
	fmt = SOUNDCARD_BUFFER_FORMAT;
	result = ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &fmt);
	if (result < 0) {
		log_msg(LOG_ERROR,
			"Set soundcard buffer format failed (%s)",
			strerror(errno));
		return (-1);
	}
	/* Set for mono operation */
	fmt = 0;
	result = ioctl(fd, SNDCTL_DSP_STEREO, &fmt);
	if (result < 0) {
		log_msg(LOG_ERROR,
			"Set soundcard mono operation failed (%s)",
			strerror(errno));
		return (-1);
	}
	/* Set signed 16-bit little-endian sample format */
	fmt = AFMT_S16_LE;
	result = ioctl(fd, SNDCTL_DSP_SETFMT, &fmt);
	if (result < 0) {
		log_msg(LOG_ERROR, "Set soundcard format failed (%s)",
			strerror(errno));
		return (-1);
	}
	/* Set sampling rate */
	fmt = SOUNDCARD_SAMPLING_RATE;
	result = ioctl(fd, SNDCTL_DSP_SPEED, &fmt);
	if (result < 0 || fmt != SOUNDCARD_SAMPLING_RATE) {
		if (result < 0)
			log_msg(LOG_ERROR,
				"Set %d Hz soundcard sampling failed (%s)",
				SOUNDCARD_SAMPLING_RATE, strerror(errno));
		else {
			char s[128];

			memset(s, 0, 128);
			sprintf(s, "Set %d Hz soundcard sampling failed ",
				SOUNDCARD_SAMPLING_RATE);
			sprintf(s + strlen(s), "(Sampling rate is %d)", fmt);
			log_msg(LOG_ERROR, s);
		}
		return (-1);
	}
	return 0;
}

int
soundcard_init(sip_user_agent_t ua, char *soundcard_device)
{
	soundcard_t soundcard = &(ua->soundcard);
	ringtone_t ringtone = &(ua->ringtone);
	struct audio_buf_info info;
	char current_soundcard[BUFSIZE];
	int result;

	if (ua == NULL || soundcard_device == NULL)
		return (-1);

	memset(current_soundcard, 0, BUFSIZE);
	strcpy(current_soundcard, soundcard->device);

	soundcard_clear(ua);

	if (soundcard_device == NULL)
		strcpy(soundcard->device, current_soundcard);
	else
		strcpy(soundcard->device, soundcard_device);

	log_msg(LOG_INFO, "Initialize soundcard [%s]", soundcard->device);

	if (strcmp(soundcard->device, "/dev/null") == 0)
		return 0;

	if (strcmp(soundcard->device, ringtone->device) == 0) {
		/*
		 * The new soundcard device is the same as the existing
		 * ringtone device.  Open the device only if its not
		 * already opened for the ringtone.
		 */
		if (ringtone->fd < 0) {
			soundcard->fd = open(soundcard->device,
					     O_RDWR | O_NONBLOCK);
			if (soundcard->fd < 0) {
				log_msg(LOG_INFO,
					"Open soundcard device %s failed (%s)",
					soundcard->device, strerror(errno));
				return (-1);
			}
			if (soundcard_setup(soundcard->fd) < 0) {
				close(soundcard->fd);
				soundcard->fd = (-1);
				return (-1);
			}
			ringtone->fd = soundcard->fd;
		} else
			soundcard->fd = ringtone->fd;

	} else {
		/*
		 * The new soundcard device is different as the existing
		 * ringtone device
		 */
		soundcard->fd = open(soundcard->device, O_RDWR | O_NONBLOCK);
		if (soundcard->fd < 0) {
			log_msg(LOG_INFO,
				"Open soundcard device %s failed (%s)",
				soundcard->device, strerror(errno));
			return (-1);
		}
		if (soundcard_setup(soundcard->fd) < 0) {
			close(soundcard->fd);
			soundcard->fd = (-1);
			return (-1);
		}
	}
	result = ioctl(soundcard->fd, SNDCTL_DSP_GETISPACE, &info);
	if (result < 0)
		return (-1);

	soundcard->ibufsize = info.fragstotal * info.fragsize;
	log_msg(LOG_INFO, "Soundcard input buffer size %d bytes",
		soundcard->ibufsize);

	soundcard->ibuf = malloc(soundcard->ibufsize);
	if (soundcard->ibuf != NULL)
		soundcard->no_soundcard = 0;
	else
		log_msg(LOG_ERROR, "Allocate soundcard input buffer failed");

	return 0;
}

static void
soundcard_read_input(soundcard_t soundcard)
{
	fd_set rfds;
	struct timeval timeout;
	struct audio_buf_info info;
	int avail, len, remaining, result, size;

	if (soundcard == NULL)
		return;

	FD_ZERO(&rfds);
	FD_SET(soundcard->fd, &rfds);
	timerclear(&timeout);
	result = select(soundcard->fd + 1, &rfds, NULL, NULL, &timeout);
	if (result < 0 || !FD_ISSET(soundcard->fd, &rfds))
		return;

	result = ioctl(soundcard->fd, SNDCTL_DSP_GETISPACE, &info);
	if (result < 0)
		return;

	/* Bytes available to be read (multiple of fragment size) */
	avail = info.fragments * info.fragsize;

	/* Bytes remaining in input buffer */
	remaining = soundcard->ibufsize - soundcard->icnt;

	/*
	 * Figure out how many fragments will fit in the remaining
	 * input buffer space
	 */
	for (size = avail;;) {
		if (size <= remaining)
			break;

		size -= info.fragsize;
		if (size <= 0)
			return;
	}
	/* Read samples into input buffer */
	if (soundcard->ihead + size <= soundcard->ibufsize) {
		/* No input buffer wraparound */
		len = read(soundcard->fd,
			   soundcard->ibuf + soundcard->ihead, size);
		if (len < 0)
			return;

		soundcard->icnt += size;
		soundcard->ihead =
			(soundcard->ihead + size) % soundcard->ibufsize;

	} else {
		/* Input buffer wraparound */
		len = read(soundcard->fd,
			   soundcard->ibuf + soundcard->ihead,
			   soundcard->ibufsize - soundcard->ihead);
		if (len < 0)
			return;

		len = read(soundcard->fd, soundcard->ibuf,
			   size - (soundcard->ibufsize - soundcard->ihead));
		if (len < 0)
			return;

		soundcard->icnt += size;
		soundcard->ihead =
			size - (soundcard->ibufsize - soundcard->ihead);
	}
}

static void
soundcard_read_frame(soundcard_t soundcard, char *buf)
{
	if (soundcard == NULL || buf == NULL)
		return;

	/* Assume buf is at lease soundcard->frame_size bytes */

	if (soundcard->itail + soundcard->frame_size <=
	    soundcard->ibufsize) {
		/* No input buffer wraparound */
		memcpy(buf, soundcard->ibuf + soundcard->itail,
		       soundcard->frame_size);

		soundcard->icnt -= soundcard->frame_size;
		soundcard->itail =
			(soundcard->itail + soundcard->frame_size) %
			soundcard->ibufsize;

	} else {
		/* Input buffer wraparound */
		memcpy(buf, soundcard->ibuf + soundcard->itail,
		       soundcard->ibufsize - soundcard->itail);
		memcpy(buf, soundcard->ibuf,
		       soundcard->frame_size -
		       (soundcard->ibufsize - soundcard->itail));

		soundcard->icnt -= soundcard->frame_size;
		soundcard->itail =
		       soundcard->frame_size -
		       (soundcard->ibufsize - soundcard->itail);
	}
}

int
soundcard_thread_func(sip_user_agent_t ua)
{
	soundcard_t soundcard = &(ua->soundcard);
	char buf[BUFSIZE], obuf[BUFSIZE];
	int len, result;

	if (ua == NULL)
		return (-1);

	/*
	 * This check is needed because this function may be called by the
	 * main SIP thread to make sure a .wav file gets played out even
	 * if there is no soundcard attached
	 */
	if (soundcard->no_soundcard || soundcard->fd < 0)
		return 0;

	/* Read soundcard microphone samples into the input buffer */
	soundcard_read_input(soundcard);

	while (soundcard->icnt >= soundcard->frame_size) {
		int state;

		/* Read a frame of sample data from the input buffer */
		soundcard_read_frame(soundcard, buf);

		/*
		 * Check whether outbound RTP packet flow is enabled and
		 * whether packet will be sent somewhere that makes sense
		 */
		state = sip_dialog_get_state(ua->dialog);
		if (state != SIPS_CONNECTED ||
		    strlen(ua->rtp.remote.host) == 0 ||
		    ua->rtp.remote.port < 0)
			continue;

		/*
		 * Check whether a .wav file has been queued up to be
		 * played
		 */
		if (ua->wavs.wav_recs != NULL)
			continue;

		/* Check whether a codec has been selected */
		if (ua->rtp.codec != RTP_PAYLOAD_G711_ULAW &&
		    ua->rtp.codec != RTP_PAYLOAD_G711_ALAW &&
		    ua->rtp.codec != RTP_PAYLOAD_G729)
			continue;

		len = BUFSIZE;
		result = codec_encode(ua->rtp.codec, (short *) buf,
				      SOUNDCARD_SAMPLE_SIZE, obuf, &len);
		if (result == 0)
			rtp_send(&(ua->rtp), obuf, len, ua->rtp.codec);
	}
	if (ua->wavs.wav_recs != NULL)
		wav_send(ua);

	return 0;
}

void
soundcard_write(soundcard_t soundcard, char *buf, int len)
{
	struct audio_buf_info info;
	int avail, pos, slen, result;

	if (soundcard == NULL || buf == NULL)
		return;

	if (soundcard->no_soundcard || soundcard->fd < 0)
		return;

	for (pos = 0; pos < len;) {
		result = ioctl(soundcard->fd, SNDCTL_DSP_GETOSPACE, &info);
		avail = info.fragments * info.fragsize;
		if (avail <= 0) {
			usleep(100);
			continue;
		}
		slen = avail;
		if (slen > len - pos)
			slen = len - pos;

		write(soundcard->fd, buf + pos, slen);
		pos += slen;
	}
}

int
soundcard_status(soundcard_t soundcard,
		 struct audio_buf_info *iinfo,
		 struct audio_buf_info *oinfo)
{
	int result;

	if (soundcard == NULL || iinfo == NULL || oinfo == NULL)
		return (-1);

	if (soundcard->no_soundcard || soundcard->fd < 0)
		return (-1);

	result = ioctl(soundcard->fd, SNDCTL_DSP_GETISPACE, iinfo);
	if (result < 0)
		return (-1);

	result = ioctl(soundcard->fd, SNDCTL_DSP_GETOSPACE, oinfo);
	if (result < 0)
		return (-1);

	return 0;
}

void
soundcard_flush(soundcard_t soundcard)
{
	fd_set rfds;
	struct timeval timeout;
	struct audio_buf_info info;
	char *buf;
	int avail, len, result;

	if (soundcard == NULL || soundcard->no_soundcard ||
	    soundcard->fd < 0)
		return;

	FD_ZERO(&rfds);
	FD_SET(soundcard->fd, &rfds);
	timerclear(&timeout);
	result = select(soundcard->fd + 1, &rfds, NULL, NULL, &timeout);
	if (result < 0 || !FD_ISSET(soundcard->fd, &rfds))
		return;

	result = ioctl(soundcard->fd, SNDCTL_DSP_GETISPACE, &info);
	if (result < 0)
		return;

	/* Bytes available to be read (multiple of fragment size) */
	avail = info.fragments * info.fragsize;

	buf = malloc(avail);
	if (buf == NULL)
		return;

	len = read(soundcard->fd, buf, avail);
	free(buf);
	log_msg(LOG_INFO, "Flushed %d bytes", len);
}
