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

#ifndef _ULD_SAL_H
#define _ULD_SAL_H


#include "uld.h"


// verify returns 1 on success
uint32_t uld_crc_calc_fst(void);
int uld_crc_verify_fst(uint32_t *crc);
int uld_crc_update_fst(uint32_t *crc);

uint32_t uld_crc_calc_fse(const struct uld_fs_entry *fse);
int uld_crc_verify_fse(const struct uld_fs_entry *fse, uint32_t *crc);
int uld_crc_update_fse(struct uld_fs_entry *fse, uint32_t *crc);


#define ULD_SECTION_MEM_NAME_LIST_COUNT             4
#define ULD_SECTION_MEM_NAME_MAX ULD_LOAD_MEM_SECTION_LIST_COUNT

#define ULD_SECTION_MEM_NAME_FIXUP_LIST_COUNT       3
#define ULD_SECTION_MEM_NAME_FIXUP_MAX ULD_LOAD_MEM_SECTION_FIXUP_LIST_COUNT

extern const char * const uld_section_mem_name_list[
        ULD_SECTION_MEM_NAME_LIST_COUNT];

#define uld_section_is_mem_sec_by_name(s) \
    (str_in_list(uld_section_mem_name_list, \
    ULD_SECTION_MEM_NAME_LIST_COUNT, (s)))

#define uld_section_is_mem_sec_fixup_by_name(s) \
    (str_in_list(uld_section_mem_name_list, \
    ULD_SECTION_MEM_NAME_FIXUP_LIST_COUNT, (s)))

uint32_t uld_section_get_type(const struct elf32_shdr *shdr, const char *name);


#define ULD_SECTION_FIND_TYPE_VMA                   0
#define ULD_SECTION_FIND_TYPE_ADJUSTED_VMA          1
#define ULD_SECTION_FIND_TYPE_LMA                   2
#define ULD_SECTION_FIND_TYPE_ADJUSTED_LMA          3
#define ULD_SECTION_FIND_TYPE_NAME                  4

union find_val {
    const void *ma;
    const char *str;
} __transparent_union;

struct uld_section *uld_section_find_in_list(
        struct uld_section *sec_list, int num, union find_val val,
        uint32_t type, uint32_t fmask, uint32_t fval);
struct uld_section *uld_section_find_in_lists(
        struct uld_section * const *sec_lists, const int *sec_list_num,
        int list_count, union find_val val, uint32_t type, uint32_t fmask,
        uint32_t fval);

struct uld_section *uld_section_find_in_list_by_lma_flag(
        struct uld_section *sec_list, int num, const void *lma,
        uint32_t fmask, uint32_t fval);
struct uld_section *uld_section_find_in_list_by_lma(
        struct uld_section *sec_list, int num, const void *lma);

struct uld_section *uld_section_find_in_list_by_adj_lma_flag(
        struct uld_section *sec_list, int num, const void *adjusted_lma,
        uint32_t fmask, uint32_t fval);
struct uld_section *uld_section_find_in_list_by_adj_lma(
        struct uld_section *sec_list, int num, const void *adjusted_lma);

struct uld_section *uld_section_find_in_lists_by_lma(
        struct uld_section * const *sec_lists, const int *sec_list_num,
        int list_count, const void *lma);

struct uld_section *uld_section_find_in_list_by_name(
        struct uld_section *sec_list, int num, const char *name);

struct uld_section *uld_section_find_in_lists_by_name(
        struct uld_section * const *sec_lists, const int *sec_list_num,
        int list_count, const char *name);

const void *uld_section_lma_to_adjusted_lma(const struct uld_section *section,
        const void *lma);

const void *uld_section_lma_to_adjusted_vma(const struct uld_section *section,
        const void *lma);


#endif  // _ULD_SAL_H
