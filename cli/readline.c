#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cornfedsipua.h"
#include "log.h"

char *
readline(char *prompt)
{
	char *buf;
	int c, pos;

	buf = malloc(BUFSIZE);
	if (buf == NULL) {
		log_msg(LOG_ERROR, "No memory for command line buffer");
		return NULL;
	}
	memset(buf, 0, BUFSIZE);

	printf(prompt);
	for (pos = 0; pos < BUFSIZE - 1; pos++) {
		c = getchar();
		if (c == '\n')
			break;
		buf[pos] = c;
	}
	return buf;
}
