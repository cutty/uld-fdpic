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


#ifdef CONFIG_CPU_VEC_IN_MEM
    .section .uld_rt.mem_vectors, "awx", %nobits
    ALIGN(2)
    .global mem_vector_table
    .type mem_vector_table, %object
mem_vector_table:
    .space (ARM_CORTEX_M_CORE_EXC_NUM - 1 + CONFIG_CPU_IRQ_NUM) * 8
SIZE(mem_vector_table)
#endif

    .section .vectors, "a", %progbits
    ALIGN(2)
    .global vector_table
    .type vector_table, %object
vector_table:
    .word _estack
    .word vector_reset
#ifdef CONFIG_CPU_VEC_IN_MEM
    vtor_to_mem mem_vector_table, ARM_CORTEX_M_CORE_EXC_NUM - 1 \
        + CONFIG_CPU_IRQ_NUM
#else
    def_vec vector_nmi
    def_vec vector_hardfault
    def_vec vector_memmanage
    def_vec vector_busfault
    def_vec vector_usagefault
    .word 0
    .word 0
    .word 0
    .word 0
    def_vec vector_svc
    def_vec vector_debugmon
    .word 0
    def_vec vector_pendsv
    def_vec vector_systick
    def_vec vector_wwdg
    def_vec vector_pvd
    def_vec vector_tamper
    def_vec vector_rtc
    def_vec vector_flash
    def_vec vector_rcc
    def_vec vector_exti0
    def_vec vector_exti1
    def_vec vector_exti2
    def_vec vector_exti3
    def_vec vector_exti4
    def_vec vector_dma1_channel1
    def_vec vector_dma1_channel2
    def_vec vector_dma1_channel3
    def_vec vector_dma1_channel4
    def_vec vector_dma1_channel5
    def_vec vector_dma1_channel6
    def_vec vector_dma1_channel7
    def_vec vector_adc1_2
    def_vec vector_usb_hp_can1_tx
    def_vec vector_usb_lp_can1_rx0
    def_vec vector_can1_rx1
    def_vec vector_can1_sce
    def_vec vector_exti9_5
    def_vec vector_tim1_brk
    def_vec vector_tim1_up
    def_vec vector_tim1_trg_com
    def_vec vector_tim1_cc
    def_vec vector_tim2
    def_vec vector_tim3
    def_vec vector_tim4
    def_vec vector_i2c1_ev
    def_vec vector_i2c1_er
    def_vec vector_i2c2_ev
    def_vec vector_i2c2_er
    def_vec vector_spi1
    def_vec vector_spi2
    def_vec vector_usart1
    def_vec vector_usart2
    def_vec vector_usart3
    def_vec vector_exti15_10
    def_vec vector_rtc_alarm
    def_vec vector_usbwakeup
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
#endif
    .word CONFIG_CPU_BOOTRAM_ADDR
SIZE(vector_table)

    .section .sw_vectors, "ax", %progbits
    ALIGN(2)
    .global sw_vector_coredump
    .type sw_vector_coredump, %function
sw_vector_coredump:
    ldr pc, [pc]
    def_vec coredump_handler
SIZE(sw_vector_coredump)

    .section .text.__vector_unhandled, "ax", %progbits
    ALIGN(2)
    .global __vector_unhandled
    .weak __vector_unhandled
    .type __vector_unhandled, %function
__vector_unhandled:
    b __vector_unhandled
SIZE(__vector_unhandled)
