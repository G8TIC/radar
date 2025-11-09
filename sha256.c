/*
 * sha256.c - simple, portable SHA-256 implementation in procedural C11
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

#include "sha256.h"

/* SHA-256 constants */
static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* rotr32() - 32 bit circular rotate right */
static inline uint32_t rotr32(uint32_t x, unsigned n)
{
    return (x >> n) | (x << (32 - n));
}

#define ch(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define maj(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define big_sigma0(x) (rotr32((x),2) ^ rotr32((x),13) ^ rotr32((x),22))
#define big_sigma1(x) (rotr32((x),6) ^ rotr32((x),11) ^ rotr32((x),25))
#define small_sigma0(x) (rotr32((x),7) ^ rotr32((x),18) ^ ((x) >> 3))
#define small_sigma1(x) (rotr32((x),17) ^ rotr32((x),19) ^ ((x) >> 10))


/* sha256_transform() - internal transform function */
static void sha256_transform(sha256_ctx *ctx, const uint8_t block[SHA256_BLOCK_SIZE])
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    size_t t;

    for (t = 0; t < 16; ++t) {
        size_t i = t * 4;
        w[t] = ((uint32_t)block[i] << 24) | ((uint32_t)block[i+1] << 16) | ((uint32_t)block[i+2] << 8) | ((uint32_t)block[i+3]);
    }

    for (t = 16; t < 64; ++t) {
        w[t] = small_sigma1(w[t-2]) + w[t-7] + small_sigma0(w[t-15]) + w[t-16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (t = 0; t < 64; ++t) {
        uint32_t t1 = h + big_sigma1(e) + ch(e,f,g) + k[t] + w[t];
        uint32_t t2 = big_sigma0(a) + maj(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}


/*
 * sha256_init() - Initialise context
 */
void sha256_init(sha256_ctx *ctx)
{
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->bitlen = ctx->datalen = 0;
    memset(ctx->data, 0, sizeof(ctx->data));
}


/*
 * sha256_update() - Add a chunk of data into an existing SHA256 computation
 */
void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len)
{
    size_t i = 0;
    while (len > 0) {
        size_t to_copy = SHA256_BLOCK_SIZE - ctx->datalen;
        if (to_copy > len) to_copy = len;
        memcpy(ctx->data + ctx->datalen, data + i, to_copy);
        ctx->datalen += to_copy;
        i += to_copy;
        len -= to_copy;

        if (ctx->datalen == SHA256_BLOCK_SIZE) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += (uint64_t)SHA256_BLOCK_SIZE * 8;
            ctx->datalen = 0;
            memset(ctx->data, 0, SHA256_BLOCK_SIZE);
        }
    }
}


/*
 * sha256_final() - Finish off SHA256 computation and output result
 */
void sha256_final(sha256_ctx *ctx, uint8_t out[SHA256_DIGEST_SIZE])
{
    uint64_t total_bits = ctx->bitlen + (uint64_t)ctx->datalen * 8;
    ctx->data[ctx->datalen++] = 0x80;

    if (ctx->datalen > 56) {
        while (ctx->datalen < 64)
            ctx->data[ctx->datalen++] = 0;
        sha256_transform(ctx, ctx->data);
        ctx->datalen = 0;
        memset(ctx->data, 0, 56);
    }

    while (ctx->datalen < 56)
        ctx->data[ctx->datalen++] = 0;

    for (int i = 0; i < 8; ++i)
        ctx->data[63 - i] = (uint8_t)((total_bits >> (8 * i)) & 0xFF);

    sha256_transform(ctx, ctx->data);

    for (int i = 0; i < 8; ++i) {
        out[i*4+0]=(uint8_t)((ctx->state[i]>>24) & 0xFF);
        out[i*4+1]=(uint8_t)((ctx->state[i]>>16) & 0xFF);
        out[i*4+2]=(uint8_t)((ctx->state[i]>>8)  & 0xFF);
        out[i*4+3]=(uint8_t)((ctx->state[i])     & 0xFF);
    }

    memset(ctx,0,sizeof(*ctx));
}


/*
 * sha256() - compute the SHA256 of 'data' of size 'len' and put the result at 'out'
 */
void sha256(uint8_t out[SHA256_DIGEST_SIZE], const uint8_t *data, size_t len)
{
    sha256_ctx ctx;
    sha256_init(&ctx);
    if (data && len > 0)
        sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}


/*
 * sha256_compare() - compare to SHA256 sums
 */
int sha256_compare(uint8_t *a, uint8_t *b)
{
        return (memcmp(a, b, SHA256_DIGEST_SIZE) == 0);
}
