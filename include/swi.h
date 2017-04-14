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

#ifndef _SWI_H
#define _SWI_H


#include "uld.h"


#define SWI_EOF                                     EOF

#define SWI_STDIN_FILENO                            0
#define SWI_STDOUT_FILENO                           1
#define SWI_STDERR_FILENO                           2

extern FILE _SWI_STDIN;
extern FILE _SWI_STDOUT;
extern FILE _SWI_STDERR;
#define SWI_STDIN  (&_SWI_STDIN)
#define SWI_STDOUT (&_SWI_STDOUT)
#define SWI_STDERR (&_SWI_STDERR)

// return bytes written on success, or SWI_EOF on error.
ssize_t swi_write(int fd, const void *buf, size_t count) __nonnull;

// return c on success, SWI_EOF on error.
int swi_dputchar(int fd, int c);

// number of centiseconds since support code execution.
int32_t swi_clock(void);

// number of seconds since 00:00 Jan 1, 1970.
time_t swi_time(void);

// semihosting exit request (will exit qemu).
void swi_exit(void) __noreturn;

#define swi_fputc(c, stream)    _uld_fputc((c), (stream))
#define swi_fputs(s, stream)    _uld_fputs((s), (stream))
#define swi_putc(c, stream)     _uld_putc((c), (stream))
#define swi_putchar(c)          _uld_putc((c), SWI_STDOUT)
#define swi_puts(s) \
    do { \
        _uld_fputs((s), SWI_STDOUT); \
        _uld_putc('\n', SWI_STDOUT); \
    } while (0)

#define swi_fwrite(ptr, size, nmemb, stream) \
    _uld_fwrite((ptr), (size), (nmemb), (stream))

#define swi_vfprintf(stream, format, ap) \
    _uld_vfprintf((stream), (format), (ap))

int swi_printf(const char *format, ...) __format(printf, 1, 2);
#define swi_fprintf(stream, format, ...) \
    _uld_fprintf((stream), (format), __VA_ARGS__)
int swi_dprintf(int fd, const char *format, ...) __format(printf, 2, 3);

#define swi_vprintf(format, ap) \
    _uld_vfprintf(SWI_STDOUT, (format), (ap))
int swi_vdprintf(int fd, const char *format, va_list ap);


#endif  // _SWI_H
