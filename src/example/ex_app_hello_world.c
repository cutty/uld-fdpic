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


int main(int argc, char **argv)
{
    uint32_t pc = cpu_get_pc();
    uint32_t sp = cpu_get_sp();
    uint32_t fb = cpu_get_fb();
    int i;

    printf("hello world!\n");

#ifdef HELLO_WORLD_DYNAMIC
    printf("this is built using dynamic linking\n");
#endif  // HELLO_WORLD_DYNAMIC

    printf("started with args:");
    if (argc) {
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
    } else {
        printf(" <none>");
    }
    printf("\nregisters pc: 0x%08lx sp: 0x%08lx fb: 0x%08lx\n", pc, sp, fb);
    // Printing function and function pointers in FDPIC will actually give
    // you the FUNCDESC_VALUE location instead of the function.  Use this
    // to get the actual location of the function.
    printf("main is located at: 0x%p\n", *(void **)main);

    swbkpt();
    while (1);
}
