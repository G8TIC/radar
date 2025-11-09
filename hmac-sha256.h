/*
 * hmac_sha256.h - simple, portable HMAC-SHA256 implementation (C11, procedural)
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Mike Tubby and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

#ifndef HMAC_SHA256_H
#define HMAC_SHA256_H

#include <stddef.h>
#include <stdint.h>

#include "sha256.h"

#define HMAC_SHA256_SIZE SHA256_DIGEST_SIZE

/*
 * Compute HMAC-SHA256
 *
 * out: pointer to output buffer (must be at least 32 bytes)
 * key: pointer to key bytes
 * key_len: length of key in bytes
 * data: pointer to input message
 * data_len: length of input message in bytes
 *
 * This function implements:
 *   HMAC(key, message) = SHA256((key XOR opad) || SHA256((key XOR ipad) || message))
 *
 */
void hmac_sha256(uint8_t out[SHA256_DIGEST_SIZE], const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len);

#endif
