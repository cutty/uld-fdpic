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

#ifndef _ULD_H
#define _ULD_H


#include "uld_def.h"

#ifdef __ULD__
#include "uld_types.h"
#else  // __ULD__
#include <sys/types.h>
#endif  // __ULD__

#include "compiler.h"
#include "libc.h"
#include "debug.h"

#ifdef __ULD__
#include "uld_print.h"

// Linker defined symbols.
extern uint8_t _estack;
extern struct uld_pstore _uld_pstore;
#define ESTACK (&_estack)
#define ULD_PSTORE (&_uld_pstore)

// uld_init.c
extern int uld_verbose;

void uld_start_init(void);
int uld_main(void);
#else  // __ULD__
#endif  // __ULD__


#endif  // _ULD_H
