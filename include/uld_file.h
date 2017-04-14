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

#ifndef _ULD_FILE_H
#define _ULD_FILE_H


#include "uld.h"


#define uld_file_idx_to_sec_type(idx) \
    (ULD_SECTION_FLAG_TYPE_FLASH << (idx))


int uld_file_sec_type_to_idx(uint32_t type);

static __inline __always_inline __notrace int uld_file_get_sec_count(
        const struct uld_file *ufile)
{
    int i;
    int count = 0;

    for (i = 0; i < ULD_FILE_SECTION_TYPE_COUNT; i++) {
        count += ufile->num.n[i];
    }
    return count;
}

struct uld_section *uld_file_get_sec_by_index(const struct uld_file *ufile,
        int index, uint32_t type_mask);
struct uld_section *uld_file_get_sec_by_name(const struct uld_file *ufile,
        const char *name, uint32_t type_mask);

// type_mask is hint, if not found in ufile will search elf headers.
// section must have space allocated by caller.
int uld_file_copy_sec_by_index(const struct uld_file *ufile,
        struct uld_section *section, int index, uint32_t type_mask);
int uld_file_copy_sec_by_name(const struct uld_file *ufile,
        struct uld_section *section, const char *name, uint32_t type_mask);

#define ULD_FILE_GET_SEC_FUNC(fname, sname, type) \
    static __inline __always_inline __notrace \
    struct uld_section *uld_file_get_sec_##fname( \
            const struct uld_file *ufile) \
    { \
        return uld_file_get_sec_by_name(ufile, sname, \
            ULD_SECTION_FLAG_TYPE_##type); \
    }

ULD_FILE_GET_SEC_FUNC(plt,          ".plt",         FLASH)
ULD_FILE_GET_SEC_FUNC(rofixup,      ".rofixup",     FLASH)
ULD_FILE_GET_SEC_FUNC(got,          ".got",         MEM)
ULD_FILE_GET_SEC_FUNC(got_plt,      ".got.plt",     MEM)
ULD_FILE_GET_SEC_FUNC(hash,         ".hash",        DYNAMIC)
ULD_FILE_GET_SEC_FUNC(dynsym,       ".dynsym",      DYNAMIC)
ULD_FILE_GET_SEC_FUNC(dynstr,       ".dynstr",      DYNAMIC)
ULD_FILE_GET_SEC_FUNC(dynamic,      ".dynamic",     DYNAMIC)
ULD_FILE_GET_SEC_FUNC(rel_dyn,      ".rel.dyn",     DYNAMIC)


const void *uld_file_lma_to_adjusted_lma(const struct uld_file *ufile,
        const void *lma);
const void *uld_file_lma_to_adjusted_vma(const struct uld_file *ufile,
        const void *lma);


#endif  // _ULD_FILE_H
