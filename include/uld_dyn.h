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

#ifndef _ULD_DYN_H
#define _ULD_DYN_H


#include "uld.h"


#define ULD_DYN_VERBOSE                             1

#define ULD_DYN_FUNCDESC_ALIGNMENT                  3
#define ULD_DYN_ALLOC_ALIGNMENT                     2

#define ULD_DYN_LOAD_SECTION_TYPE_MASK \
    (ULD_SECTION_FLAG_TYPE_FLASH | \
     ULD_SECTION_FLAG_TYPE_MEM | \
     ULD_SECTION_FLAG_TYPE_DYNAMIC)


unsigned long uld_dyn_elf_hash(const unsigned char *name);

const struct elf32_sym *uld_dyn_find_dynsym_elf_hash_sec(const char *name,
        const struct uld_section *hash_sec,
        const struct uld_section *dynstr_sec,
        const struct uld_section *dynsym_sec);
const struct elf32_sym *uld_dyn_find_dynsym_elf_hash_file(const char *name,
        const struct uld_file *ufile);
const struct elf32_sym *uld_dyn_find_dynsym_linear_sec(const char *name,
        const struct uld_section *dynstr_sec,
        const struct uld_section *dynsym_sec);
const struct elf32_sym *uld_dyn_find_dynsym_linear_file(const char *name,
        const struct uld_file *ufile);

// -1 error, -2 out of space
// creates a dep list in order that they should be loaded, linked and
// initialized. On return dep_list[idx - 1] == fse.
int uld_dyn_create_fse_dep_list(const struct uld_fs_entry *fse,
        const struct uld_fs_entry **dep_list, int *idx, int list_size);

// This will call uld_dyn_create_fse_dep_list to get the exact number of
// dependencies and should is only useful for memory savings.
int uld_dyn_get_fse_dep_count(const struct uld_fs_entry *fse);

int uld_dyn_get_dep_list_sec_count(const struct uld_fs_entry **dep_list,
        int dep_count, uint32_t type_mask);

// membase r/w, membase may be moved for alignment. Allocated (w/o) is the size
// from the possibly updated membase and includes padding in between files.
// ufile_list[x].membase should be used directly as the base for each file's
// memory space.
int uld_dyn_load_fse_dep_list(const struct uld_fs_entry **dep_list,
        int dep_count, struct uld_file *ufile_list,
        struct uld_section *sec_list, int sec_count, uint8_t **membase,
        size_t *allocated);

int uld_dyn_link_file_list(const struct uld_file *ufile_list, int file_count,
        uint8_t *dl_alloc_base, size_t *dl_alloc_size);

int uld_dyn_exec_fse(const struct uld_fs_entry *fse, void *sp_base, int argc,
        const char **argv);


#define uld_dyn_get_sym_name(sym, dynstr_sec) \
    (((const char *)((dynstr_sec)->adjusted_lma)) + (sym)->st_name)

#define uld_dyn_get_sym_index(sym, dynsym_sec) \
    (unsigned int)((((const uint8_t *)(sym)) - \
    ((const uint8_t *)(dynsym_sec)->adjusted_lma)) / \
    sizeof(struct elf32_sym))


// Get dynamic structures by index.
#define uld_dyn_get_dyn_by_index_sec(idx, dyn_sec) \
    (((struct elf32_dyn *)((dyn_sec)->adjusted_lma)) + (idx))

#define uld_dyn_get_dyn_by_index_file(idx, ufile) \
    uld_dyn_get_dyn_by_index_sec((idx), \
    uld_file_get_sec_dyn((ufile)))

#define uld_dyn_get_dynsym_by_index_sec(idx, dynsym_sec) \
    (((struct elf32_sym *)((dynsym_sec)->adjusted_lma)) + (idx))

#define uld_dyn_get_dynsym_by_index_file(idx, ufile) \
    uld_dyn_get_dynsym_by_index_sec((idx), \
    uld_file_get_sec_dynsym((ufile)))

