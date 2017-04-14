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


static uint8_t cpu_flash_lock_state = 1;

void cpu_reset_clks(void)
{
}

void cpu_init_clks(void)
{
}

int cpu_flash_is_locked(void)
{
    return cpu_flash_lock_state;
}

int cpu_flash_unlock(void)
{
    if (cpu_flash_lock_state) {
        puts("Unlocking flash");
        cpu_flash_lock_state = 0;
    }
    return 0;
}

int cpu_flash_lock(void)
{
    if (!cpu_flash_lock_state) {
        puts("Locking flash");
        cpu_flash_lock_state = 1;
    }
    return 0;
}

int cpu_flash_erase(void *s, size_t n)
{
    if (cpu_flash_lock_state) {
        return -1;
    }
    printf("Erasing flash: 0x%p - 0x%p\n", s, s + n);
    memset(s, 0, n);
    return 0;
}

int cpu_flash_write(void *dest, const void *src, size_t n)
{
    if (cpu_flash_lock_state) {
        return -1;
    }
    printf("Writing to flash: 0x%p - 0x%p\n", dest, dest + n);
    memmove(dest, src, n);
    return 0;
}
