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

#include <alloca.h>

#include "uld.h"
#include "cpu.h"

#include "ex_libs.h"


// These have hidden visibility.
int ex_garage_call_count_get;
int ex_garage_call_count_set = 2;
static int ex_garage_call_count_inc_dec;
static int ex_garage_call_count_print = 1;

int ex_garage_car_count __export = 2;
int ex_garage_truck_count __export;
const char *ex_garage_name __export = "Squeaky's garage";
void (*mystery_func)(void) __export = ex_print_cow;


__ctor static void ex_garage_ctor_1(void)
{
    uint32_t pc = cpu_get_pc();
    uint32_t sp = cpu_get_sp();
    uint32_t fb = cpu_get_fb();
    printf("** constructor %s called **\n", __func__);
    printf("registers pc: 0x%08lx sp: 0x%08lx fb: 0x%08lx\n", pc, sp, fb);
    printf("stdin: %p stdout: %p stderr: %p\n", stdin, stdout, stderr);
    printf("write: %p puts: %p printf: %p\n", *(void **)write,
            *(void **)puts, *(void **)printf);
    ex_garage_print_call_counts();
}

__ctor static void ex_garage_ctor_2(void)
{
    uint32_t pc = cpu_get_pc();
    uint32_t sp = cpu_get_sp();
    uint32_t fb = cpu_get_fb();
    printf("** constructor %s called **\n", __func__);
    printf("registers pc: 0x%08lx sp: 0x%08lx fb: 0x%08lx\n", pc, sp, fb);
    printf("ex_print_dl_alloc_* [<%p>] [<%p>] [<%p>]\n",
            &ex_print_dl_alloc_a, &ex_print_dl_alloc_b,
            &ex_print_dl_alloc_c);
}

__export void ex_garage_set_cars(int cars)
{
    int *car_count = &ex_garage_car_count;
    int *call_count = &ex_garage_call_count_set;
    (*call_count)++;
    *car_count = cars;
}

__export int ex_garage_get_cars(void)
{
    int *car_count = &ex_garage_car_count;
    int *call_count = &ex_garage_call_count_get;
    (*call_count)++;
    return *car_count;
}

__export void ex_garage_inc_car(void)
{
    int *call_count = &ex_garage_call_count_inc_dec;
    void (*set)(int) = ex_garage_set_cars;
    int (*get)(void) = ex_garage_get_cars;
    (*call_count)++;
    set(get() + 1);
}

__export void ex_garage_dec_car(void)
{
    int *call_count = &ex_garage_call_count_inc_dec;
    (*call_count)++;
    ex_garage_set_cars(ex_garage_get_cars() - 1);
}

__export void ex_garage_set_trucks(int trucks)
{
    ex_garage_call_count_set++;
    ex_garage_truck_count = trucks;
}

__export int ex_garage_get_trucks(void)
{
    ex_garage_call_count_get++;
    return ex_garage_truck_count;
}

__export void ex_garage_inc_truck(void)
{
    ex_garage_call_count_inc_dec++;
    void (*set)(int) = ex_garage_set_trucks;
    int (*get)(void) = ex_garage_get_trucks;
    set(get() + 1);
}

__export void ex_garage_dec_truck(void)
{
    ex_garage_call_count_inc_dec++;
    ex_garage_set_trucks(ex_garage_get_trucks() - 1);
}

__export int ex_garage_get_vehicles(void)
{
    int *truck_count = &ex_garage_truck_count;
    int *call_count = &ex_garage_call_count_get;
    ex_garage_call_count_get = *call_count + 1;
    return ex_garage_car_count + *truck_count;
}

__export void ex_garage_print_car_count(void)
{
    int *call_count = &ex_garage_call_count_print;
    int (*get)(void) = ex_garage_get_cars;
    int (*print_upper)(const char *) = ex_print_upper;
    (*call_count)++;
    print_upper("CAR count: ");
    printf("%d cars\n", get());
}

__export void ex_garage_print_truck_count(void)
{
    ex_garage_call_count_print++;
    ex_print_lower("TRUCK count: ");
    printf("%d trucks\n", ex_garage_get_trucks());
}

__export void ex_garage_print_vehicle_count(void)
{
    int *call_count = &ex_garage_call_count_print;
    ex_garage_call_count_print = *call_count + 1;
    printf("total count: %d vehicles\n", ex_garage_get_vehicles());
}

__export void ex_garage_print_call_counts(void)
{
    int *call_count = &ex_garage_call_count_print;
    int *set_count = &ex_garage_call_count_set;
    int *inc_dec_count = &ex_garage_call_count_inc_dec;
    *call_count = ex_garage_call_count_print + 1;
    printf("call count - get: %d set: %d inc/dec: %d print: %d\n",
        ex_garage_call_count_get, *set_count, *inc_dec_count,
        ex_garage_call_count_print);
}

__export void ex_garage_print_banner(void)
{
    static const char welcome[] = "Welcome to ";
    char *banner;
    char *ptr;
    size_t len;
    int *call_count = &ex_garage_call_count_print;

    *call_count = ex_garage_call_count_print + 1;

    len = strlen(ex_garage_name);
    // welcome contains \0
    banner = alloca(sizeof(welcome) + len);
    ptr = banner;
    strcpy(ptr, welcome);
    ptr += sizeof(welcome) - 1;
    strcpy(ptr, ex_garage_name);
    ptr += len;
    *ptr = '\0';

    ex_print_box_text(banner);
}
