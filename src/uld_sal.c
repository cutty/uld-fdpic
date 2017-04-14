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
#include "uld_sal.h"
#include "util.h"


// Only some sections loaded into memory require rofixup and should be placed
// at the beginning of the array for use in uld_load_apply_mem_rofixup.
// When updating this list update the defines in uld_sal.h
// Note: .data rofixup is done during build/flash relocation.
const char * const uld_section_mem_name_list[
        ULD_SECTION_MEM_NAME_LIST_COUNT] = {
    ".got",         // mem/flash rofixup
    ".got.plt",     // mem rofixup
    ".data",        // flash rofixup
    ".bss",
};


uint32_t uld_crc_calc_fst(void)
{
    return crc32(ULD_PSTORE->fs_table_pri.table_base,
            ULD_PSTORE->fs_table_pri.table_size, UTIL_CRC32_INIT);
}

int uld_crc_verify_fst(uint32_t *crc)
{
    uint32_t c = uld_crc_calc_fst();
    if (crc) {
        *crc = c;
    }
    return ULD_PSTORE->fs_table_pri.crc == c;
}

int uld_crc_update_fst(uint32_t *crc)
{
    uint32_t c = uld_crc_calc_fst();
    if (crc) {
        *crc = c;
    }
    printf("Updating fst crc: 0x%08lx\n", c);
    return cpu_flash_write(&ULD_PSTORE->fs_table_pri.crc, &c,
            sizeof(uint32_t));
}

uint32_t uld_crc_calc_fse(const struct uld_fs_entry *fse)
{
    return crc32(fse->base, fse->size, UTIL_CRC32_INIT);
}

int uld_crc_verify_fse(const struct uld_fs_entry *fse, uint32_t *crc)
{
    uint32_t c = uld_crc_calc_fse(fse);
    if (crc) {
        *crc = c;
    }
    return fse->crc == c;
}

int uld_crc_update_fse(struct uld_fs_entry *fse, uint32_t *crc)
{
    uint32_t c = uld_crc_calc_fse(fse);
    if (crc) {
        *crc = c;
    }
    printf("Updating fse crc: 0x%08lx\n", c);
    return cpu_flash_write(&fse->crc, &c, sizeof(uint32_t));
}

uint32_t uld_section_get_type(const struct elf32_shdr *shdr, const char *name)
{
    if (shdr->sh_flags & SHF_ALLOC) {
        if (uld_section_is_mem_sec_by_name(name)) {
            return ULD_SECTION_FLAG_TYPE_MEM;
        } else {
            switch (shdr->sh_type) {
            case SHT_PROGBITS:
            case SHT_NOBITS:
            case SHT_INIT_ARRAY:
            case SHT_FINI_ARRAY:
            case SHT_PREINIT_ARRAY:
                return ULD_SECTION_FLAG_TYPE_FLASH;
                break;

            // The dynamic symbol strtab will have the alloc flag set, the
            // static symbol strtab will not and be typed as other.
            case SHT_STRTAB:
            case SHT_RELA:
            case SHT_HASH:
            case SHT_DYNAMIC:
            case SHT_REL:
            case SHT_DYNSYM:
                return ULD_SECTION_FLAG_TYPE_DYNAMIC;
                break;

            default:
                swbkpt();
                break;
            }
        }
    }
    return ULD_SECTION_FLAG_TYPE_OTHER;
}

struct uld_section *uld_section_find_in_list(
        struct uld_section *sec_list, int num, union find_val val,
        uint32_t type, uint32_t fmask, uint32_t fval)
{
    union find_val sec_val;
    int i;

    if (!sec_list || num <= 0) {
        return NULL;
    }

    switch (type) {
    case ULD_SECTION_FIND_TYPE_VMA:
    case ULD_SECTION_FIND_TYPE_ADJUSTED_VMA:
    case ULD_SECTION_FIND_TYPE_LMA:
    case ULD_SECTION_FIND_TYPE_ADJUSTED_LMA:
        break;

    case ULD_SECTION_FIND_TYPE_NAME:
        if (!val.str) {
            return NULL;
        }
        break;

    default:
        return NULL;
        break;
    }

    for (i = 0; i < num; i++) {
        if ((sec_list[i].flags & fmask) != fval) {
            continue;
        }
        switch (type) {
        case ULD_SECTION_FIND_TYPE_VMA:
            sec_val.ma = (const void *)sec_list[i].shdr->sh_addr;
            break;

        case ULD_SECTION_FIND_TYPE_ADJUSTED_VMA:
            sec_val.ma = sec_list[i].adjusted_vma;
            break;

        case ULD_SECTION_FIND_TYPE_LMA:
            sec_val.ma = sec_list[i].lma;
            break;

        case ULD_SECTION_FIND_TYPE_ADJUSTED_LMA:
            sec_val.ma = sec_list[i].adjusted_lma;
            break;

        case ULD_SECTION_FIND_TYPE_NAME:
            sec_val.str = sec_list[i].name;
            break;

        default:
            return NULL;
            break;
        }

        switch (type) {
        case ULD_SECTION_FIND_TYPE_VMA:
        case ULD_SECTION_FIND_TYPE_ADJUSTED_VMA:
        case ULD_SECTION_FIND_TYPE_LMA:
        case ULD_SECTION_FIND_TYPE_ADJUSTED_LMA:
            if (val.ma >= sec_val.ma &&
                    val.ma < sec_val.ma + sec_list[i].shdr->sh_size) {
                return &sec_list[i];
            }
            break;

        case ULD_SECTION_FIND_TYPE_NAME:
            if (!(strcmp(val.str, sec_val.str))) {
                return &sec_list[i];
            }
            break;

        default:
            return NULL;
            break;
        }
    }

    return NULL;
}

