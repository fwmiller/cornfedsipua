#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "history.h"
#include "log.h"

static char cornfeddir[BUFSIZE];
static char historyfile[BUFSIZE];
static char backuphistoryfile[BUFSIZE];
static history_event_t history_event_list = NULL;

int
history_init()
{
	char *homedir;

	memset(cornfeddir, 0, BUFSIZE);
	memset(historyfile, 0, BUFSIZE);
	memset(backuphistoryfile, 0, BUFSIZE);

	homedir = getenv("HOME");
	if (homedir == NULL) {
		log_msg(LOG_ERROR, "Could not get home directory");
		return (-1);
	}
	strcpy(cornfeddir, homedir);
	strcat(cornfeddir, CORNFED_DIR);
	log_msg(LOG_INFO, "Cornfed directory [%s]", cornfeddir);

	strcpy(historyfile, cornfeddir);
	strcat(historyfile, HISTORY_FILE);
	log_msg(LOG_INFO, "History file [%s]", historyfile);

	strcpy(backuphistoryfile, historyfile);
	strcat(backuphistoryfile, "~");
	log_msg(LOG_INFO, "Backup history file [%s]", backuphistoryfile);

	return (-1);
}

void
history_dump_event(history_event_t e, char *s)
{
	strncpy(s, ctime(&(e->time)) + 4, 15);

	strcat(s, " ");

	switch (e->event) {
	case HIST_EVENT_INITIATED:
		strcat(s, "Initiated");
		break;

	case HIST_EVENT_CANCELED:
		strcat(s, "Canceled");
		break;

	case HIST_EVENT_RECEIVED:
		strcat(s, "Received");
		break;

	case HIST_EVENT_CONNECTED:
		strcat(s, "Connected");
		break;

	case HIST_EVENT_HANGUP:
		strcat(s, "Hangup");
		break;

	default:
		strcat(s, "ILLEGAL HISTORY EVENT");
	}
	sprintf(s + strlen(s), " [%s]", e->endpoint);
}

void
history_dump()
{
	history_event_t event;
	char s[128];

	log_msg(LOG_INFO, "Call history:");

	for (event = history_event_list; event != NULL; event = event->next) {
		memset(s, 0, 128);
		history_dump_event(event, s);
		log_msg(LOG_INFO, s);
	}
}

history_event_t
history_new_event(time_t time, int event, char *endpoint)
{
	history_event_t e;

	e = malloc(sizeof(struct history_event));
	if (e == NULL)
		return NULL;

	memset(e, 0, sizeof(struct history_event));
	e->time = time;
	e->event = event;
	strncpy(e->endpoint, endpoint, HIST_ENDPOINT_SIZE - 1);
	return e;
}

#define HISTORY_FILE_LINE(LINE)						\
{									\
	char s[128];							\
									\
	memset(s, 0, 128);						\
	sprintf(s, LINE);						\
	result = write(fd, s, strlen(s));				\
	if (result < 0) {						\
		log_msg(LOG_ERROR, "Could not write to history file");	\
		return (-1);						\
	}								\
}

static int
history_create_history_file()
{
	int fd, result;

	fd = open(historyfile, O_WRONLY | O_CREAT, 0600);
	if (fd < 0) {
		log_msg(LOG_ERROR, "Could not create history file [%s]",
		historyfile);
		return (-1);
	}
	HISTORY_FILE_LINE("#\n");
	HISTORY_FILE_LINE("# Do not edit -- file generated automagically\n");
	HISTORY_FILE_LINE("#\n");

	close(fd);
	return 0;
}

static int
history_file_read_line(int fd, char *line)
{
	int len, nread;
	char ch;

	for (len = 0;;) {
		nread = read(fd, &ch, 1);
		if (nread < 0)
			return (-1);
		if (nread == 0 || ch == '\n')
			break;
		line[len++] = ch;
	}
	return len;
}

#define NEXTARG(ARG)							\
	for (pos = 0;;) {						\
		ch = line[lpos++];					\
		if (ch == ',' || ch == '\0')				\
			break;						\
		ARG[pos++] = ch;					\
	}

static void
history_parse_line(char *line, time_t *time, int *event, char *endpoint)
{
	char timestr[32], eventstr[16];
	int lpos = 0, pos;
	int ch;

	memset(timestr, 0, 32);
	NEXTARG(timestr);
	*time = (time_t) strtoul(timestr, NULL, 10);

	memset(eventstr, 0, 16);
	NEXTARG(eventstr);
	*event = atoi(eventstr);

	NEXTARG(endpoint);
}

