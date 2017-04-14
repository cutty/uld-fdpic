/*
 * Copyright (c) 2016, 2017 Joe Vernaci
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
*/

#ifndef _BASE64_H
#define _BASE64_H


#include "uld.h"


struct base64_state {
    unsigned int dest_len:8;
    unsigned int dest_pos:8;
    unsigned int brem:4;
    unsigned int brem_len:2;
    unsigned int init:1;
    unsigned int final:1;
};

#define BASE64_STATE_INIT(__DEST_LEN__) {.dest_len = __DEST_LEN__}


ssize_t base64_encode(void *dest, const void **src, ssize_t n,
        struct base64_state *state) __nonnull_a(1, 4);

ssize_t base64_final(void *dest, struct base64_state *state) __nonnull;

ssize_t base64_encode_cb_puts(void *dest, const void *src, ssize_t n,
        struct base64_state *state, int (*cb)(const char *)) __nonnull;

ssize_t base64_final_cb_puts(void *dest, struct base64_state *state,
        int (*cb)(const char *)) __nonnull;

ssize_t base64_encode_cb_write(void *dest, const void *src, ssize_t n,
        struct base64_state *state,
        ssize_t (*cb)(int fd, const void *buf, size_t count), int fd)
        __nonnull;

ssize_t base64_final_cb_write(void *dest, struct base64_state *state,
        ssize_t (*cb)(int fd, const void *buf, size_t count), int fd)
        __nonnull;


#endif  // _BASE64_H
