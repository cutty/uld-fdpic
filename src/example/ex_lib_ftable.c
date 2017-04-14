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

#include "uld.h"
#include "cpu.h"

#include "ex_libs.h"


static int ex_ftable_func_a(int x);
static int ex_ftable_func_b(int x);

struct ex_ftable ex_ftable __export = {
    .ptr_a = ex_ftable_func_a,
    .ptr_b = ex_ftable_func_b,
    .ptr_c = ex_ftable_func_c
};

struct ex_ftable *ex_ftable_ptr __export = &ex_ftable;


__ctor static void ex_ftable_ctor(void)
{
    uint32_t pc = cpu_get_pc();
    uint32_t sp = cpu_get_sp();
    uint32_t fb = cpu_get_fb();
    printf("** constructor %s called **\n", __func__);
    printf("registers pc: 0x%08lx sp: 0x%08lx fb: 0x%08lx\n", pc, sp, fb);
    printf("[<%p>] ex_ftable .ptr_a: 0x%p  .ptr_b 0x%p  .ptr_c 0x%p\n",
        &ex_ftable,
        *(void **)ex_ftable.ptr_a,
        *(void **)ex_ftable.ptr_b,
        *(void **)ex_ftable.ptr_c);
}

static int ex_ftable_func_a(int x)
{
    printf("ex_ftable_func_a called with x: %d\n", x);
    return 0;
}

int ex_ftable_func_b(int x)
{
    printf("ex_ftable_func_b called with x: %d\n", x);
    return 0;
}

__export int ex_ftable_func_c(int x)
{
    printf("ex_ftable_func_c called with x: %d\n", x);
    return 0;
}

__export int ex_ftable_change_funcs(void)
{
    static int ex_ftable_state;

    switch (ex_ftable_state) {
    case 0:
        ex_ftable.ptr_a = ex_ftable_func_b;
        ex_ftable.ptr_b = ex_ftable_func_c;
        ex_ftable.ptr_c = ex_ftable_func_a;
        ex_ftable_state = 1;
        break;

    case 1:
        ex_ftable_ptr->ptr_a = ex_ftable_func_c;
        ex_ftable_ptr->ptr_b = ex_ftable_func_a;
        ex_ftable_ptr->ptr_c = ex_ftable_func_b;
        ex_ftable_state = 2;
        break;

    case 2:
        ex_ftable_ptr->ptr_a = ex_ftable_func_a;
        ex_ftable_ptr->ptr_b = ex_ftable_func_a;
        ex_ftable_ptr->ptr_c = ex_ftable_func_a;
        ex_ftable_state = 3;
        break;

    default:
        ex_ftable.ptr_a = ex_ftable_func_c;
        ex_ftable.ptr_b = ex_ftable_func_c;
        ex_ftable.ptr_c = ex_ftable_func_c;
        ex_ftable_state = 0;
        break;
    }

    printf("ex_ftable_change_funcs switching table to state %d\n",
            ex_ftable_state);

    return 0;
}
