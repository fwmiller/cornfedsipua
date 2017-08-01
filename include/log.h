#ifndef __LOG_H
#define __LOG_H

#include <stdarg.h>

#define LOG_ERROR	0
#define LOG_WARNING	1
#define LOG_CONNECTION	2
#define LOG_EVENT	3
#define LOG_INFO	4

void log_msg(int level, const char *format, ...);
void log_feedback(char *text);
int log_get_level(void);
void log_set_level(int level);

#endif