#define uld_dyn_get_rel_dyn_by_index_sec(idx, rel_dyn_sec) \
    (((struct elf32_rel *)((rel_dyn_sec)->adjusted_lma)) + (idx))

#define uld_dyn_get_rel_dyn_by_index_file(idx, ufile) \
    uld_dyn_get_rel_dyn_by_index_sec((idx), \
    uld_file_get_sec_rel_dyn((ufile)))


// Get dynamic structure counts in section.
#define uld_dyn_get_dyn_count_sec(dyn_sec) \
   ((dyn_sec)->shdr->sh_size / sizeof(struct elf32_dyn))

#define uld_dyn_get_dyn_count_file(ufile) \
    uld_dyn_get_dyn_count_sec(uld_file_get_sec_dyn((ufile)))

#define uld_dyn_get_dynsym_count_sec(dynsym_sec) \
   ((dynsym_sec)->shdr->sh_size / sizeof(struct elf32_sym))

#define uld_dyn_get_dynsym_count_file(ufile) \
    uld_dyn_get_dynsym_count_sec(uld_file_get_sec_dynsym((ufile)))

#define uld_dyn_get_rel_dyn_count_sec(rel_dyn_sec) \
   ((rel_dyn_sec)->shdr->sh_size / sizeof(struct elf32_rel))

#define uld_dyn_get_rel_dyn_count_file(ufile) \
    uld_dyn_get_rel_dyn_count_sec(uld_file_get_sec_rel_dyn((ufile)))


// Iterate over dynamic strctures.
#define uld_dyn_for_each_dyn_sec(pos, dyn_sec) \
    for ((pos) = (const struct elf32_dyn *)(dyn_sec)->adjusted_lma; \
        (pos)->d_tag != DT_NULL; (pos)++)

#define uld_dyn_for_each_dyn_file(pos, ufile) \
    uld_dyn_for_each_dyn_sec((pos), \
    (uld_file_get_sec_dynamic((ufile))))

#define uld_dyn_for_each_dynsym_sec(pos, idx, dynsym_sec) \
    for ((pos) = (const struct elf32_sym *)(dynsym_sec)->adjusted_lma, \
        (idx) = 0; \
        (idx) < (dynsym_sec)->shdr->sh_size / sizeof(struct elf32_sym); \
        (idx)++, (pos)++)

#define uld_dyn_for_each_dynsym_file(pos, idx, ufile) \
    uld_dyn_for_each_dynsym_sec((pos), (idx), \
    uld_file_get_sec_dynsym((ufile)))

#define uld_dyn_for_each_rel_dyn_sec(pos, idx, rel_dyn_sec) \
    for ((pos) = (const struct elf32_rel *)(rel_dyn_sec)->adjusted_lma, \
        (idx) = 0; \
        (idx) < (rel_dyn_sec)->shdr->sh_size / sizeof(struct elf32_rel); \
        (idx)++, (pos)++)

#define uld_dyn_for_each_rel_dyn_file(pos, idx, ufile) \
    uld_dyn_for_each_rel_dyn_sec((pos), (idx), \
    uld_file_get_sec_rel_dyn((ufile)))

static __inline __always_inline __notrace
const struct elf32_dyn *uld_dyn_next_dt_needed(const struct elf32_dyn *dyn)
{
    while (dyn->d_tag) {
        if (dyn->d_tag == DT_NEEDED) {
            return dyn;
        }
        dyn++;
    }
    return NULL;
}

#define uld_dyn_for_each_dt_needed(pos, dyn_sec) \
    for ((pos) = uld_dyn_next_dt_needed( \
        (const struct elf32_dyn *)(dyn_sec)->adjusted_lma); \
        (pos); (pos) = uld_dyn_next_dt_needed(++(pos)))


#endif  // _ULD_DYN_H
