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

.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

#include "asm/cpu.h"
#include "asm/asm.h"


    .section .text.vector_reset, "ax", %progbits
    ALIGN(2)
    .global vector_reset
    .weak vector_reset
    .type vector_reset, %function
vector_reset:
#ifdef CONFIG_CPU_VEC_IN_MEM
    @ If vec table is in ram init first.
    ldr r0, =mem_vector_table
    ldr r1, =_e_mem_vector_table
    ldr r2, =#CPU_MEMVEC_JMP_INSN
    ldr r3, =__vector_unhandled
    b .Lmem_vector_table_loop
.Lmem_vector_table_write:
    str r2, [r0], #4
    str r3, [r0], #4
.Lmem_vector_table_loop:
    cmp r0, r1
    bcc .Lmem_vector_table_write
#endif

    @ Copy .data section into ram.
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
    b .Lcopy_data_loop
.Lcopy_data_write:
    ldr r3, [r2], #4
    str r3, [r0], #4
.Lcopy_data_loop:
    cmp r0, r1
    bcc .Lcopy_data_write

    @ Zero out .bss section.
    ldr r0, =_sbss
    ldr r1, =_ebss
    mov r2, #0
    b .Lzero_bss_loop
.Lzero_bss_write:
    str r2, [r0], #4
.Lzero_bss_loop:
    cmp r0, r1
    bcc .Lzero_bss_write

    @ Finish libc and system init in C.
    bl uld_start_init

    @ Run uld_main.
    bl uld_main
vector_reset_halt:
    b vector_reset_halt
SIZE(vector_reset)


    .section .uld_rt.rt_tbl_ptr, "aw", %nobits
    ALIGN(2)
    .global _rt_tbl_ptr
    .type _rt_tbl_ptr, %object
_rt_tbl_ptr:
    .space 4
SIZE(_rt_tbl_ptr)
