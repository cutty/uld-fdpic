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
#include "uld_dyn.h"
#include "uld_fs.h"
#include "uld_reloc.h"


int uld_main(void)
{
    const void *ufile_dest;
    const struct uld_fs_entry *exec_fse;
    const struct uld_fs_entry *move_fse;
    const char *rv[5];
    const char **argv = NULL;
    const char *exec_name;
    const char *move_name = NULL;
    void *sp_base = NULL;
    int argc = 0;


    switch (ULD_PSTORE->boot_action) {
    case 0:
        exec_name = "hello_world_static.elf";
        break;

    case 1:
        uld_verbose = 1;
        exec_name = "hello_world.elf";
        rv[0] = "foo";
        rv[1] = "bar";
        rv[2] = NULL;
        argv = rv;
        argc = 2;

    case 2:
        exec_name = "hello_world.elf";
        move_name = exec_name;
        rv[0] = "moved";
        rv[1] = "hello";
        rv[2] = "world";
        rv[3] = NULL;
        argv = rv;
        argc = 3;
        break;

    case 3:
        exec_name = "dyn_test.elf";
        rv[0] = "woof";
        rv[1] = "bark";
        rv[2] = NULL;
        argv = rv;
        argc = 2;
        break;

    case 4:
        uld_verbose = 2;
        exec_name = "dyn_test.elf";
        break;

    case 5:
        uld_verbose = 1;
        exec_name = "dyn_test.elf";
        move_name = "libexc.so";
        rv[0] = "moved";
        rv[1] = "libexc.so";
        rv[2] = NULL;
        argv = rv;
        argc = 2;
        break;

    case 6:
        sp_base = ESTACK;
        exec_name = "dyn_test.elf";
        rv[0] = "stack";
        rv[1] = "reset";
        rv[2] = NULL;
        argv = rv;
        argc = 2;
        break;

    default:
        printf("invalid boot_action: %ld\n", ULD_PSTORE->boot_action);
        swbkpt();
        break;
    }

    exec_fse = uld_fs_get_file_by_name(ULD_PSTORE->fs_table_pri.head,
            exec_name);
    if (!exec_fse) {
        printf("could not find exec file: %s\n", exec_name);
        swbkpt();
    }

    if (move_name) {
        move_fse = uld_fs_get_file_by_name(ULD_PSTORE->fs_table_pri.head,
                move_name);
        if (!move_fse) {
            printf("could not find move file: %s\n", move_name);
            swbkpt();
        }
        ufile_dest = uld_fs_find_free_space(ULD_PSTORE, move_fse->size);
        printf("moving %s from %p to %p\n", move_fse->name, move_fse->base,
                ufile_dest);
        // if exec_fse == move_fse, moving the file will update fse.  Do not
        // need to lookup again.
        uld_reloc_move_fse(move_fse, ufile_dest);
    }

    uld_dyn_exec_fse(exec_fse, sp_base, argc, argv);

    swbkpt();
    while (1);
}
