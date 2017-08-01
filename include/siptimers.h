#ifndef __SIP_TIME_H
#define __SIP_TIME_H

struct timeval {
	long tv_sec;	/* seconds */
	long tv_usec;	/* microseconds */
};

struct timezone {
	int tz_minuteswest;	/* minutes W of Greenwich */
	int tz_dsttime;		/* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);

#define timeradd(a, b, res)				\
	(res)->tv_sec = (a)->tv_sec + (b)->tv_sec;	\
	(res)->tv_usec = (a)->tv_usec + (b)->tv_usec;	\
	if ((res)->tv_usec >= 1000000) {		\
		(res)->tv_sec++;			\
		(res)->tv_usec -= 1000000;		\
	}

#define timersub(a, b, res)				\
	(res)->tv_sec = (a)->tv_sec - (b)->tv_sec;	\
	(res)->tv_usec = (a)->tv_usec - (b)->tv_usec;	\
	if ((res)->tv_usec < 0) {			\
		(res)->tv_sec--;			\
		(res)->tv_usec += 1000000;		\
	}

#define timerisset(a) ((a)->tv_sec || (a)->tv_usec)

#define timercmp(a, b, cmp)				\
	((a)->tv_sec cmp (b)->tv_sec ||			\
	 ((a)->tv_sec == (b)->tv_sec &&			\
	  (a)->tv_usec cmp (b)->tv_usec))

#define timerclear(a) ((a)->tv_sec = (a)->tv_usec = 0)


#endif
