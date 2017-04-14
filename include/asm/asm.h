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

#ifndef _ASM_ASM_H
#define _ASM_ASM_H

#include "asm/cpu.h"


#ifdef __ASSEMBLER__
#ifdef ALIGN
#undef ALIGN
#endif
#define ALIGN(alignment) .align alignment, 0xbf00

#ifdef SIZE
#undef SIZE
#endif
#define SIZE(name) .size name, .-name

    .macro      vtor_to_mem, base, count, idx=0
    .word       \base + (\idx * 8) + 1
    .if         \count > (\idx + 1)
    vtor_to_mem \base, \count, "(\idx + 1)"
    .endif
    .endm

    .macro      def_vec, vec_name, def_name=__vector_unhandled
    .word       \vec_name
    .weak       \vec_name
    .thumb_set  \vec_name, \def_name
    .endm

    .macro      app_def_vec, vec_name, def_name=__vector_unhandled
    b.w \vec_name
    .weak       \vec_name
    .thumb_set  \vec_name, \def_name
    .endm
#endif  // __ASSEMBLER__


#endif  // _ASM_ASM_H
