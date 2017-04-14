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


    .section .vectors, "ax", %progbits
    ALIGN(2)
    .global vector_table
    .type vector_table, %object
vector_table:
    app_def_vec vector_nmi
    app_def_vec vector_hardfault
    app_def_vec vector_memmanage
    app_def_vec vector_busfault
    app_def_vec vector_usagefault
    .word 0
    .word 0
    .word 0
    .word 0
    app_def_vec vector_svc
    app_def_vec vector_debugmon
    .word 0
    app_def_vec vector_pendsv
    app_def_vec vector_systick
    app_def_vec vector_wwdg
    app_def_vec vector_pvd
    app_def_vec vector_tamper
    app_def_vec vector_rtc
    app_def_vec vector_flash
    app_def_vec vector_rcc
    app_def_vec vector_exti0
    app_def_vec vector_exti1
    app_def_vec vector_exti2
    app_def_vec vector_exti3
    app_def_vec vector_exti4
    app_def_vec vector_dma1_channel1
    app_def_vec vector_dma1_channel2
    app_def_vec vector_dma1_channel3
    app_def_vec vector_dma1_channel4
    app_def_vec vector_dma1_channel5
    app_def_vec vector_dma1_channel6
    app_def_vec vector_dma1_channel7
    app_def_vec vector_adc1_2
    app_def_vec vector_usb_hp_can1_tx
    app_def_vec vector_usb_lp_can1_rx0
    app_def_vec vector_can1_rx1
    app_def_vec vector_can1_sce
    app_def_vec vector_exti9_5
    app_def_vec vector_tim1_brk
    app_def_vec vector_tim1_up
    app_def_vec vector_tim1_trg_com
    app_def_vec vector_tim1_cc
    app_def_vec vector_tim2
    app_def_vec vector_tim3
    app_def_vec vector_tim4
    app_def_vec vector_i2c1_ev
    app_def_vec vector_i2c1_er
    app_def_vec vector_i2c2_ev
    app_def_vec vector_i2c2_er
    app_def_vec vector_spi1
    app_def_vec vector_spi2
    app_def_vec vector_usart1
    app_def_vec vector_usart2
    app_def_vec vector_usart3
    app_def_vec vector_exti15_10
    app_def_vec vector_rtc_alarm
    app_def_vec vector_usbwakeup
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
SIZE(vector_table)

    .section .text.__vector_unhandled, "ax", %progbits
    ALIGN(2)
    .global __vector_unhandled
    .weak __vector_unhandled
    .type __vector_unhandled, %function
__vector_unhandled:
    b __vector_unhandled
SIZE(__vector_unhandled)
