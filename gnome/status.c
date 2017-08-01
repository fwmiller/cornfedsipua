#include "gui.h"
#include "log.h"

static int initialized = 0;
static pthread_t status_thread;
static int idle = 1;
static struct timeval last_msg;

static void
status_display(char *msg)
{
	gtk_statusbar_push(GTK_STATUSBAR(status_bar),
			   status_bar_context_id, msg);
}

static void *
status_thread_func(void *arg)
{
	struct timeval now, dur, end;
	struct timezone tz;

	dur.tv_sec = 10;
	dur.tv_usec = 0;
	for (;;) {
		if (idle) {
			usleep(500000);
			continue;
		}
		gettimeofday(&now, &tz);
		timeradd(&last_msg, &dur, &end);
		if (timercmp(&now, &end, >=)) {
			gdk_threads_enter();
			status_display("Cornfed SIP User Agent");
			gdk_threads_leave();
			idle = 1;
		}
	}
}

static void
status_init()
{
	int result;

	result = pthread_create(
		&status_thread, NULL, status_thread_func, NULL);
	if (result != 0) {
		log_msg(LOG_ERROR, "Create status thread failed (%s)",
			strerror(result));
	}
}

void
status(char *msg)
{
	if (!initialized) {
		status_init();
		initialized = 1;
	}
	if (idle) {
		struct timezone tz;

		gettimeofday(&last_msg, &tz);
		idle = 0;
	}
	gdk_threads_enter();
	status_display(msg);
	gdk_threads_leave();
}

void
status_callback(char *msg)
{
	if (!initialized) {
		status_init();
		initialized = 1;
	}
	if (idle) {
		struct timezone tz;

		gettimeofday(&last_msg, &tz);
		idle = 0;
	}
	status_display(msg);
}
