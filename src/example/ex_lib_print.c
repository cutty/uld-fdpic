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

#include <alloca.h>

#include "uld.h"

#include "ex_libs.h"


// By not referencing any of these variables the linker will not allocate
// space for them but leave the dynamic symbols.  It is then left up to
// dynamic linker to allocate space.  By referencing any of these variables
// all variables will have space allocated.
// Note: This will prevent gdb from having valid addresses for the variables.
int ex_print_hidden_a;
int ex_print_dl_alloc_a __export;
int ex_print_dl_alloc_b __export;
int ex_print_dl_alloc_c __export;

static const char moo_str[] =
    " ______\n"
    "< moo! >\n"
    " ------\n"
    "        \\   ^__^\n"
    "         \\  (oo)\\_______\n"
    "            (__)\\       )\\/\\\n"
    "                ||----w |\n"
    "                ||     ||\n";


__export int ex_print_nputchar(int c, size_t n)
{
    size_t r;
    char chars[16];

    if (n <= 0) {
        return 0;
    }

    memset(chars, c, sizeof(chars));

    while (n) {
        r = fwrite(chars, 1, __MIN(n, sizeof(chars)), stdout);
        n -= r;
    }

    return 0;
}

static void ex_print_lmr(int left, int middle, int right, size_t n)
{
    putc(left, stdout);
    ex_print_nputchar(middle, n);
    putc(right, stdout);
    putc('\n', stdout);
}

static void ex_print_box_top(int x)
{
    ex_print_lmr('/', '-', '\\', x);
}

static void ex_print_box_bottom(int x)
{
    ex_print_lmr('\\', '-', '/', x);
}

__export int ex_print_box(int x, int y)
{
    x = __MAX(x, 2) - 2;
    y = __MAX(y, 2) - 2;

    ex_print_box_top(x);
    while (y--) {
        putc('|', stdout);
        ex_print_nputchar(' ', x);
        putc('|', stdout);
        putc('\n', stdout);
    }
    ex_print_box_bottom(x);

    return 0;
}

__export int ex_print_box_text(const char *s)
{
    int x;

    if (!s) {
        return -1;
    }

    x = strlen(s);

    ex_print_box_top(x);
    printf("|%s|\n", s);
    ex_print_box_bottom(x);

    return 0;
}

static int ex_print_case(const char *s, int toupper)
{
    char *out;
    char *ptr;

    if (!s) {
        return -1;
    }

    out = alloca(strlen(s) + 1);
    ptr = out;

    if (toupper) {
        while (*s) {
            if (*s >= 'a' && *s <= 'z') {
                *ptr++ = *s++ -'a' + 'A';
            } else {
                *ptr++ = *s++;
            }
        }
    } else {
        while (*s) {
            if (*s >= 'A' && *s <= 'Z') {
                *ptr++ = *s++ -'A' + 'a';
            } else {
                *ptr++ = *s++;
            }
        }
    }
    *ptr = '\0';
    fputs(out, stdout);

    return 0;
}

__export int ex_print_upper(const char *s)
{
    return ex_print_case(s, 1);
}

__export int ex_print_lower(const char *s)
{
    return ex_print_case(s, 0);
}

__export void ex_print_cow(void)
{
    puts(moo_str);
}
