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


#define ULD_FWRITE_FLAGS_LJUST     0x00000001
#define ULD_FWRITE_FLAGS_RJUST     0x00000002
#define ULD_FWRITE_FLAGS_LSPACE    0x00000004
#define ULD_FWRITE_FLAGS_ZERO_PAD  0x00000008
#define ULD_FWRITE_FLAGS_LONG      0x00000010
#define ULD_FWRITE_FLAGS_LONG_LONG 0x00000020
#define ULD_FWRITE_FLAGS_SIGNED    0x00000040


void *_uld_memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = dest;
    const uint8_t *s = src;

    while (n--) {
        *d++ = *s++;
    }
    return dest;
}
__export void *memcpy(void *dest, const void *src, size_t n) __weak
        __alias("_uld_memcpy");

void *_uld_memmove(void *dest, const void *src, size_t n)
{
    uint8_t *d = dest;
    const uint8_t *s = src;

    // Find copy direction to prevent overwriting src if memory areas overlap.
    if (s > d) {
        // Copy moving forward.
        while (n--) {
            *d++ = *s++;
        }
    } else if (s < d) {
        // Copy moving backward.
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    // s == d, do nothing.

    return dest;
}
__export void *memmove(void *dest, const void *src, size_t n)
        __weak __alias("_uld_memmove");

void *_uld_memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;
    while (n--) {
        *p++ = (char)c;
    }
    return s;
}
__export void *memset(void *s, int c, size_t n) __weak __alias("_uld_memset");

int _uld_strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}
__export int strcmp(const char *s1, const char *s2) __weak
        __alias("_uld_strcmp");

char *_uld_strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}
__export char *strcpy(char *dest, const char *src) __weak
        __alias("_uld_strcpy");

size_t _uld_strlen(const char *s)
{
    const char *b = s;
    while(*s++);
    return s - b - 1;
}
__export size_t strlen(const char *s) __weak __alias("_uld_strlen");

size_t _uld_strnlen(const char *s, size_t maxlen)
{
    const char *b = s;
    while(*s++ && maxlen--);
    return s - b - 1;
}
__export size_t strnlen(const char *s, size_t maxlen) __weak
        __alias("_uld_strnlen");

ssize_t _uld_write(int fd, const void *buf, size_t count)
{
    FILE *stream = stdout;
    return stream->write_f(stream->fd, buf, count);
}
__export ssize_t write(int fd, const void *buf, size_t count)
        __weak __alias("_uld_write");

int _uld_fputc(int c, FILE *stream)
{
    return stream->dputchar_f(stream->fd, c);
}
__export int fputc(int c, FILE *stream) __weak __alias("_uld_fputc");

int _uld_fputs(const char *s, FILE *stream)
{
    ssize_t len = _uld_strlen(s);
    if (stream->write_f(stream->fd, s, len) == len) {
        return len;
    }
    return EOF;
}
__export int fputs(const char *s, FILE *stream) __weak __alias("_uld_fputs");

int _uld_putc(int c, FILE *stream)
{
    return stream->dputchar_f(stream->fd, c);
}
__export int putc(int c, FILE *stream) __weak __alias("_uld_putc");

int _uld_putchar(int c)
{
    FILE *stream = stdout;
    return stream->dputchar_f(stream->fd, c);
}
__export int putchar(int c) __weak __alias("_uld_putchar");

int _uld_puts(const char *s)
{
    FILE *stream = stdout;
    int ret = _uld_fputs(s, stream);
    if (ret >= 0 && stream->dputchar_f(stream->fd, '\n') == '\n') {
        return ret + 1;
    }
    return EOF;
}
__export int puts(const char *s) __weak __alias("_uld_puts");

size_t _uld_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    ssize_t r = stream->write_f(stream->fd, ptr, size * nmemb);
    if (r < 0) {
        return EOF;
    }
    return (size_t)r / size;
}
__export size_t fwrite(const void *ptr, size_t size, size_t nmemb,
        FILE *stream) __weak __alias("_uld_fwrite");

