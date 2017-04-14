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

#ifndef _ULD_EXEC_H
#define _ULD_EXEC_H


#include "uld.h"


void uld_exec_call_vv_fp_array(void (**arr_start)(void),
        void (**arr_end)(void));

void uld_exec_call_vv_fp_array_fdpic_base(void (**arr_start)(void),
        void (**arr_end)(void), uint32_t fdpic_base);

int uld_exec_elf_call_init_funcs(struct uld_file *ufile);

int uld_exec_file(const struct uld_file *ufile, void *sp_base, int argc,
        const char **argv);

// Functions in uld_exec_asm.S
void uld_exec_call_vv_fp_fdpic_base(void (*fp)(void), uint32_t fdpic_base);

// Note: argv and contents will be copied to sp_base before stack reset.
// If this overwrites the working stack it may cause unpredictable
// results.
int uld_exec_elf_call_entry(const void *entry, void *sp_base, int argc,
        const char **argv, uint32_t fdpic_base);


#endif  // _ULD_EXEC_H
