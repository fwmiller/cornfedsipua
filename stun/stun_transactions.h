#ifndef __STUN_TRANSACTIONS
#define __STUN_TRANSACTIONS

#include "cornfedsipua.h"

#define STUN_TID_SIZE		12

#define STUN_PROTOCOL_NULL	0
#define STUN_PROTOCOL_SIP	1
#define STUN_PROTOCOL_RTP	2

struct stun_transaction {
	struct stun_transaction *next;
	int protocol;
	u8_t tid[STUN_TID_SIZE];
};

typedef struct stun_transaction *stun_transaction_t;

void stun_transaction_init(stun_transaction_t transaction);
void stun_transaction_push(stun_transaction_t transaction);
stun_transaction_t stun_transaction_pop(void);
void stun_transactions_init(void);
void stun_transaction_list_insert(stun_transaction_t transaction);
void stun_transaction_list_remove(stun_transaction_t transaction);
stun_transaction_t stun_transaction_find(char *tid);

#endif