static int _uld_fwrite_pad(FILE *stream, int count, int zero_pad) {
    const char *chars = zero_pad ? "0000000000" : "          ";
    ssize_t r;
    int c = count;

    if (count <= 0) {
        return 0;
    }

    while (c) {
        r = stream->write_f(stream->fd, chars, MIN(c, 10));
        if (r < 0) {
            return EOF;
        }
        c -= r;
    }

    return count;
}

static int _uld_fwrite_hex(FILE *stream, uint32_t v, int width, int flags)
{
    char s[UTIL_MIN_UTOHEX_BUF];
    ssize_t r;
    int ljust = flags & ULD_FWRITE_FLAGS_LJUST;
    int bytes = utohex(s, v);

    if (bytes < 0) {
        return EOF;
    }

    if (!ljust && width - bytes > 0) {
        if (_uld_fwrite_pad(stream, width - bytes,
                flags & ULD_FWRITE_FLAGS_ZERO_PAD) == EOF) {
            return EOF;
        }
    }

    r = stream->write_f(stream->fd, s, bytes);
    if (r == EOF) {
        return r;
    }

    if (ljust && width - bytes > 0) {
        if (_uld_fwrite_pad(stream, width - bytes, 0) == EOF) {
            return EOF;
        }
    }

    return MAX(bytes, width);
}

static int _uld_fwrite_dec(FILE *stream, unsigned int v, int width, int flags)
{
    char s[UTIL_MIN_UITOA_BUF];
    char *p;
    ssize_t r = 0;
    int ljust = flags & ULD_FWRITE_FLAGS_LJUST;
    int pos_space = (flags &
            (ULD_FWRITE_FLAGS_LSPACE | ULD_FWRITE_FLAGS_SIGNED)) ==
            (ULD_FWRITE_FLAGS_LSPACE | ULD_FWRITE_FLAGS_SIGNED);
    int bytes = uitoa(s, v, flags & ULD_FWRITE_FLAGS_SIGNED);
    int pc = 0;

    if (bytes < 0) {
        return EOF;
    }

    // null char offset.
    p = &s[UTIL_MIN_UITOA_BUF - bytes - 1];

    if (*p == '-') {
        r = stream->dputchar_f(stream->fd, *p);
        p++;
        pc = -1;
    } else if (pos_space && *p != '-') {
        r = stream->dputchar_f(stream->fd, ' ');
        bytes++;
        pc = -1;
    }
    if (r == EOF) {
        return r;
    }

    if (!ljust && width - bytes > 0) {
        if (_uld_fwrite_pad(stream, width - bytes,
                flags & ULD_FWRITE_FLAGS_ZERO_PAD) == EOF) {
            return EOF;
        }
    }

    r = stream->write_f(stream->fd, p, bytes + pc);
    if (r == EOF) {
        return r;
    }

    if (ljust && width - bytes > 0) {
        if (_uld_fwrite_pad(stream, width - bytes, 0) == EOF) {
            return EOF;
        }
    }

    return MAX(bytes, width);
}

static int _uld_fwrite_str(FILE *stream, const char *s, int width, int flags)
{
    ssize_t r;
    int ljust = flags & ULD_FWRITE_FLAGS_LJUST;
    int bytes = _uld_strlen(s);

    if (!ljust && width - bytes > 0) {
        if (_uld_fwrite_pad(stream, width - bytes,
                flags & ULD_FWRITE_FLAGS_ZERO_PAD) == EOF) {
            return EOF;
        }
    }

    r = stream->write_f(stream->fd, s, bytes);
    if (r == EOF) {
        return r;
    }

    if (ljust && width - bytes > 0) {
        if (_uld_fwrite_pad(stream, width - bytes, 0) == EOF) {
            return EOF;
        }
    }

    return MAX(bytes, width);
}

