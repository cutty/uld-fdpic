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

#include "uld.h"
#include "base64.h"


static const char base64_char[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


ssize_t base64_encode(void *dest, const void **src, ssize_t n,
        struct base64_state *state)
{
    const uint8_t *srcp;
    uint8_t *destp = dest;
    unsigned int dest_rem;
    unsigned int brem;
    unsigned int brem_len;

    if (!dest || !state || state->final || n < 0) {
        return -1;
    }

    if (!state->init) {
        if (state->dest_len < 5) {
            return -1;
        }
        state->dest_len = (((state->dest_len - 1) / 4) * 4) + 1;
        state->dest_pos = 0;
        state->brem = 0;
        state->brem_len = 0;
        state->init = 1;
        state->final = 0;
    }

    if (state->dest_pos == state->dest_len) {
        state->dest_pos = 0;
    }

    destp += state->dest_pos;
    dest_rem = state->dest_len - state->dest_pos;
    brem = state->brem;
    brem_len = state->brem_len;

    // Do final.
    if (!n) {
        state->final = 1;

        // Since the state init above ensured whole lines will never have
        // padding the code below already appended null char.  Nothing to do.
        if (state->dest_pos == 0) {
            return 0;
        }

        switch (brem_len) {
        case 0:
            // No padding but not a whole line, drop below to append null char.
            state->dest_pos += 1;
            break;

        case 1:
            *destp++ = base64_char[brem << 4];
            *destp++ = '=';
            *destp++ = '=';
            state->dest_pos += 4;
            break;

        case 2:
            *destp++ = base64_char[brem << 2];
            *destp++ = '=';
            state->dest_pos += 3;
            break;

        default:
            return -1;
            break;
        }

        *destp++ = '\0';
        return 0;
    }

    if (!src || !*src) {
        return -1;
    }
    srcp = *src;

    do {
        switch (brem_len) {
        case 0:
            *destp++ = base64_char[*srcp >> 2];
            brem = *srcp++ & 0x3;
            brem_len++;
            break;

        case 1:
            *destp++ = base64_char[brem << 4 | *srcp >> 4];
            brem = *srcp++ & 0xf;
            brem_len++;
            break;

        case 2:
            *destp++ = base64_char[brem << 2 | *srcp >> 6];
            *destp++ = base64_char[*srcp++ & 0x3f];
            brem = 0;
            brem_len = 0;
            dest_rem--;
            break;

        default:
            return -1;
            break;
        }

        n--;
        dest_rem--;

        if (dest_rem == 1) {
            *destp = '\0';
            dest_rem--;
        }
    } while (n && dest_rem);

    state->dest_pos = state->dest_len - dest_rem;
    state->brem = brem;
    state->brem_len = brem_len;
    *src = srcp;

    return n;
}

ssize_t base64_final(void *dest, struct base64_state *state)
{
    return base64_encode(dest, NULL, 0, state);
}

ssize_t base64_encode_cb_puts(void *dest, const void *src, ssize_t n,
        struct base64_state *state, int (*cb)(const char *))
{
    const void *p = src;
    int ret;

    while (n > 0) {
        n = base64_encode(dest, &p, n, state);
        if (n <= 0) {
            return n;
        }
        ret = cb(dest);
        if (ret < 0) {
            return ret;
        }
    }
    return n;
}

ssize_t base64_final_cb_puts(void *dest, struct base64_state *state,
        int (*cb)(const char *))
{
    ssize_t ret;
    ret = base64_final(dest, state);
    if (ret == 0) {
        ret = cb(dest);
    }
    if (ret > 0) {
        ret = 0;
    }
    return ret;
}

// Mostly duplicate code, but it is unlikely both the puts and the write
// callback functions would be used in the same program.  This is going
// to favor stack size over code reuse using a callback redirector.
ssize_t base64_encode_cb_write(void *dest, const void *src, ssize_t n,
        struct base64_state *state,
        ssize_t (*cb)(int fd, const void *buf, size_t count), int fd)
{
    const void *p = src;
    ssize_t ret;

    while (n > 0) {
        n = base64_encode(dest, &p, n, state);
        if (n <= 0) {
            return n;
        }
        // This will only ever output whole lines only.  Use
        // state->dest_len - 1 for trailing null char.
        ret = cb(fd, dest, state->dest_len - 1);
        if (ret < 0) {
            return ret;
        }
    }
    return n;
}

ssize_t base64_final_cb_write(void *dest, struct base64_state *state,
        ssize_t (*cb)(int fd, const void *buf, size_t count), int fd)
{
    unsigned int len;
    ssize_t ret;

    ret = base64_final(dest, state);
    if (ret == 0) {
        // If state->dest_pos == 0 dest contains a whole line (this
        // happens in base64_encode).  In either case -1 for trailing
        // null char.
        len = (state->dest_pos) ? state->dest_pos : state->dest_len;
        len--;
        ret = cb(fd, dest, len);
    }
    if (ret > 0) {
        ret = 0;
    }
    return ret;
}