static void
history_insert_event(history_event_t e)
{
	history_event_t c;

	if (history_event_list == NULL) {
		e->next = NULL;
		history_event_list = e;
		return;
	}
	if (e->time > history_event_list->time) {
		e->next = history_event_list;
		history_event_list = e;
		return;
	}
	e->next = NULL;
	for (c = history_event_list;; c = c->next) {
		if (c->next == NULL) {
			c->next = e;
			return;
		}
		if (e->time > c->next->time) {
			e->next = c->next;
			c->next = e;
			return;
		}
	}
}

int
history_read_file()
{
	struct stat buf;
	char line[128];
	int fd, result;

	memset(&buf, 0, sizeof(struct stat));
	result = stat(historyfile, &buf);
	if (result < 0) {
		memset(&buf, 0, sizeof(struct stat));
		result = stat(cornfeddir, &buf);
		if (result < 0) {
			result = mkdir(cornfeddir, 0777);
			if (result < 0) {
				log_msg(LOG_ERROR,
					"Create cornfed directory failed");
				return (-1);
			}
		}
		result = history_create_history_file();
		if (result < 0)
			return (-1);
	}
	fd = open(historyfile, O_RDONLY);
	if (fd < 0) {
		log_msg(LOG_ERROR, "Open history file failed");

		fd = open(backuphistoryfile, O_RDONLY);
		if (fd < 0) {
			log_msg(LOG_ERROR,
				"Open backup history file failed");
			return (-1);
		}
	}
#if 0
	log_msg(LOG_INFO, "History file contents:");
#endif
	for (;;) {
		history_event_t e;
		time_t time = 0;
		char endpoint[HIST_ENDPOINT_SIZE];
		int event = 0;

		memset(line, 0, 128);
		result = history_file_read_line(fd, line);
		if (result < 0) {
			log_msg(LOG_ERROR, "Read history file failed");
			return (-1);
		}
		if (result == 0)
			break;

		/* Skip comment lines */
		if (line[0] == '#')
			continue;

		memset(endpoint, 0, HIST_ENDPOINT_SIZE);
		history_parse_line(line, &time, &event, endpoint);
#if 0
		log_msg(LOG_INFO, "time %u event %d endpoint [%s]",
			time, event, endpoint);
#endif
		e = history_new_event(time, event, endpoint);
		if (e != NULL)
			history_insert_event(e);
	}
	return (-1);
}

int
history_write_file()
{
	history_event_t e;
	int fd, result;

	result = rename(historyfile, backuphistoryfile);
	if (result < 0)
		log_msg(LOG_ERROR, "Rename history file failed");

	fd = open(historyfile, O_WRONLY | O_CREAT, 0600);
	if (fd < 0) {
		log_msg(LOG_ERROR, "Could not create history file [%s]",
			historyfile);
		return (-1);
	}
	HISTORY_FILE_LINE("#\n");
	HISTORY_FILE_LINE("# Do not edit -- file generated automagically\n");
	HISTORY_FILE_LINE("#\n");

	for (e = history_event_list; e != NULL; e = e->next) {
		char line[128];

		memset(line, 0, 128);
		sprintf(line, "%u,%d,%s\n",
			(unsigned int) e->time, e->event, e->endpoint);
		HISTORY_FILE_LINE(line);
	}
	close(fd);
	return 0;
}

void
history_add_event(sip_user_agent_t ua, int event, char *endpoint)
{
	history_event_t e;

	e = history_new_event(time(NULL), event, endpoint);
	if (e != NULL) {
		char s[128];

		history_insert_event(e);

		memset(s, 0, 128);
		history_dump_event(e, s);
		if (ua->ua_history_update != NULL)
			(*(ua->ua_history_update)) (s);
	}
}

history_event_t
history_first_event()
{
	return history_event_list;
}

history_event_t
history_next_event(history_event_t e)
{
	return e->next;
}

void
history_clear(sip_user_agent_t ua)
{
	while (history_event_list != NULL) {
		history_event_t e = history_event_list;

		history_event_list = e->next;
		free(e);
	}
	if (ua->ua_history_clear != NULL)
		(*(ua->ua_history_clear)) ();
}
