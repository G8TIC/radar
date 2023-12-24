/*
 * sha512.h -- Header for sha512c derived from:
 *
 *	https://opensource.apple.com/source/network_cmds/network_cmds-511/unbound/compat/sha512.c.auto.html
 */

#ifndef _sha512_H
#define _sha512_H

#include <stdint.h>			/* for int types */
#include <sys/types.h>			/* for endian definitions */

//#undef BYTE_ORDER
//#define BYTE_ORDER LITTLE_ENDIAN

#define SHA512_BLOCK_LENGTH		128
#define SHA512_DIGEST_LENGTH		64
#define SHA512_DIGEST_STRING_LENGTH	(SHA512_DIGEST_LENGTH * 2 + 1)

typedef struct _sha512_ctx {
	uint64_t state[8];
	uint64_t bitcount[2];
	uint8_t	buffer[SHA512_BLOCK_LENGTH];
} sha512_ctx;

void sha512_init(sha512_ctx*);
void sha512_update(sha512_ctx*, void*, size_t);
void sha512_final(uint8_t[SHA512_DIGEST_LENGTH], sha512_ctx*);
void sha512(uint8_t *out, void *data, unsigned int data_len);

#endif
