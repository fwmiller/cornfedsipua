#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "log.h"
#include "sip.h"
#include "soundcard.h"

static void
ringtone_file_clear(ringtone_t ringtone)
{
	if (ringtone == NULL)
		return;

	ringtone->no_ringtone = 1;
	memset(ringtone->file, 0, BUFSIZE);
	if (ringtone->wdata != NULL)
		free(ringtone->wdata);
	ringtone->wdata = NULL;
	ringtone->wdata_len = 0;
}

int
ringtone_file_init(ringtone_t ringtone, const char *ringtone_file)
{
	char *data;
	int i, len;

	if (ringtone == NULL || ringtone_file == NULL)
		return (-1);

	ringtone_file_clear(ringtone);

	/* Load ringtone .wav file */
	data = wav_load(ringtone_file, &len);
	if (data == NULL)
		return (-1);
	strncpy(ringtone->file, ringtone_file, BUFSIZE - 1);
	ringtone->wdata_len = len;

	ringtone->wdata = malloc(2 * ringtone->wdata_len);
	if (ringtone->wdata == NULL) {
		ringtone_file_clear(ringtone);
		free(data);
		return (-1);
	}
	for (i = 0; i < ringtone->wdata_len; i++)
		ringtone->wdata[i] = (short) ((data[i] - 0x0080) << 8);
	free(data);

	if (ringtone->fd > 2)
		ringtone->no_ringtone = 0;
	else
		ringtone->no_ringtone = 1;

	return 0;
}

static void
ringtone_device_clear(sip_user_agent_t ua)
{
	ringtone_t ringtone = &(ua->ringtone);
	soundcard_t soundcard = &(ua->soundcard);

	if (ua == NULL)
		return;

	ringtone->no_ringtone = 1;
	memset(ringtone->device, 0, BUFSIZE);

	if (ringtone->fd > 2)
		close(ringtone->fd);

	ringtone->fd = (-1);
	if (strcmp(ringtone->device, soundcard->device) == 0)
		soundcard->fd = (-1);
}

int
ringtone_device_init(sip_user_agent_t ua, char *ringtone_device)
{
	ringtone_t ringtone = &(ua->ringtone);
	soundcard_t soundcard = &(ua->soundcard);

	if (ua == NULL || ringtone_device == NULL)
		return (-1);

	ringtone_device_clear(ua);
	strcpy(ringtone->device, ringtone_device);

	if (strcmp(ringtone->device, "/dev/null") == 0)
		return 0;

	if (strcmp(ringtone->device, soundcard->device) == 0) {
		/*
		 * The new ringtone device is the same as the existing
		 * soundcard device.  Open the device only if its not
		 * already opened for the soundcard.
		 */
		if (soundcard->fd < 0) {
			ringtone->fd = open(ringtone->device,
					    O_RDWR | O_NONBLOCK);
			if (ringtone->fd < 0) {
				log_msg(LOG_INFO,
					"Open ringtone device %s failed (%s)",
					ringtone->device, strerror(errno));
				return (-1);
			}
			if (soundcard_setup(ringtone->fd) < 0) {
				close(ringtone->fd);
				ringtone->fd = (-1);
				return (-1);
			}
			soundcard->fd = ringtone->fd;
		} else
			ringtone->fd = soundcard->fd;

	} else {
		/*
		 * The new ringtone device is different as the existing
		 * soundcard device
		 */
		ringtone->fd = open(ringtone->device, O_RDWR | O_NONBLOCK);
		if (ringtone->fd < 0) {
			log_msg(LOG_INFO,
				"Open ringtone device %s failed (%s)",
				ringtone->device, strerror(errno));
			return (-1);
		}
		if (soundcard_setup(ringtone->fd) < 0) {
			close(ringtone->fd);
			ringtone->fd = (-1);
			return (-1);
		}
	}
	if (ringtone->wdata != NULL)
		ringtone->no_ringtone = 0;
	else
		ringtone->no_ringtone = 1;

	return 0;
}

static void
ringtone_write(sip_user_agent_t ua, char *buf, int len)
{
	ringtone_t ringtone;
	struct audio_buf_info info;
	int pos, slen, result;

	if (ua == NULL || buf == NULL)
		return;

	ringtone = &(ua->ringtone);

	if (ringtone->no_ringtone || ringtone->fd < 0)
		return;
	
	for (pos = 0; pos < len;) {
		usleep(100);
	
		result = ioctl(ringtone->fd, SNDCTL_DSP_GETOSPACE, &info);
		if (info.bytes <= 0)
			continue;
	
		slen = info.bytes;
		if (slen > len - pos)
			slen = len - pos;
	
		write(ringtone->fd, buf + pos, slen);
		pos += slen;
	}
}

void
ringtone_play(sip_user_agent_t ua)
{
	if (ua == NULL)
		return;

	ringtone_write(ua,
		       (char *) (ua->ringtone.wdata),
		       2 * ua->ringtone.wdata_len);
}
