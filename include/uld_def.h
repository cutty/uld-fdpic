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

#ifndef _ULD_DEF_H
#define _ULD_DEF_H


typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

#ifndef va_list
#define va_list __builtin_va_list
#endif
#ifndef va_start
#define va_start(ap, last) __builtin_va_start((ap), (last))
#endif
#ifndef va_arg
#define va_arg(ap, type) __builtin_va_arg((ap), type)
#endif
#ifndef va_end
#define va_end(ap) __builtin_va_end((ap))
#endif


#define __ALIGN_MASK(bits) ((1 << (bits)) - 1)
#define ALIGN(val, bits) (val + __ALIGN_MASK((typeof(val))(bits))) & \
    ~(__ALIGN_MASK((typeof(val))(bits)))
#define ALIGN_PTR(ptr, bits) (typeof(ptr))(ALIGN(((uintptr_t)ptr), (bits)))


#ifdef __MIN
#undef __MIN
#endif
#define __MIN(a, b) ({ \
        typeof(a) _a = (a); \
        typeof(b) _b = (b); \
        _a < _b ? _a : _b; \
    })
#ifdef MIN
#undef MIN
#endif
#define MIN(a, b) __MIN(a, b)
#ifdef __MAX
#undef __MAX
#endif
#define __MAX(a, b) ({ \
        typeof(a) _a = (a); \
        typeof(b) _b = (b); \
        _a > _b ? _a : _b; \
    })
#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) __MAX(a, b)


#ifdef inline
#undef inline
#endif
#define inline inline
#ifdef __inline
#undef __inline
#endif
#define __inline inline


#endif  // _ULD_DEF_H