struct uld_section *uld_section_find_in_lists(
        struct uld_section * const *sec_lists, const int *sec_list_num,
        int list_count, union find_val val, uint32_t type, uint32_t fmask,
        uint32_t fval)
{
    struct uld_section *sec;
    int i;

    if (!sec_lists || !sec_list_num || list_count <= 0) {
        return NULL;
    }

    for (i = 0; i < list_count; i++) {
        sec = uld_section_find_in_list(sec_lists[i], sec_list_num[i], val,
                type, fmask, fval);
        if (sec) {
            return sec;
        }
    }

    return NULL;
}

struct uld_section *uld_section_find_in_list_by_lma_flag(
        struct uld_section *sec_list, int num, const void *lma,
        uint32_t fmask, uint32_t fval)
{
    union find_val val = {.ma = lma};
    return uld_section_find_in_list(sec_list, num, val,
            ULD_SECTION_FIND_TYPE_LMA, fmask, fval);
}

struct uld_section *uld_section_find_in_list_by_lma(
        struct uld_section *sec_list, int num, const void *lma)
{
    union find_val val = {.ma = lma};
    return uld_section_find_in_list(sec_list, num, val,
            ULD_SECTION_FIND_TYPE_LMA, ULD_SECTION_FLAG_NONE,
            ULD_SECTION_FLAG_NONE);
}

struct uld_section *uld_section_find_in_list_by_adj_lma_flag(
        struct uld_section *sec_list, int num, const void *adjusted_lma,
        uint32_t fmask, uint32_t fval)
{
    union find_val val = {.ma = adjusted_lma};
    return uld_section_find_in_list(sec_list, num, val,
            ULD_SECTION_FIND_TYPE_ADJUSTED_LMA, fmask, fval);
}

struct uld_section *uld_section_find_in_list_by_adj_lma(
        struct uld_section *sec_list, int num, const void *adjusted_lma)
{
    union find_val val = {.ma = adjusted_lma};
    return uld_section_find_in_list(sec_list, num, val,
            ULD_SECTION_FIND_TYPE_ADJUSTED_LMA, ULD_SECTION_FLAG_NONE,
            ULD_SECTION_FLAG_NONE);
}

struct uld_section *uld_section_find_in_lists_by_lma(
        struct uld_section * const *sec_lists, const int *sec_list_num,
        int list_count, const void *lma)
{
    union find_val val = {.ma = lma};
    return uld_section_find_in_lists(sec_lists, sec_list_num, list_count,
            val, ULD_SECTION_FIND_TYPE_LMA, ULD_SECTION_FLAG_NONE,
            ULD_SECTION_FLAG_NONE);
}

struct uld_section *uld_section_find_in_list_by_name(
        struct uld_section *sec_list, int num, const char *name)
{
    union find_val val = {.str = name};
    return uld_section_find_in_list(sec_list, num, val,
            ULD_SECTION_FIND_TYPE_NAME, ULD_SECTION_FLAG_NONE,
            ULD_SECTION_FLAG_NONE);
}

struct uld_section *uld_section_find_in_lists_by_name(
        struct uld_section * const *sec_lists, const int *sec_list_num,
        int list_count, const char *name)
{
    union find_val val = {.str = name};
    return uld_section_find_in_lists(sec_lists, sec_list_num, list_count,
            val, ULD_SECTION_FIND_TYPE_NAME, ULD_SECTION_FLAG_NONE,
            ULD_SECTION_FLAG_NONE);
}

const void *uld_section_lma_to_adjusted_ma(const struct uld_section *section,
        const void *lma, int to_adj_lma)
{
    const void *adj_base;

    if (!section || !lma) {
        return NULL;
    }

    if (lma < section->lma || lma >= section->lma + section->shdr->sh_size) {
        return NULL;
    }

    // Do not do adjustments on NOBITS sections if lma is requested or
    // section has not been loaded.
    if (section->shdr->sh_type == SHT_NOBITS &&(to_adj_lma ||
            !(section->flags & ULD_SECTION_FLAG_STATUS_MEM_LOADED))) {
        return NULL;
    }

    adj_base = to_adj_lma ? section->adjusted_lma : section->adjusted_vma;
    return lma - section->lma + adj_base;
}

const void *uld_section_lma_to_adjusted_lma(const struct uld_section *section,
        const void *lma)
{
    return uld_section_lma_to_adjusted_ma(section, lma, 1);
}

const void *uld_section_lma_to_adjusted_vma(const struct uld_section *section,
        const void *lma)
{
    return uld_section_lma_to_adjusted_ma(section, lma, 0);
}
