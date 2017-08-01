#ifndef __STUN_IMPL_H
#define __STUN_IMPL_H

#include "cornfedsipua.h"

/* STUN message header types */
#define STUN_TYPE_BINDING_REQUEST		0x0001
#define STUN_TYPE_BINDING_RESPONSE		0x0101
#define STUN_TYPE_BINDING_ERROR			0x0111
#define STUN_TYPE_SHARED_SECRET_REQUEST		0x0002
#define STUN_TYPE_SHARED_SECRET_RESPONSE	0x0102
#define STUN_TYPE_SHARED_SECRET_ERROR		0x0112

/* STUN attribute header types */
#define STUN_ATTR_MAPPED_ADDRESS		0x0001
#define STUN_ATTR_RESPONSE_ADDRESS		0x0002
#define STUN_ATTR_CHANGE_REQUEST		0x0003
#define STUN_ATTR_SOURCE_ADDRESS		0x0004
#define STUN_ATTR_CHANGED_ADDRESS		0x0005
#define STUN_ATTR_USERNAME			0x0006
#define STUN_ATTR_PASSWORD			0x0007
#define STUN_ATTR_MESSAGE_INTEGRITY		0x0008
#define STUN_ATTR_ERROR_CODE			0x0009
#define STUN_ATTR_UNKNOWN_ATTRIBUTES		0x000a
#define STUN_ATTR_REFLECTED_FROM		0x000b
#define STUN_ATTR_REALM				0x0014
#define STUN_ATTR_NONCE				0x0015
#define STUN_ATTR_XOR_MAPPED_ADDRESS		0x0020
#define STUN_ATTR_SERVER			0x8022
#define STUN_ATTR_FINGERPRINT			0x8023
#define STUN_ATTR_ALTERNATE_SERVER		0x8023
#define STUN_ATTR_REFRESH_INTERVAL		0x8024

/* STUN address families */
#define STUN_ADDRESS_FAMILY_IPV4		0x01
#define STUN_ADDRESS_FAMILY_IPV6		0x02

typedef struct stun_attr_hdr *stun_attr_hdr_t;
typedef struct stun_address *stun_address_t;
typedef struct stun_error_code *stun_error_code_t;

struct stun_attr_hdr {
	u16_t type;
	u16_t length;
} __attribute__((packed));

struct stun_address {
	u8_t pad;
	u8_t family;
	u16_t port;
	u32_t address;
} __attribute__((packed));

struct stun_error_code {
	/* Bit ordering is Intel x86 specific */
	struct {
		u32_t zero:21;
		u32_t error_class:3;
		u32_t error_number:8;
	} code;

	u8_t reason_phrase[1];
} __attribute__((packed));

#endif
