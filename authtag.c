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

#if 0
static uint8_t testkey[AUTHTAG_KEY_LEN] = {
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
                0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f		};
#endif

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

        //hmac_sha256(hmac, in, inlen, testkey, AUTHTAG_KEY_LEN);
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
 */
void authtag_init(char *secret)
{
        sha512(key, secret, strlen(secret));

        if (debug)
                hex_dump("Key", key, AUTHTAG_KEY_LEN);
}

