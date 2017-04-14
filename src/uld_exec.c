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
#include "uld_exec.h"


const char * const uld_exec_init_section_list[] = {
    ".preinit_array",
    ".init_array"
};


void uld_exec_call_vv_fp_array(void (**arr_start)(void),
        void (**arr_end)(void))
{
    if (!arr_start || !arr_end) {
        return;
    }

    while (arr_start < arr_end) {
        if (arr_start) {
            (*arr_start)();
        }
        arr_start++;
    }
}

void uld_exec_call_vv_fp_array_fdpic_base(void (**arr_start)(void),
        void (**arr_end)(void), uint32_t fdpic_base)
{
    if (!arr_start || !arr_end) {
        return;
    }

    while (arr_start < arr_end) {
        if (arr_start) {
            uld_exec_call_vv_fp_fdpic_base(*arr_start, fdpic_base);
        }
        arr_start++;
    }
}


int uld_exec_elf_call_init_funcs(struct uld_file *ufile)
{
    const struct elf32_ehdr *ehdr;
    const struct elf32_shdr *init_shdr;
    void (**arr_start)(void);
    unsigned int i;

    if (!ufile) {
        return -1;
    }

    ehdr = ufile->fse->base;

    for (i = 0; i < sizeof(uld_exec_init_section_list) / sizeof(char *); i++) {
        init_shdr = elf32_get_section_by_name(ehdr, NULL,
                uld_exec_init_section_list[i]);
        if (!init_shdr) {
            continue;
        }

        arr_start = (void (**)(void))elf32_get_section_flash_addr(
                init_shdr, ehdr);
        if (uld_verbose) {
            printf("calling %ld init functions in %s - %s [<%p>]\n",
                    init_shdr->sh_size / sizeof(void *), ufile->fse->name,
                    uld_exec_init_section_list[i], arr_start);
        }

        uld_exec_call_vv_fp_array_fdpic_base(arr_start,
                arr_start + (init_shdr->sh_size / sizeof(void *)),
                (uint32_t)ufile->membase);
    }

    return 0;
}

int uld_exec_file(const struct uld_file *ufile, void *sp_base, int argc,
        const char **argv)
{
    const void *entryfp;
    int ret;
    int i;

    if (!ufile || argc < 0 || (argc && !argv)) {
        return -1;
    }

    entryfp = elf32_get_adjusted_entry(ufile->fse->base, NULL);

    sp_base = (void *)((uintptr_t)sp_base & ~0x7);

    printf("\nstarting %s entry at [<%p>] with args:", ufile->fse->name,
            entryfp);
    if (argc) {
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
    } else {
        printf(" <none>");
    }
    printf("\nusing stack base: %p fdpic base: %p\n\n", sp_base,
            ufile->membase);

#ifdef ULD_BREAK_BEFORE_ENTRY
    puts("break before entry - gdb: uc to continue");
    swbkpt();
#endif

    ret = uld_exec_elf_call_entry(entryfp, sp_base, argc, argv,
            (uint32_t)ufile->membase);

    return ret;
}
