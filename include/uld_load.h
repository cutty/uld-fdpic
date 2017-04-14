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

#ifndef _ULD_LOAD_H
#define _ULD_LOAD_H


#include "uld.h"


#define ULD_LOAD_MEMBASE_ALIGNMENT                  3


// set shidx -1 to find it self.
// shstrtab_faddr is optional
int uld_load_create_section(struct uld_section *section,
        const struct elf32_ehdr *ehdr, const void *base,
        const struct elf32_shdr *shdr, int shidx,
        const void *shstrtab_faddr);

int uld_load_get_sec_count(const struct elf32_ehdr *ehdr,
        const void *base, uint32_t type_mask);

int uld_load_create_sec_list(const struct elf32_ehdr *ehdr,
        const void *base, struct uld_section *sec_list, int snum,
        uint32_t type_mask);

static __inline __always_inline __notrace int uld_load_create_flash_sec_list(
        const struct elf32_ehdr *ehdr, const void *base,
        struct uld_section *flash_list, int fnum)
{
    return uld_load_create_sec_list(ehdr, base, flash_list, fnum,
            ULD_SECTION_FLAG_TYPE_FLASH);
}

static __inline __always_inline __notrace int uld_load_create_mem_sec_list(
        const struct elf32_ehdr *ehdr, const void *base,
        struct uld_section *mem_list, int mnum)
{
    return uld_load_create_sec_list(ehdr, base, mem_list, mnum,
            ULD_SECTION_FLAG_TYPE_MEM);
}

static __inline __always_inline __notrace int uld_load_create_other_sec_list(
        const struct elf32_ehdr *ehdr, const void *base,
        struct uld_section *other_list, int onum)
{
    return uld_load_create_sec_list(ehdr, base, other_list, onum,
            ULD_SECTION_FLAG_TYPE_OTHER);
}

uint8_t *uld_load_get_next_membase(uint8_t *last_membase, size_t last_memsize);

int uld_load_alloc_mem_sections(const struct elf32_ehdr *ehdr,
        const void *base, uint8_t *membase, size_t *allocated,
        struct uld_section *mem_list, int mnum);

int uld_load_create_file(const struct uld_fs_entry *fse,
        struct uld_section *sec_list, int snum, uint32_t type_mask,
        struct uld_file *ufile);

// On call membase/allocated should hold the last base/allocated values set by
// uld_load_file.  On the first call use ptr/0 to start allocation at a
// specific address or NULL/0 to initialize values directly after uld memory
// space.  The original value of membase may be modified for alignment
// constraints.
int uld_load_file(const struct uld_fs_entry *fse,
        struct uld_section *sec_list, int snum, uint32_t type_mask,
        uint8_t **membase, size_t *allocated, struct uld_file *ufile);


#endif  // _ULD_LOAD_H