int _uld_vfprintf(FILE *stream, const char *format, va_list ap)
{
    const char *sptr;
    const char *ptr;
    int count = 0;
    int flags;
    int width;

    if (!stream || !format) {
        return EOF;
    }

    ptr = format;

    while (*ptr) {
        sptr = ptr;
        while (*ptr && *ptr != '%') {
            ptr++;
        }

        count += stream->write_f(stream->fd, sptr, ptr - sptr);

        if (!*ptr) {
            goto done;
        }

        if (*ptr != '%') {
            swbkpt();
        }

        flags = 0;
        width = 0;

next_flag_mod:
        ptr++;
        switch (*ptr) {
        case ' ':
            flags |= ULD_FWRITE_FLAGS_LSPACE;
            goto next_flag_mod;
            break;

        case '-':
            flags |= ULD_FWRITE_FLAGS_LJUST;
            goto next_flag_mod;
            break;

        case 'l':
            if (flags & ULD_FWRITE_FLAGS_LONG) {
                swbkpt();
            }
            flags |= ULD_FWRITE_FLAGS_LONG;
            goto next_flag_mod;
            break;

        case '0':
            if (!width) {
                flags |= ULD_FWRITE_FLAGS_ZERO_PAD;
            } else {
                width *= 10;
            }
            goto next_flag_mod;
            break;

        default: {
            if (*ptr >= '1' && *ptr <= '9') {
                width *= 10;
                width += *ptr - '0';
                goto next_flag_mod;
            }
            break;
        }
        }

        switch (*ptr) {
        case '-':

        case '%':
            stream->dputchar_f(stream->fd, '%');
            count++;
            break;

        case 'p': {
            void *_p = va_arg(ap, void *);
            count += _uld_fwrite_hex(stream, (uint32_t)((uintptr_t)(_p)),
                    sizeof(void *) * 2, ULD_FWRITE_FLAGS_ZERO_PAD);
            break;
        }

        case 'd':
            flags |= ULD_FWRITE_FLAGS_SIGNED;
        case 'u': {
            unsigned int _u = va_arg(ap, unsigned int);
            count += _uld_fwrite_dec(stream, _u, width, flags);
            break;
        }

        case 's': {
            char * _s = va_arg(ap, char *);
            count += _uld_fwrite_str(stream, _s, width, flags);
            break;
        }

        case 'X':
        case 'x': {
            unsigned int _u = va_arg(ap, unsigned int);
            count += _uld_fwrite_hex(stream, _u, width, flags);
            break;
        }

        default: {
            swbkpt();
            break;
        }
        }

        if (!*ptr++) {
            break;
        }
    }

done:
    return count;
}
__export int vfprintf(FILE *stream, const char *format, va_list ap)
        __weak __alias("_uld_vfprintf");

int _uld_printf(const char *format, ...)
{
    va_list ap;
    int ret;
    va_start(ap, format);
    ret = _uld_vfprintf(stdout, format, ap);
    va_end(ap);
    return ret;
}
__export int printf(const char *format, ...) __weak __alias("_uld_printf");

int _uld_fprintf(FILE *stream, const char *format, ...)
{
    va_list ap;
    int ret;
    va_start(ap, format);
    ret = _uld_vfprintf(stream, format, ap);
    va_end(ap);
    return ret;
}
__export int fprintf(FILE *stream, const char *format, ...) __weak
        __alias("_uld_fprintf");

int _uld_dprintf(int fd, const char *format, ...)
{
    va_list ap;
    int ret;
    va_start(ap, format);
    ret = _uld_vdprintf(fd, format, ap);
    va_end(ap);
    return ret;
}
__export int dprintf(int fd, const char *format, ...) __weak
        __alias("_uld_dprintf");

int _uld_vprintf(const char *format, va_list ap)
{
    return _uld_vfprintf(stdout, format, ap);
}
__export int vprintf(const char *format, va_list ap) __weak
        __alias("_uld_vprintf");

int _uld_vdprintf(int fd, const char *format, va_list ap)
{
    FILE stream;
    _uld_memcpy(&stream, stdout, sizeof(FILE));
    stream.fd = fd;
    return _uld_vfprintf(&stream, format, ap);
}
__export int vdprintf(int fd, const char *format, va_list ap) __weak
        __alias("_uld_vdprintf");
