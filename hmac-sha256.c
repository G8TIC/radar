/*
 * Copyright 2006 Apple Computer, Inc.  All rights reserved.
 * 
 * iTunes U Sample Code License
 * IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") 
 * in consideration of your agreement to the following terms, and your use, 
 * installation, modification or distribution of this Apple software constitutes 
 * acceptance of these terms.  If you do not agree with these terms, please do not use, 
 * install, modify or distribute this Apple software.
 * 
 * In consideration of your agreement to abide by the following terms and subject to
 * these terms, Apple grants you a personal, non-exclusive, non-transferable license, 
 * under Apple's copyrights in this original Apple software (the "Apple Software"): 
 * 
 * (a) to internally use, reproduce, modify and internally distribute the Apple 
 * Software, with or without modifications, in source and binary forms, within your 
 * educational organization or internal campus network for the sole purpose of 
 * integrating Apple's iTunes U software with your internal campus network systems; and 
 * 
 * (b) to redistribute the Apple Software to other universities or educational 
 * organizations, with or without modifications, in source and binary forms, for the 
 * sole purpose of integrating Apple's iTunes U software with their internal campus 
 * network systems; provided that the following conditions are met:
 * 
 * 	-  If you redistribute the Apple Software in its entirety and without 
 *     modifications, you must retain the above copyright notice, this entire license 
 *     and the disclaimer provisions in all such redistributions of the Apple Software.
 * 	-  If you modify and redistribute the Apple Software, you must indicate that you
 *     have made changes to the Apple Software, and you must retain the above
 *     copyright notice, this entire license and the disclaimer provisions in all
 *     such redistributions of the Apple Software and/or derivatives thereof created
 *     by you.
 *     -  Neither the name, trademarks, service marks or logos of Apple may be used to 
 *     endorse or promote products derived from the Apple Software without specific 
 *     prior written permission from Apple.  
 * 
 * Except as expressly stated above, no other rights or licenses, express or implied, 
 * are granted by Apple herein, including but not limited to any patent rights that may
 * be infringed by your derivative works or by other works in which the Apple Software 
 * may be incorporated.  THE APPLE SOFTWARE IS PROVIDED BY APPLE ON AN "AS IS" BASIS.  
 * APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AND HEREBY DISCLAIMS ALL WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE 
 * OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS OR SYSTEMS.  
 * APPLE IS NOT OBLIGATED TO PROVIDE ANY MAINTENANCE, TECHNICAL OR OTHER SUPPORT FOR 
 * THE APPLE SOFTWARE, OR TO PROVIDE ANY UPDATES TO THE APPLE SOFTWARE.  IN NO EVENT 
 * SHALL APPLE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION 
 * OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT 
 * (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.         
 * 
 * Rev.  120806												
 */


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>	//  Added by RKW, needed for types uint8_t, uint32_t; requires C99 compiler
#include <string.h>

#include "sha256.h"
#include "hmac-sha256.h"


//  Changed by RKW, formal arg is now const uint8_t
//    from const unsigned char

void hmac_sha256_initialize(hmac_sha256_t *hmac, const uint8_t *key, int length)
{
    int i;
    /* Prepare the inner hash key block, hashing the key if it's too long. */
    if (length <= 64) {
        for (i = 0; i < length; ++i) hmac->key[i] = key[i] ^ 0x36;
        for (; i < 64; ++i) hmac->key[i] = 0x36;
    } else {
        sha256_initialize(&(hmac->sha));
        sha256_finalize(&(hmac->sha), key, length);
        for (i = 0; i < 32; ++i) hmac->key[i] = hmac->sha.hash[i] ^ 0x36;
        for (; i < 64; ++i) hmac->key[i] = 0x36;
    }
    /* Initialize the inner hash with the key block. */
    sha256_initialize(&(hmac->sha));
    sha256_update(&(hmac->sha), hmac->key, 64);
}

//  Changed by RKW, formal arg is now const uint8_t
//    from const unsigned char
void hmac_sha256_update(hmac_sha256_t *hmac, const uint8_t *message, int length)
{
    /* Update the inner hash. */
    sha256_update(&(hmac->sha), message, length);
}

