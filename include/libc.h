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

#ifndef _LIBC_H
#define _LIBC_H


#define EOF                                         (-1)


typedef struct FILE {
    ssize_t (*write_f)(int fd, const void *buf, size_t count);
    int (*dputchar_f)(int fd, int c);
    void *data;
    int fd;
} FILE;


// string.h
void *_uld_memcpy(void *dest, const void *src, size_t n);
void *_uld_memmove(void *dest, const void *src, size_t n);
void *_uld_memset(void *s, int c, size_t n);
int _uld_strcmp(const char *s1, const char *s2);
char *_uld_strcpy(char *dest, const char *src);
size_t _uld_strlen(const char *s);
size_t _uld_strnlen(const char *s, size_t maxlen);

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);


// stdio.h
ssize_t _uld_write(int fd, const void *buf, size_t count);

int _uld_fputc(int c, FILE *stream);
int _uld_fputs(const char *s, FILE *stream);
int _uld_putc(int c, FILE *stream);
int _uld_putchar(int c);
int _uld_puts(const char *s);

size_t _uld_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

int _uld_vfprintf(FILE *stream, const char *format, va_list ap);

int _uld_printf(const char *format, ...) __format(printf, 1, 2);
int _uld_fprintf(FILE *stream, const char *format, ...) __format(printf, 2, 3);
int _uld_dprintf(int fd, const char *format, ...) __format(printf, 2, 3);

int _uld_vprintf(const char *format, va_list ap);
int _uld_vdprintf(int fd, const char *format, va_list ap);

ssize_t write(int fd, const void *buf, size_t count);

int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
int putc(int c, FILE *stream);
int putchar(int c);
int puts(const char *s);

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

int vfprintf(FILE *stream, const char *format, va_list ap);

int printf(const char *format, ...) __format(printf, 1, 2);
int fprintf(FILE *stream, const char *format, ...) __format(printf, 2, 3);
int dprintf(int fd, const char *format, ...) __format(printf, 2, 3);

int vprintf(const char *format, va_list ap);
int vdprintf(int fd, const char *format, va_list ap);


#define CONFIG_STDIO_DEFAULT_SWI
#ifdef CONFIG_STDIO_DEFAULT_SWI
#include "swi.h"
#define stdin  SWI_STDIN
#define stdout SWI_STDOUT
#define stderr SWI_STDERR
#else
#error "No stdio default defined"
#endif


#endif  // _LIBC_H
