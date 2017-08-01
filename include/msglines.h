#ifndef __LINES_H
#define __LINES_H

#define MAX_MSG_LINES	128

struct msglines {
	char *msgbuf;
	int lines;
	int body;
	int pos[MAX_MSG_LINES];
	int len[MAX_MSG_LINES];
};

typedef struct msglines *msglines_t;

#if _DEBUG
void dump_msglines(msglines_t msglines);
#endif
int get_msglines(char *msgbuf, int len, msglines_t msglines);
char *msglines_find_hdr(msglines_t msglines, char *hdr);
int msglines_get_method(msglines_t msglines);

#endif
