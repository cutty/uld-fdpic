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
#include "util.h"


static const char hexchar[16] = "0123456789abcdef";


// CRC32 implimentation that matches crc32 installed by libarchive-zip-perl
// and zlib.crc32 in Python's standard libraries.
uint32_t crc32(const void *buf, size_t size, uint32_t crc) {
    const uint8_t *p = (const uint8_t *)buf;
    int i;

    crc ^= ~0;
    while (size--) {
        crc ^= (uint32_t)*p++;
        i = 8;
        while (i--) {
            if (crc & 0x1) {
                crc = (crc >> 1) ^ UTIL_CRC32_POLY;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^= ~0;
}

int utohex(char *s, uint32_t v)
{
    int r = 0;
    int c = 0;
    uint32_t t;

    if (!s) {
        return -1;
    }

    // Moving forward through the array.
    while (++c <= 8) {
        t = v >> 28;
        // Write if (t) non zero char, (r) have written a char,
        if (t || r) {
            *s++ = hexchar[t];
            r++;
        }
        v <<= 4;
    }
    *s = '\0';

    return r;
}

int utohex_pad(char *s, uint32_t v, int l)
{
    int r = 0;
    int c = 0;
    uint32_t t;

    if (!s || l < 0 || l > 8) {
        return -1;
    }

    // Moving forward through the array.
    while (++c <= 8) {
        t = v >> 28;
        // Write if (t) non zero char, (r) have written a char,
        // ((8 - l - c) < 0) need padding based on l.
        if (t || r || (8 - l - c) < 0) {
            *s++ = hexchar[t];
            r++;
        }
        v <<= 4;
    }
    *s = '\0';

    return r;
}

int uitoa(char *s, unsigned int v, int is_signed)
{
    unsigned int t;
    int is_neg = 0;
    char *p;

    if (!s) {
        return -1;
    }

    s[11] = '\0';
    p = &s[10];

    if (is_signed && (int)v < 0) {
#if 0
        v = (unsigned int)((int)v * -1);
#else
        v ^= 0xffffffff;
        v += 1;
#endif
        is_neg = 1;
    }

    do {
#if 0
        *p-- = (v % 10) + '0';
        v /= 10;
#else
        t = v / 10;
        *p-- = v - (10 * t) + '0';
        v = t;
#endif
    } while(v);

    if (is_neg) {
        *p-- = '-';
    }

    return &s[10] - p;
}

char *stprcpy(char *dest, const char *src)
{
    size_t l = strlen(src);
    dest -= l;
    // Use memmove to handle when src and dest overlap.
    memmove(dest, src, l + 1);
    return dest;
}

int get_str_idx_in_list(const char * const *l, int lnum, const char *s)
{
    int idx = lnum;
    while (idx--) {
        if (!strcmp(*l, s)) {
            return lnum - idx - 1;
        }
        l++;
    }
    return -1;
}

int str_in_list(const char * const *l, int lnum, const char *s)
{
    return get_str_idx_in_list(l, lnum, s) < 0 ? 0 : 1;
}
