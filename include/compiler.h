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

#ifndef _COMPILER_H
#define _COMPILER_H


#ifdef __alias
#undef __alias
#endif
#ifndef CONFIG_NO_ATTR_ALIAS
#define __alias(name) __attribute__((alias(name)))
#else
#define __alias
#endif

#ifdef __aligned
#undef __aligned
#endif
#ifndef CONFIG_NO_ATTR_ALIGNED
#define __aligned(alignment) __attribute__((aligned(alignment)))
#else
#define __aligned
#endif

#ifdef __always_inline
#undef __always_inline
#endif
#ifndef CONFIG_NO_ATTR_ALWAYS_INLINE
#define __always_inline __attribute__((always_inline))
#else
#define __always_inline
#endif

#ifdef __ctor
#undef __ctor
#endif
#ifndef CONFIG_NO_ATTR_CTOR
#define __ctor __attribute__((constructor))
#else
#define __ctor
#endif

#ifdef __ctor_p
#undef __ctor_p
#endif
#ifndef CONFIG_NO_ATTR_CTOR
#define __ctor_p(priority) __attribute__((constructor(priority)))
#else
#define __ctor_p
#endif

#ifdef __dtor
#undef __dtor
#endif
#ifndef CONFIG_NO_ATTR_DTOR
#define __dtor __attribute__((destructor))
#else
#define __dtor
#endif

#ifdef __dtor_p
#undef __dtor_p
#endif
#ifndef CONFIG_NO_ATTR_DTOR
#define __dtor_p(priority) __attribute__((destructor(priority)))
#else
#define __dtor_p
#endif

#ifdef __export
#undef __export
#endif
#ifndef CONFIG_NO_ATTR_EXPORT
#define __export __attribute__((visibility("default")))
#else
#define __export
#endif

#ifdef __format
#undef __format
#endif
#ifndef CONFIG_NO_ATTR_FORMAT
#define __format(archetype, str_idx, first_to_check) \
    __attribute__((format (archetype, str_idx, first_to_check)))
#else
#define __format
#endif

#ifdef __hidden
#undef __hidden
#endif
#ifndef CONFIG_NO_ATTR_HIDDEN
#define __hidden __attribute__((visibility("hidden")))
#else
#define __hidden
#endif

#ifdef __interrupt
#undef __interrupt
#endif
#ifndef CONFIG_NO_ATTR_INTERRUPT
#define __interrupt __attribute__((interrupt))
#else
#define __interrupt
#endif

// The interrupt attribute can take an optional argument but it is
// ignored on ARMv7-M.
#ifdef __interrupt_a
#undef __interrupt_a
#endif
#ifndef CONFIG_NO_ATTR_INTERRUPT
#define __interrupt_a(param) __attribute__((interrupt(param)))
#else
#define __interrupt_a
#endif

#ifdef __nonnull
#undef __nonnull
#endif
#ifndef CONFIG_NO_ATTR_NONNULL
#define __nonnull __attribute__((nonnull))
#else
#define __nonnull
#endif

#ifdef __nonnull_a
#undef __nonnull_a
#endif
#ifndef CONFIG_NO_ATTR_NONNULL
#define __nonnull_a(...) __attribute__((nonnull(__VA_ARGS__)))
#else
#define __nonnull_a(...)
#endif

#ifdef __noreturn
#undef __noreturn
#endif
#ifndef CONFIG_NO_ATTR_NORETURN
#define __noreturn __attribute__((noreturn))
#else
#define __noreturn
#endif

#ifdef __notrace
#undef __notrace
#endif
#ifndef CONFIG_NO_ATTR_NOTRACE
#define __notrace __attribute__((no_instrument_function))
#else
#define __notrace
#endif

#ifdef __section
#undef __section
#endif
#ifndef CONFIG_NO_ATTR_SECTION
#define __section(name) __attribute__((section(name)))
#else
#define __section
#endif

#ifdef __transparent_union
#undef __transparent_union
#endif
#ifndef CONFIG_NO_ATTR_TRANSPARENT_UNION
#define __transparent_union __attribute__((transparent_union))
#else
#define __transparent_union
#endif

#ifdef __used
#undef __used
#endif
#ifndef CONFIG_NO_ATTR_USED
#define __used __attribute__((used))
#else
#define __used
#endif

#ifdef __weak
#undef __weak
#endif
#ifndef CONFIG_NO_ATTR_WEAK
#define __weak __attribute__((weak))
#else
#define __weak
#endif


#endif  // _COMPILER_H
