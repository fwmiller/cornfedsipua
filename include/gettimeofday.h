int
gettimeofday (struct timeval *tp, void *tz)
{
	
	struct timespec ts;
	if (tp == NULL)
		return (-1);
	
	
	memset(ts,0,sizeof(struct timespec));
	
	clock_gettime(CLOCK_REALTIME,&ts);
	
	tp->tv_sec = ts->tv_sec;
	tp->tv_usec = ts-> tv_nsec / 1000;
	}