//  Changed by RKW, formal arg is now const uint8_t
//    from const unsigned char
void hmac_sha256_finalize(hmac_sha256_t *hmac, const uint8_t *message, int length)
{
    int i;
    /* Finalize the inner hash and store its value in the digest array. */
    sha256_finalize(&(hmac->sha), message, length);
    for (i = 0; i < 32; ++i) hmac->digest[i] = hmac->sha.hash[i];
    /* Convert the inner hash key block to the outer hash key block. */
    for (i = 0; i < 64; ++i) hmac->key[i] ^= (0x36 ^ 0x5c);
    /* Calculate the outer hash. */
    sha256_initialize(&(hmac->sha));
    sha256_update(&(hmac->sha), hmac->key, 64);
    sha256_finalize(&(hmac->sha), hmac->digest, 32);
    /* Use the outer hash value as the HMAC digest. */
    for (i = 0; i < 32; ++i) hmac->digest[i] = hmac->sha.hash[i];
}


// copy result back to user
void hmac_sha256_result(hmac_sha256_t *hmac, uint8_t digest[SHA256_SIZE])
{
    memcpy(digest, hmac->digest, SHA256_SIZE);
}


//  Changed by RKW, formal args are now uint8_t, const uint8_t
//    from unsinged char, const unsigned char respectively
void hmac_sha256(uint8_t digest[SHA256_SIZE], const uint8_t *message, int message_length, const uint8_t *key, int key_length)
{
    hmac_sha256_t hmac;

    hmac_sha256_initialize(&hmac, key, key_length);
    hmac_sha256_finalize(&hmac, message, message_length);
    hmac_sha256_result(&hmac, digest);
}




#if 0

int main(void)
{
    uint8_t hmac[SHA256_SIZE];
    uint8_t key[64];
    uint8_t msg[128];
    int i;
    
    for (i=0; i<sizeof(key); i++)
        key[i] = i;
        
    for (i=0; i<100000; i++) {
        msg[0] = i>>8;
        msg[1] = i & 0xff;
        
        hmac_sha256(hmac, msg, sizeof(msg), key, sizeof(key));
    }
    
}

#endif



#if 0


/******************************************************************************
 * Input/output.
 */

int main(int argc, const char *const *argv) {
    hmac_sha256 hmac;
    sha256 sha;
    unsigned char key[64];
    unsigned char block[1024];
    int i, c, d;
    /* Parse and verify arguments. */
    int hexadecimals = (argc == 2 && strcmp(argv[1], "-x") == 0);
    if (argc > 2 || argc > 1 && !hexadecimals) {
        fprintf(stderr, "%s -- %s\n%s\n%s\n%s\n%s\n",
                "hmac-sha256: illegal option", argv[1],
                "usage: hmac-sha256 [ -x ]",
                "       -x interpret the key line as a hexadecimal value",
                "       the first line of input should be the key",
                "       the rest of the input should be the message to sign");
        exit(1);
    }
    /* Read the key, hashing it if necessary. */
    sha256_initialize(&sha);
    for (i = 0; (c = getchar()) > 31 && c < 127; ++i) {
        if (i > 0 && i % 64 == 0) sha256_update(&sha, key, 64);
        if (!hexadecimals) {
            key[i % 64] = c;
        } else if (isxdigit(c) && isxdigit(d = getchar())) {
            key[i % 64] = (c % 32 + 9) % 25 * 16 + (d % 32 + 9) % 25;
        } else {
            c = '?';
            break;
        }
    }
    /* Handle "\r\n" and "\r" like "\n". */
    if (i > 0 && c == '\r' && (c = getchar()) != '\n' && c != EOF) {
        ungetc(c, stdin);
        c = '\n';
    }
    /* Display an error and exit if the key is invalid. */
    if (i == 0 || c != '\n' && c != EOF) {
        fprintf(stderr, "hmac-sha256: invalid key\n");
        exit(1);
    }
    /* Initialize the HMAC-SHA256 digest with the key or its hash. */
    if (i <= 64) {
        hmac_sha256_initialize(&hmac, key, i);
    } else {
        sha256_finalize(&sha, key, i % 64);
        hmac_sha256_initialize(&hmac, sha.hash, 64);
    }
    /* Read the message, updating the HMAC-SHA256 digest accordingly. */
    if (c != EOF) {
        while (!feof(stdin) && !ferror(stdin)) {
            i = fread(block, 1, sizeof(block), stdin);
            hmac_sha256_update(&hmac, block, i);
        }
    }
    /* Finalize the HMAC-SHA256 digest and output its value. */
    hmac_sha256_finalize(&hmac, NULL, 0);
    for (i = 0; i < 32; ++i) {
		//  Cast added by RKW to get format specifier to work as expected
        printf("%02lx", (unsigned long)hmac.digest[i]);
    }
    putchar('\n');
    /* That's all folks! */
    return 0;
}

#endif
