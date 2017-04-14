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

#ifndef _UTIL_H
#define _UTIL_H


#include "uld.h"


#define UTIL_CRC32_POLY                             0xEDB88320
#define UTIL_CRC32_INIT                             0x00000000

#define UTIL_MIN_UTOHEX_BUF                         9
#define UTIL_MIN_UITOA_BUF                          12


uint32_t crc32(const void *buf, size_t size, uint32_t crc) __nonnull;

// Returns number of bytes written excluding null terminator or negative on
// error.  s must be UTIL_MIN_UTOHEX_BUF bytes.
int utohex(char *s, uint32_t v);

// Returns number of bytes written excluding null terminator or negative on
// error.  Argument l will be the minimum number of bytes printed (0 padded)
// but s must be UTIL_MIN_UTOHEX_BUF bytes.
int utohex_pad(char *s, uint32_t v, int l);

// Returns number of bytes written excluding null terminator or negative on
// error.  Bytes are written with the null terminator at
// s[UTIL_MIN_UITOA_BUF - 1]
int uitoa(char *s, unsigned int v, int is_signed);

// Copies string including the NULL byte where dest points to end of the
// string.  Returns the start of the copied string (dest - strlen(src)).
char *stprcpy(char *dest, const char *src);

// Return index if string s in in array l, < 0 if not.
int get_str_idx_in_list(const char * const *l, int lnum, const char *s);

// Returns 1 if string s is in array l, 0 if not.
int str_in_list(const char * const *l, int lnum, const char *s);


#endif  // _UTIL_H
