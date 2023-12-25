/*
 * authtag.h -- Authentication tag signing of Radar messages
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "sha256.h"
#include "hmac-sha256.h"
#include "sha512.h"
#include "hex.h"
#include "authtag.h"

extern int debug;

static uint8_t key[AUTHTAG_KEY_LEN];


/*
 * authtag_sign() - compute HMAC-SHA256 of message data using provided keys and
 * return a portion of size 'outlen' to the user's buffer as an authentication tag.
 *
 * HMAC-SHA256 is FIPS-198 and FIPS-140-2 compliant in this use case.
 *
 * It would be nice to use Blake-3 for increased speed.
 *
 * There's a bit of security by obsecurity in here as we select a portion of the HMAC
 * (of size outlen) from the overall HMAC by obtaining an offset dependant on the value
 * of the 23rd byte.
 *
 * An observer (watching the wire protocol) and without the source can't see this trick. 
 *
 */
void authtag_sign(uint8_t *out, int outlen, void *in, int inlen)
{
        uint8_t hmac[HMAC_SHA256_SIZE];
        int mod = HMAC_SHA256_SIZE - outlen;
        int idx;

        hmac_sha256(hmac, in, inlen, key, AUTHTAG_KEY_LEN);
        idx = hmac[22] % mod;
        memcpy(out, &hmac[idx], outlen);
}


/*
 * authtag_check() - check an authentication tag
 */
int authtag_check(uint8_t *tag, int taglen, uint8_t *in, int inlen)
{
        uint8_t hmac[HMAC_SHA256_SIZE];
        int mod = HMAC_SHA256_SIZE - taglen;
        int idx;
        int i, j = 0, k = 0;
        
        hmac_sha256(hmac, in, inlen, key, AUTHTAG_KEY_LEN);

        idx = hmac[22] % mod;

        for (i = 0; i < taglen; i++, idx++) {
        
                if (hmac[idx] == tag[i])
                        j++;
                else
                        k++;
        }

        return (k == 0);
}


/*
 * authtag_init() - create two keys for HMAC from user provided inner PSK (ipsk) and outer PSK (opsk)
 *
 * This takes a variable length pass-phase/secret key as the input and generates a 512-bit (64 byte)
 * SHA512 has as the output.
 *
 * The 512-bit output is effectively 'key expansion' from the input secret and results in 512-bits/
 * 64-bytes of material that is optimal for HMAC-SHA256 as this needs two 32-byte keys.
 * 
 */
void authtag_init(char *secret)
{
        sha512(key, secret, strlen(secret));

        if (debug)
                hex_dump("Key", key, AUTHTAG_KEY_LEN);
}

