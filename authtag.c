/*
 * authtag.h -- Authentication tag signing of Radar messages
 * Author: Michael J. Tubby B.Sc. MIET G8TIC    mike@tubby.org
 *
 * Radar V2 uses a 64-bit Authentication Tag on each message to protect data
 * in transit from corruption, forgery and replay attacks.
 *
 * The Authentication Tag is a truncated HMAC-SHA256 of the message using a
 * pre-shared key known only to the originator and recipient.  The pre-shared
 * key is in the form of a pass-phrase that is expanded to a 512-bit data block
 * using SHA512 (key expansion).
 *
 * If both parties set a pre-shared key/pass-phrase then the aggregator can
 * authenticate messages and be sure that they have not be damaged in transit,
 * have not been spoofed and come from the originator.
 *
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
 * authtag_init() - create key for ue by HMAC-256 functions later
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

