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


    .section .text.uld_exec_call_vv_fp_fdpic_base, "ax", %progbits
    ALIGN(2)
    .global uld_exec_call_vv_fp_fdpic_base
    .type uld_exec_call_vv_fp_fdpic_base, %function
uld_exec_call_vv_fp_fdpic_base:
    push {r9, lr}
    mov r9, r1
    blx r0
    pop {r9, pc}
SIZE(uld_exec_call_vv_fp_fdpic_base)

@int uld_exec_elf_call_entry(void *entry, void *sp_base, int argc,
@        const char **argv, uint32_t fdpic_base);
@  r0       : entry
@  r1       : sp_base
@  r2       : argc
@  r3       : argv
@  [sp,#0]  : fdpic_base
    .section .text.uld_exec_elf_call_entry, "ax", %progbits
    ALIGN(2)
    .global uld_exec_elf_call_entry
    .type uld_exec_elf_call_entry, %function
uld_exec_elf_call_entry:
   .cfi_startproc
    push {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
   .cfi_def_cfa_offset 40
   .cfi_offset 4, -40
   .cfi_offset 5, -36
   .cfi_offset 6, -32
   .cfi_offset 7, -28
   .cfi_offset 8, -24
   .cfi_offset 9, -20
   .cfi_offset 10, -16
   .cfi_offset 11, -12
   .cfi_offset 12, -8
   .cfi_offset 14, -4
    sub sp, sp, #0              @ alloc local storage
   .cfi_def_cfa_offset 40

    mov r4, r0                  @ entry
    mov r5, r1                  @ sp_base
    mov r6, r2                  @ argc
    mov r7, sp                  @ start of local storage (and stack cleanup)
                                @ and ptr to end of target argv
    add r2, r2, #1              @ argc + 1 to for argv null termination.
    add r8, r3, r2, lsl #2      @ ptr to end of caller argv.  Not read from
                                @ caller argv but allows same offset for
                                @ target argv

    add r2, r2, #1              @ (argc + 1) & ~1 (stack double word aligned)
    bic r2, r2, #1
    sub sp, sp, r2, lsl #2      @ alloc space for target argv
    mov r9, #-4                 @ start argv decreasing offset from end
    mov r0, #0                  @ null terminate target argv[argc]
    str r0, [r7, r9]

    cmp r5, #0                  @ r11 = r5 == 0 : 0 ? r5 - r7
    ite eq                      @ r11: argv offset
    moveq r11, #0               @ r5: sp_base
    subne r11, r5, r7           @ r7: end of target argv

    add r10, r6, #1
    b .Lcopy_argv_loop
.Lcopy_argv_write:
    sub r9, #4
    ldr r0, [r8, r9]
    blx strlen

    add r0, r0, #8              @ add 1 to len for null byte and
    bic r0, r0, #7              @ double word align (down)
    sub sp, sp, r0              @ alloc space for next str

    add r0, sp, r11             @ apply offset and save argv
    str r0, [r7, r9]

    mov r0, sp
    ldr r1, [r8, r9]
    blx strcpy

.Lcopy_argv_loop:
    subs r10, r10, #1
    bne .Lcopy_argv_write

#ifdef ULD_BREAK_BEFORE_STACK_RESET
    bkpt #1
#endif

    ldr r9, [r7, #40]           @ read fdpic_base from stack args before
                                @ possible stack reset

    mov r0, r11                 @ skip stack reset if no argv offset
    cbz r0, .Lno_stack_reset
    add r0, sp, r11             @ dest: sp + offset
    mov r1, sp                  @ src: sp
    sub r2, r7, r1              @ size: target argv end - sp (in r1)
    blx memmove
    add r7, r7, r11             @ add offset to target argv end
                                @ used to calculate argv below
    add r0, sp, r11             @ set new stack
    bic r0, r0, #7              @ double word align down
    mov sp, r0
.Lno_stack_reset:

    add r0, r6, #1              @ argv = target argv end - ((argc + 1) * 4)
    sub r1, r7, r0, lsl #2
    mov r0, r6                  @ argc

    blx r4

    add r7, r7, #0              @ free local storage
    mov sp, r7
    pop {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}
   .cfi_endproc
SIZE(uld_exec_elf_call_entry)
