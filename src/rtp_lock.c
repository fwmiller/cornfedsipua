#include <errno.h>
#include "sip.h"

void
rtp_lock(rtp_session_t rtp)
{
	if (rtp == NULL)
		return;

	pthread_mutex_lock(&(rtp->mutex));
}

int
rtp_trylock(rtp_session_t rtp)
{
	int result;

	if (rtp == NULL)
		return (-1);

	result = pthread_mutex_trylock(&(rtp->mutex));
	if (result == EBUSY)
		return (-1);
	return 0;
}

void
rtp_unlock(rtp_session_t rtp)
{
	if (rtp == NULL)
		return;

	pthread_mutex_unlock(&(rtp->mutex));
}
