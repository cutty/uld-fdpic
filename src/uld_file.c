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
#include "uld_file.h"
#include "uld_load.h"
#include "uld_sal.h"


int uld_file_sec_type_to_idx(uint32_t type)
{
    type &= ULD_SECTION_FLAG_TYPE_MASK;
    switch (type) {
    case ULD_SECTION_FLAG_TYPE_FLASH: return ULD_FILE_SECTION_IDX_FLASH;
    case ULD_SECTION_FLAG_TYPE_MEM: return ULD_FILE_SECTION_IDX_MEM;
    case ULD_SECTION_FLAG_TYPE_OTHER: return ULD_FILE_SECTION_IDX_OTHER;
    default: return -1;
    }
}

struct uld_section *uld_file_get_sec_by_index(const struct uld_file *ufile,
        int index, uint32_t type_mask)
{
    int i;
    int j;
    int type;

    if (!ufile || index < 0) {
        return NULL;
    }

    for (i = 0; i < ULD_FILE_SECTION_TYPE_COUNT; i++) {
        type = type_mask & uld_file_idx_to_sec_type(i);
        if (!type) {
            continue;
        }
        for (j = 0; j < ufile->num.n[i]; j++) {
            if (ufile->sec.s[i][j].shidx == index) {
                return &ufile->sec.s[i][j];
            }
        }
    }

    return NULL;
}

struct uld_section *uld_file_get_sec_by_name(const struct uld_file *ufile,
        const char *name, uint32_t type_mask)
{
    int i;
    int j;
    int type;

    if (!ufile || !name) {
        return NULL;
    }

    for (i = 0; i < ULD_FILE_SECTION_TYPE_COUNT; i++) {
        type = type_mask & uld_file_idx_to_sec_type(i);
        if (!type) {
            continue;
        }
        for (j = 0; j < ufile->num.n[i]; j++) {
            if (!strcmp(ufile->sec.s[i][j].name, name)) {
                return &ufile->sec.s[i][j];
            }
        }
    }

    return NULL;
}

int uld_file_copy_sec_by_index(const struct uld_file *ufile,
        struct uld_section *section, int index, uint32_t type_mask)
{
    struct uld_section *p;
    const struct elf32_ehdr *ehdr;
    const struct elf32_shdr *shdr;
    int ret = 0;

    if (!ufile || !section) {
        return -1;
    }

    p = uld_file_get_sec_by_index(ufile, index, type_mask);
    if (p) {
        memcpy(section, p, sizeof(struct uld_section));
        return ret;
    }

    ehdr = (const struct elf32_ehdr*)ufile->fse->base;
    shdr = elf32_get_section_by_index(ehdr, NULL, index);
    if (!shdr) {
        return -1;
    }

    ret = uld_load_create_section(section, ehdr, NULL, shdr, index, NULL);

    return ret;
}

int uld_file_copy_sec_by_name(const struct uld_file *ufile,
        struct uld_section *section, const char *name, uint32_t type_mask)
{
    struct uld_section *p;
    const struct elf32_ehdr *ehdr;
    const struct elf32_shdr *shdr;
    int ret = 0;

    if (!ufile || !section || !name) {
        return -1;
    }

    p = uld_file_get_sec_by_name(ufile, name, type_mask);
    if (p) {
        memcpy(section, p, sizeof(struct uld_section));
        return ret;
    }

    ehdr = (const struct elf32_ehdr*)ufile->fse->base;
    shdr = elf32_get_section_by_name(ehdr, NULL, name);
    if (!shdr) {
        return -1;
    }

    ret = uld_load_create_section(section, ehdr, NULL, shdr, -1, NULL);

    return ret;
}

static const void *uld_file_lma_to_adjusted_ma(const struct uld_file *ufile,
        const void *lma, int to_adj_lma)
{
    struct uld_section *section;

    section = uld_section_find_in_lists_by_lma(ufile->sec.s, ufile->num.n,
        ULD_FILE_SECTION_TYPE_COUNT, lma);
    if (!section) {
        return NULL;
    }

    return to_adj_lma ? uld_section_lma_to_adjusted_lma(section, lma) :
            uld_section_lma_to_adjusted_vma(section, lma);
}

const void *uld_file_lma_to_adjusted_lma(const struct uld_file *ufile,
        const void *lma)
{
    return uld_file_lma_to_adjusted_ma(ufile, lma, 1);
}

const void *uld_file_lma_to_adjusted_vma(const struct uld_file *ufile,
        const void *lma)
{
    return uld_file_lma_to_adjusted_ma(ufile, lma, 0);
}
