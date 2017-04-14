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

#ifndef _ASM_CPU_H
#define _ASM_CPU_H


#define ARM_THUMB_BRANCH_ADDR(x)                    ((x) | 0x1)
#define ARM_CORTEX_M_CORE_EXC_NUM                   15

#define CONFIG_SRAM_SIZE                            20
#define CONFIG_SRAM_BASE_ADDR                       0x20000000
#define CONFIG_FLASH_SIZE                           128
#define CONFIG_FLASH_BASE_ADDR                      0x08000000

#define CONFIG_CPU_IRQ_NUM                          50
#define CONFIG_CPU_BOOTRAM_ADDR                     0xf108f85f
#ifdef QEMU
#define CONFIG_CPU_VEC_IN_MEM                       1
#endif  // QEMU

// VEC_NUM uses +2 for initial SP value and rootram addr.
#define CPU_VEC_NUM \
    (ARM_CORTEX_M_CORE_EXC_NUM + CONFIG_CPU_IRQ_NUM + 2)
#define CPU_VEC_SIZE                                (CPU_VEC_NUM * 4)
#define CPU_VEC_BASE                                CONFIG_FLASH_BASE_ADDR

#define CPU_SWVEC_ALIGN                             4
#define CPU_SWVEC_BASE \
    ALIGN((CPU_VEC_BASE + CPU_VEC_SIZE), CPU_SWVEC_ALIGN)
#define CPU_SWVEC_ADDR(x) \
    ARM_THUMB_BRANCH_ADDR(CPU_SWVEC_BASE + ((x) * 8))

#define CPU_SWVEC_COREDUMP                          0
#define CPU_SWVEC_COREDUMP_ADDR \
    CPU_SWVEC_ADDR(CPU_SWVEC_COREDUMP)

// memory vector format:
// ldr pc, [pc]
// .word addr
// insn performs a BXWritePC and bit 0 of addr must be set.
// See DDI0403E A2.3.1.
// binary for insn: ldr pc, [pc]
#define CPU_MEMVEC_JMP_INSN                         0xf000f8df


#endif  // _ASM_CPU_H
