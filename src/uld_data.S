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


    .section .uld_pdata.pstore, "a", %progbits
    ALIGN(2)
    .global _uld_pstore
    .type _uld_pstore, %object
_uld_pstore:
    .word 0x00000000                @ .boot_action
    .word _s_files                  @ .files_base
    .word _files_size               @ .files_size
    .word _fs_table_pri             @ .fs_table_pri.head
    .word _fs_table_pri             @ .fs_table_pri.table_base
    .word _fs_table_size            @ .fs_table_pri.size
.global _uld_pstore__fs_table_pri__crc
_uld_pstore__fs_table_pri__crc:
    .word 0x00000000                @ .fs_table_pri.crc
SIZE(_uld_pstore)

    .section .uld_pstore_ptr.data, "a", %progbits
    ALIGN(2)
    .global _uld_pstore_ptr
    .type _uld_pstore_ptr, %object
_uld_pstore_ptr:
    .word _uld_pstore
SIZE(_uld_pstore_ptr)
