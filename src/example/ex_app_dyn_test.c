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


__ctor static void ex_dyn_test_ctor(void)
{
    uint32_t pc = cpu_get_pc();
    uint32_t sp = cpu_get_sp();
    uint32_t fb = cpu_get_fb();
    printf("** constructor %s called **\n", __func__);
    printf("registers pc: 0x%08lx sp: 0x%08lx fb: 0x%08lx\n", pc, sp, fb);
    printf("stdin: %p stdout: %p stderr: %p\n", stdin, stdout, stderr);
    printf("write: %p puts: %p printf: %p\n", *(void **)write,
            *(void **)puts, *(void **)printf);
}

static void ex_dyn_test_print_ex_ftable(void)
{
    printf(" .ptr_a: 0x%p  .ptr_b 0x%p  .ptr_c 0x%p\n",
        *(void **)ex_ftable.ptr_a,
        *(void **)ex_ftable.ptr_b,
        *(void **)ex_ftable.ptr_c);
}

static void ex_dyn_test_print_ex_ftable_ptr(void)
{
    printf("->ptr_a: 0x%p ->ptr_b 0x%p ->ptr_c 0x%p\n",
        *(void **)ex_ftable_ptr->ptr_a,
        *(void **)ex_ftable_ptr->ptr_b,
        *(void **)ex_ftable_ptr->ptr_c);
}

void ex_dyn_test_libftable(void)
{
    int (*func_c)(int) = ex_ftable_func_c;
    int (*change_funcs)(void) = ex_ftable_change_funcs;
    void (*print_ex_ftable)(void) = ex_dyn_test_print_ex_ftable;
    void (*print_ex_ftable_ptr)(void) = ex_dyn_test_print_ex_ftable_ptr;
    struct ex_ftable *ftable = &ex_ftable;

    ex_dyn_test_print_ex_ftable();
    ex_dyn_test_print_ex_ftable_ptr();
    printf("** using libftable via ftable [<%p>]...\n", ftable);
    ex_ftable.ptr_a(1);
    ex_ftable.ptr_b(2);
    ex_ftable.ptr_c(3);

    printf("** using libftable directly...\n");
    ex_ftable_func_c(4);
    ex_ftable_change_funcs();

    ex_dyn_test_print_ex_ftable_ptr();
    printf("** using libftable via ftable pointer...\n");
    ex_ftable_ptr->ptr_a(5);
    ex_ftable_ptr->ptr_b(6);
    ex_ftable_ptr->ptr_c(7);

    printf("** using libftable via function pointers...\n");
    func_c(8);
    change_funcs();
    print_ex_ftable();
    print_ex_ftable_ptr();

    putchar('\n');
}

void ex_dyn_test_print_libgarage(void)
{
    int *car_count = &ex_garage_car_count;
    int *truck_count = &ex_garage_truck_count;
    void (*print_vehicle_count)(void) = ex_garage_print_vehicle_count;

    printf("expected car count: %d - ", *car_count);
    ex_garage_print_car_count();
    printf("expected truck count: %d - ", *truck_count);
    ex_garage_print_truck_count();
    printf("expected total count: %d - ", ex_garage_car_count +
            ex_garage_truck_count);
    print_vehicle_count();
}

void ex_dyn_test_verify_libgarage(int cars, int trucks)
{
    if (cars != ex_garage_car_count) {
        printf("cars: %d != ex_garage_car_count: %d\n",
                cars, ex_garage_car_count);
        swbkpt();
    }
    if (trucks != ex_garage_truck_count) {
        printf("trucks: %d != ex_garage_truck_count: %d\n",
                trucks, ex_garage_truck_count);
        swbkpt();
    }
}

void ex_dyn_test_libgarage(void)
{
    void (*vv_fptr)(void);
    void (*print_libgarage)(void) = ex_dyn_test_print_libgarage;
    void (*verify_libgarage)(int, int) = ex_dyn_test_verify_libgarage;
    int *car_count = &ex_garage_car_count;
    int *truck_count = &ex_garage_truck_count;
    int test_car_count = *car_count;
    int test_truck_count = *truck_count;
    int count;

    ex_garage_print_banner();
    ex_dyn_test_print_libgarage();
    putchar('\n');

    count = 5;
    vv_fptr = ex_garage_inc_truck;
    while (count--) {
        ex_garage_inc_car();
        vv_fptr();
    }
    test_car_count += 5;
    test_truck_count += 5;
    ex_dyn_test_verify_libgarage(test_car_count, test_truck_count);
    ex_dyn_test_print_libgarage();
    putchar('\n');

    count = 2;
    vv_fptr = ex_garage_dec_truck;
    while (count--) {
        ex_garage_dec_car();
        vv_fptr();
    }
    test_car_count -= 2;
    test_truck_count -= 2;
    ex_dyn_test_verify_libgarage(test_car_count, test_truck_count);
    ex_dyn_test_print_libgarage();
    putchar('\n');

    count = 5;
    vv_fptr = ex_garage_inc_car;
    while (count--) {
        vv_fptr();
        ex_garage_inc_truck();
    }
    test_car_count += 5;
    test_truck_count += 5;
    verify_libgarage(test_car_count, test_truck_count);
    print_libgarage();
    putchar('\n');

    count = 2;
    vv_fptr = ex_garage_dec_car;
    while (count--) {
        vv_fptr();
        ex_garage_dec_truck();
    }
    test_car_count -= 2;
    test_truck_count -= 2;
    verify_libgarage(test_car_count, test_truck_count);
    print_libgarage();

    ex_garage_print_call_counts();
}

int main(int argc, char **argv)
{
    uint32_t pc = cpu_get_pc();
    uint32_t sp = cpu_get_sp();
    uint32_t fb = cpu_get_fb();
    int i;

    printf("dyn_test.elf\n");

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
    printf("main is located at: 0x%p\n\n", *(void **)main);

    ex_dyn_test_libftable();

    ex_dyn_test_libgarage();

    mystery_func();

    swbkpt();
    while (1);
}
