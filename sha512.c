/*
 * sha512.c - simple, portable SHA-512 implementation (C11, procedural)
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <stdio.h>

#include "sha512.h"

#define ROTR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define SHR(x, n) ((x) >> (n))
#define ch(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define bsig0(x) (ROTR64((x), 28) ^ ROTR64((x), 34) ^ ROTR64((x), 39))
#define bsig1(x) (ROTR64((x), 14) ^ ROTR64((x), 18) ^ ROTR64((x), 41))
#define ssig0(x) (ROTR64((x), 1)  ^ ROTR64((x), 8)  ^ SHR((x), 7))
#define ssig1(x) (ROTR64((x), 19) ^ ROTR64((x), 61) ^ SHR((x), 6))

static const uint64_t k[80] = {
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 
    0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 
    0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 
    0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694, 
    0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 
    0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 
    0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70, 
    0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df, 
    0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b, 
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 
    0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 
    0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 
    0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3, 
    0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec, 
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 
    0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 
    0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b, 
    0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c, 
    0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

static void sha512_transform(sha512_ctx *ctx, const uint8_t block[SHA512_BLOCK_SIZE])
{
    uint64_t w[80];
    uint64_t a, b, c, d, e, f, g, h;
    size_t i;

    /* prepare message schedule (big-endian) */
    for (i = 0; i < 16; ++i) {
        w[i] = ((uint64_t)block[i * 8 + 0] << 56)
             | ((uint64_t)block[i * 8 + 1] << 48)
             | ((uint64_t)block[i * 8 + 2] << 40)
             | ((uint64_t)block[i * 8 + 3] << 32)
             | ((uint64_t)block[i * 8 + 4] << 24)
             | ((uint64_t)block[i * 8 + 5] << 16)
             | ((uint64_t)block[i * 8 + 6] << 8)
             | ((uint64_t)block[i * 8 + 7]);
    }

    for (i = 16; i < 80; ++i) {
        w[i] = ssig1(w[i - 2]) + w[i - 7] + ssig0(w[i - 15]) + w[i - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 80; ++i) {
        uint64_t t1 = h + bsig1(e) + ch(e, f, g) + k[i] + w[i];
        uint64_t t2 = bsig0(a) + maj(a, b, c);
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

void sha512_init(sha512_ctx *ctx)
{
    ctx->state[0] = 0x6a09e667f3bcc908ULL;
    ctx->state[1] = 0xbb67ae8584caa73bULL;
    ctx->state[2] = 0x3c6ef372fe94f82bULL;
    ctx->state[3] = 0xa54ff53a5f1d36f1ULL;
    ctx->state[4] = 0x510e527fade682d1ULL;
    ctx->state[5] = 0x9b05688c2b3e6c1fULL;
    ctx->state[6] = 0x1f83d9abfb41bd6bULL;
    ctx->state[7] = 0x5be0cd19137e2179ULL;
    ctx->bitlen_high = 0;
    ctx->bitlen_low = 0;
    ctx->buffer_len = 0;
    /* buffer need not be cleared here */
}

static void sha512_add_bits(sha512_ctx *ctx, size_t byte_count)
{
    /* add byte_count * 8 to 128-bit bit length stored as high/low */
    uint64_t add = (uint64_t)byte_count * 8ULL;
    uint64_t prev = ctx->bitlen_low;
    ctx->bitlen_low += add;
    if (ctx->bitlen_low < prev) {
        /* overflow of low 64-bit part */
        ctx->bitlen_high += 1ULL;
    }
}

void sha512_update(sha512_ctx *ctx, const uint8_t *data, size_t len)
{
    size_t i = 0;

    if (len == 0 || data == NULL) {
        return;
    }

    while (len > 0) {
        size_t to_copy = SHA512_BLOCK_SIZE - ctx->buffer_len;
        if (to_copy > len) to_copy = len;

        memcpy(ctx->buffer + ctx->buffer_len, data + i, to_copy);
        ctx->buffer_len += to_copy;
        i += to_copy;
        len -= to_copy;

        /* update bit length for the message bytes only */
        sha512_add_bits(ctx, to_copy);

        if (ctx->buffer_len == SHA512_BLOCK_SIZE) {
            sha512_transform(ctx, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
}

void sha512_final(sha512_ctx *ctx, uint8_t out[SHA512_DIGEST_SIZE])
{
    size_t i;
    /* preserve message length (before padding) */
    uint64_t high = ctx->bitlen_high;
    uint64_t low  = ctx->bitlen_low;

    /* append the '1' bit (0x80) into the buffer (without changing bitlen) */
    ctx->buffer[ctx->buffer_len++] = 0x80u;

    /* if buffer_len > 112, we need to pad to end of block, transform, then pad another block */
    if (ctx->buffer_len > 112) {
        /* pad with zeros to full block */
        while (ctx->buffer_len < SHA512_BLOCK_SIZE) {
            ctx->buffer[ctx->buffer_len++] = 0x00u;
        }
        sha512_transform(ctx, ctx->buffer);
        ctx->buffer_len = 0;
    }

    /* pad with zeros until buffer has 112 bytes (so last 16 bytes are for length) */
    while (ctx->buffer_len < 112) {
        ctx->buffer[ctx->buffer_len++] = 0x00u;
    }

    /* append 128-bit length big-endian: high then low */
    for (i = 0; i < 8; ++i) {
        ctx->buffer[ctx->buffer_len++] = (uint8_t)(high >> (56 - i * 8));
    }
    for (i = 0; i < 8; ++i) {
        ctx->buffer[ctx->buffer_len++] = (uint8_t)(low >> (56 - i * 8));
    }

    /* now process final block(s) */
    if (ctx->buffer_len == SHA512_BLOCK_SIZE) {
        sha512_transform(ctx, ctx->buffer);
    } else {
        /* should not happen because we filled to 128, but handle defensively */
        memset(ctx->buffer + ctx->buffer_len, 0, SHA512_BLOCK_SIZE - ctx->buffer_len);
        sha512_transform(ctx, ctx->buffer);
    }

    /* produce final digest (big-endian state words) */
    for (i = 0; i < 8; ++i) {
        out[i * 8 + 0] = (uint8_t)(ctx->state[i] >> 56);
        out[i * 8 + 1] = (uint8_t)(ctx->state[i] >> 48);
        out[i * 8 + 2] = (uint8_t)(ctx->state[i] >> 40);
        out[i * 8 + 3] = (uint8_t)(ctx->state[i] >> 32);
        out[i * 8 + 4] = (uint8_t)(ctx->state[i] >> 24);
        out[i * 8 + 5] = (uint8_t)(ctx->state[i] >> 16);
        out[i * 8 + 6] = (uint8_t)(ctx->state[i] >> 8);
        out[i * 8 + 7] = (uint8_t)(ctx->state[i]);
    }

    /* clear the context */
    memset(ctx, 0, sizeof(*ctx));
}

void sha512(uint8_t out[SHA512_DIGEST_SIZE], const uint8_t *data, size_t len)
{
    sha512_ctx ctx;
    sha512_init(&ctx);
    if (data != NULL && len > 0) sha512_update(&ctx, data, len);
    sha512_final(&ctx, out);
}

