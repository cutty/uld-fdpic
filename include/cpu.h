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

#ifndef _CPU_H
#define _CPU_H


#include "asm/cpu.h"


#define CPU_CALL_ABS(x) \
    do { \
        ((void (*)(void))ARM_THUMB_BRANCH_ADDR((x)))(); \
    } while (0)

// AAPCS callee must preserve r4-r8, r10, r11, sp.
// FDPIC requires caller to preserve r9.
// See ARM IHI0042F 5.1.1.
#define CPU_CALL_ABS_ASM(x) \
    do { \
        asm volatile ("blx %0" \
                : \
                : "r" (ARM_THUMB_BRANCH_ADDR((x))) \
                : "r0", "r1", "r2", "r3", "r9", "ip", \
                "lr", "memory", "cc"); \
    } while (0)

static __inline __always_inline __notrace uint32_t cpu_get_fb(void)
{
    uint32_t fd;
    asm volatile("mov %0, r9"
            : "=r" (fd)
            :
            :);
    return fd;
}

static __inline __always_inline __notrace uint32_t cpu_get_sp(void)
{
    uint32_t sp;
    asm volatile("mov %0, sp"
            : "=r" (sp)
            :
            :);
    return sp;
}

// This needs to be inline or it will just return the pc of this function.
static __inline __always_inline __notrace uint32_t cpu_get_pc(void)
{
    uint32_t pc;
    asm volatile("mov %0, pc"
            : "=r" (pc)
            :
            :);
    return pc;
}

void cpu_reset_clks(void);
void cpu_init_clks(void);

int cpu_flash_is_locked(void);
int cpu_flash_unlock(void);
int cpu_flash_lock(void);
int cpu_flash_erase(void *s, size_t n);
int cpu_flash_write(void *dest, const void *src, size_t n);


#endif  // _CPU_H
