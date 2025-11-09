/*
 * hmac_sha256.c - simple, portable HMAC-SHA256 implementation (C11, procedural)
 *
 * SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Mike Tubby G8TIC mike@tubby.org and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 *
 */

#include <stdio.h>
#include <string.h>

#include "hmac-sha256.h"

#define IPAD 0x36
#define OPAD 0x5c

void hmac_sha256(uint8_t out[SHA256_DIGEST_SIZE], const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len)
{
    uint8_t lkey[SHA256_BLOCK_SIZE];		/* local key */
    uint8_t okey[SHA256_BLOCK_SIZE];		/* outer key */
    uint8_t ikey[SHA256_BLOCK_SIZE];		/* inner key */
    uint8_t hash[SHA256_DIGEST_SIZE];		/* inner/temporary hash */
    size_t i;

    /* Step 1: process the key */
    memset(lkey, 0, SHA256_BLOCK_SIZE);
    
    if (key_len > SHA256_BLOCK_SIZE) {
        /* hash long keys down to 32 bytes */
        sha256(lkey, key, key_len);
    } else {
        memcpy(lkey, key, key_len);
    }

    /* Step 2: prepare inner and outer padded keys */
    for (i = 0; i < SHA256_BLOCK_SIZE; ++i) {
        ikey[i] = lkey[i] ^ IPAD;
        okey[i] = lkey[i] ^ OPAD;
    }

    /* Step 3: inner hash = SHA256(ikey || data) */
    {
        sha256_ctx ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, ikey, SHA256_BLOCK_SIZE);
        sha256_update(&ctx, data, data_len);
        sha256_final(&ctx, hash);
    }

    /* Step 4: outer hash = SHA256(okey || hash) */
    {
        sha256_ctx ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, okey, SHA256_BLOCK_SIZE);
        sha256_update(&ctx, hash, SHA256_DIGEST_SIZE);
        sha256_final(&ctx, out);
    }

    /* clean up - don't leave sensitive data in memory */
    memset(lkey, 0, sizeof(lkey));
    memset(ikey, 0, sizeof(ikey));
    memset(okey, 0, sizeof(okey));
    memset(hash, 0, sizeof(hash));
}

