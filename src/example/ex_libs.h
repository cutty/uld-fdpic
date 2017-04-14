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

#ifndef _SRC_EXAMPLE_EX_LIBS_H
#define _SRC_EXAMPLE_EX_LIBS_H


// ex_lib_ftable.c
typedef int (*ex_ftable_func)(int x);

struct ex_ftable {
    ex_ftable_func ptr_a;
    ex_ftable_func ptr_b;
    ex_ftable_func ptr_c;
};

extern struct ex_ftable ex_ftable;
extern struct ex_ftable *ex_ftable_ptr;

int ex_ftable_func_c(int x);
int ex_ftable_change_funcs(void);


// ex_lib_print.c
extern int ex_print_dl_alloc_a;
extern int ex_print_dl_alloc_b;
extern int ex_print_dl_alloc_c;

int ex_print_nputchar(int c, size_t n);
int ex_print_box(int x, int y);
int ex_print_box_text(const char *s);
int ex_print_upper(const char *s);
int ex_print_lower(const char *s);
void ex_print_cow(void);


// ex_lib_garage.c
extern int ex_garage_car_count;
extern int ex_garage_truck_count;
extern const char *ex_garage_name;
extern void (*mystery_func)(void);

void ex_garage_set_cars(int cars);
int ex_garage_get_cars(void);
void ex_garage_inc_car(void);
void ex_garage_dec_car(void);
void ex_garage_set_trucks(int trucks);
int ex_garage_get_trucks(void);
void ex_garage_inc_truck(void);
void ex_garage_dec_truck(void);
int ex_garage_get_vehicles(void);
void ex_garage_print_car_count(void);
void ex_garage_print_truck_count(void);
void ex_garage_print_vehicle_count(void);
void ex_garage_print_call_counts(void);
void ex_garage_print_banner(void);


#endif  // _SRC_EXAMPLE_EX_LIBS_H
