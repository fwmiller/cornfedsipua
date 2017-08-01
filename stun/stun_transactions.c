#include <stdlib.h>
#include <string.h>
#include "stun_transactions.h"

#define MAX_STUN_TRANSACTIONS	16

static struct stun_transaction stun_transaction_pool[MAX_STUN_TRANSACTIONS];
static stun_transaction_t stun_transaction_stack = NULL;
static stun_transaction_t stun_transaction_list = NULL;

void
stun_transaction_init(stun_transaction_t transaction)
{
	if (transaction == NULL)
		return;

	transaction->next = NULL;
	transaction->protocol = STUN_PROTOCOL_NULL;
	memset(transaction->tid, 0, STUN_TID_SIZE);
}

void
stun_transaction_push(stun_transaction_t transaction)
{
	if (transaction == NULL)
		return;

	stun_transaction_init(transaction);
	transaction->next = stun_transaction_stack;
	stun_transaction_stack = transaction;
}

stun_transaction_t
stun_transaction_pop()
{
	stun_transaction_t transaction = stun_transaction_stack;

	if (stun_transaction_stack != NULL) {
		stun_transaction_stack = stun_transaction_stack->next;
		transaction->next = NULL;
	}
	return transaction;
}

void
stun_transactions_init()
{
	int i;

	memset(stun_transaction_pool, 0,
	       MAX_STUN_TRANSACTIONS * sizeof(struct stun_transaction));

	for (i = 0; i < MAX_STUN_TRANSACTIONS; i++)
		stun_transaction_push(&(stun_transaction_pool[i]));
}

void
stun_transaction_list_insert(stun_transaction_t transaction)
{
	if (transaction == NULL)
		return;

	transaction->next = stun_transaction_list;
	stun_transaction_list = transaction;
}

void
stun_transaction_list_remove(stun_transaction_t transaction)
{
	stun_transaction_t tr;

	if (transaction == NULL)
		return;

	if (stun_transaction_list == NULL)
		return;

	if (transaction == stun_transaction_list) {
		stun_transaction_list = transaction->next;
		transaction->next = NULL;
		return;
	}
	for (tr = stun_transaction_list; tr->next != NULL; tr = tr->next)
		if (transaction == tr->next) {
			tr->next = transaction->next;
			transaction->next = NULL;
			return;
		}
}

stun_transaction_t
stun_transaction_find(char *tid)
{
	stun_transaction_t tr;

	if (tid == NULL)
		return NULL;

	for (tr = stun_transaction_list; tr != NULL; tr = tr->next)
		if (memcmp(tid, tr->tid, STUN_TID_SIZE) == 0)
			return tr;

	return NULL;
}
