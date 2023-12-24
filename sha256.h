/*
 * sha256.h -- a header file for SHA256 functions
 */

#ifndef _SHA256_H
#define _SHA256_H

#define SHA256_SIZE 32

#include <stdint.h>

/*
 * SHA256 context
 */
typedef struct {
  uint8_t hash[SHA256_SIZE];	// Changed by RKW, unsigned char becomes uint8_t
  uint32_t buffer[16];		// Changed by RKW, unsigned long becomes uint32_t
  uint32_t state[8];		// Changed by RKW, unsinged long becomes uint32_t
  uint8_t length[8];		// Changed by RKW, unsigned char becomes uint8_t
} sha256_t;


void sha256_initialize(sha256_t *sha);
void sha256_update(sha256_t *sha, const uint8_t *message, int length);
void sha256_finalize(sha256_t *sha, const uint8_t *message, int length);
void sha256(uint8_t hash[SHA256_SIZE], const uint8_t *message, int length);
int  sha256_compare(uint8_t *, uint8_t *);


#endif
