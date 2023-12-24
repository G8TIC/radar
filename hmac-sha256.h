/*
 * hmac-sha256.h -- a header file for HMAC-SHA256 and SHA256 functions
 */

#ifndef _HMAC_SHA256
#define _HMAC_SHA256

#include <stdint.h>
#include "sha256.h"

#define HMAC_SHA256_SIZE SHA256_SIZE	// HMAC is the same size as a SHA256

typedef struct _hmac_sha256 {
    uint8_t digest[SHA256_SIZE];	// Changed by RKW, unsigned char becomes uint_8
    uint8_t key[64];			// Changed by RKW, unsigned char becomes uint_8
    sha256_t sha;
} hmac_sha256_t;

void hmac_sha256_initialize(hmac_sha256_t *hmac, const uint8_t *key, int length);
void hmac_sha256_update(hmac_sha256_t *hmac, const uint8_t *message, int length);
void hmac_sha256_finalize(hmac_sha256_t *hmac, const uint8_t *message, int length);
void hmac_sha256_result(hmac_sha256_t *hmac, uint8_t digest[SHA256_SIZE]);
void hmac_sha256(uint8_t digest[HMAC_SHA256_SIZE], const uint8_t *message, int message_length, const uint8_t *key, int key_length);

#endif
