#include <stdio.h>
#include <string.h>
#include "log.h"
#include "sip.h"

#define MAX_DIALOGS	128

static struct sip_dialog sip_dialog_pool[MAX_DIALOGS];
static sip_dialog_t sip_dialog_stack = NULL;

int
sip_dialog_get_state(sip_dialog_t dialog)
{
	if (dialog == NULL)
		return (-1);

	return dialog->state;
}

void
sip_dialog_set_state(sip_dialog_t dialog, int state)
{
	if (dialog == NULL)
		return;

	if (state < SIPS_IDLE || state > SIPS_CONNECTED)
		return;

	dialog->state = state;
}

void
sip_dialog_stack_init()
{
	int i;

	for (i = 0; i < MAX_DIALOGS; i++)
		sip_dialog_push(&(sip_dialog_pool[i]));
}

void
sip_dialog_push(sip_dialog_t dialog)
{
	if (dialog == NULL)
		return;

	sip_dialog_init(dialog);
	dialog->next = sip_dialog_stack;
	sip_dialog_stack = dialog;
}

sip_dialog_t
sip_dialog_pop()
{
	sip_dialog_t dialog = sip_dialog_stack;

	if (sip_dialog_stack != NULL) {
		sip_dialog_stack = sip_dialog_stack->next;
		dialog->next = NULL;
	}
	return dialog;
}

void
sip_dialog_init(sip_dialog_t dialog)
{
	if (dialog == NULL)
		return;

	dialog->prev = NULL;
	dialog->next = NULL;
	dialog->state = SIPS_IDLE;
	sip_dialog_rtt_init(dialog);
	sip_timer_init(dialog);
	sip_route_set_init(&dialog->route_set);
	sip_free_via_hdrs(&(dialog->via_hdrs));
	memset(dialog->call_id, 0, BUFSIZE);
	memset(dialog->local_tag, 0, BUFSIZE);
	memset(dialog->remote_tag, 0, BUFSIZE);
	dialog->local_seq = (-1);
	dialog->remote_seq = (-1);
	sip_uri_init(&(dialog->local_uri));
	sip_uri_init(&(dialog->remote_uri));
	sip_uri_init(&(dialog->reg_uri));
	sip_uri_init(&(dialog->remote_target));
	memset(dialog->last_resp, 0, BUFSIZE);
	dialog->auth_type = SIPA_NULL;
	memset(dialog->auth_nonce, 0, BUFSIZE);
	memset(dialog->auth_realm, 0, BUFSIZE);
	memset(dialog->auth_user, 0, BUFSIZE);
	memset(dialog->auth_response, 0, BUFSIZE);
	memset(dialog->authorization, 0, BUFSIZE);
}

sip_dialog_t
sip_dialog_list_find_remote(sip_uri_t uri, sip_dialog_t list)
{
	sip_dialog_t dialog;

	for (dialog = list; dialog != NULL; dialog = dialog->next)
		if (sip_uri_cmp(&(dialog->remote_uri), uri) == 0)
			break;

	return dialog;
}

void
sip_dialog_list_insert_head(sip_dialog_t dialog, sip_dialog_t *list)
{
	if (dialog == NULL || list == NULL)
		return;

	dialog->prev = NULL;
	if (*list == NULL) {
		dialog->next = NULL;
		*list = dialog;
		return;
	}
	dialog->next = *list;
	(*list)->prev = dialog;
	*list = dialog;
}

void
sip_dialog_list_insert_tail(sip_dialog_t dialog, sip_dialog_t *list)
{
	sip_dialog_t tail;

	if (dialog == NULL || list == NULL)
		return;

	if (*list == NULL) {
		dialog->prev = NULL;
		dialog->next = NULL;
		*list = dialog;
		return;
	}
	for (tail = *list;; tail = tail->next)
		if (tail->next == NULL) {
			dialog->prev = tail;
			dialog->next = NULL;
			tail->next = dialog;
			break;
		}
}

void
sip_dialog_list_remove(sip_dialog_t dialog, sip_dialog_t *list)
{
	if (dialog == NULL || list == NULL)
		return;

	if (dialog->prev == NULL)
		*list = dialog->next;
	else
		dialog->prev->next = dialog->next;

	if (dialog->next != NULL)
		dialog->next->prev = dialog->prev;

	dialog->prev = NULL;
	dialog->next = NULL;
}

void
sip_dialog_rtt_init(sip_dialog_t dialog)
{
	if (dialog == NULL)
		return;

	timerclear(&(dialog->rtt));
	timerclear(&(dialog->rtt_start));
}

void
sip_dialog_rtt_start(sip_dialog_t dialog)
{
	struct timezone tz;
	
	if (dialog == NULL)
		return;
		
		

	timerclear(&(dialog->rtt));
	gettimeofday(&(dialog->rtt_start), &tz);
}

void
sip_dialog_rtt_stop(sip_dialog_t dialog)
{
	struct timeval now;
	struct timezone tz;
	int sec, mil;
	char s[128];

	if (dialog == NULL)
		return;

	gettimeofday(&now, &tz);
	timersub(&now, &(dialog->rtt_start), &(dialog->rtt));

	sec = dialog->rtt.tv_sec;
	mil = dialog->rtt.tv_usec / 1000;

	memset(s, 0, 128);
	sprintf(s, "RTT estimate");

	if (sec > 1)
		sprintf(s + strlen(s), " %d seconds", sec);
	else if (sec == 1)
		sprintf(s + strlen(s), " 1 second");

	if (mil > 1)
		sprintf(s + strlen(s), " %d milliseconds", mil);
	else if (mil == 1)
		sprintf(s + strlen(s), " 1 millisecond");

	log_msg(LOG_INFO, s);
}
