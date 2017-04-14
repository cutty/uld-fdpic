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
#include "uld_load.h"
#include "uld_rofixup.h"
#include "uld_sal.h"


struct do_flash_fixup_userdata {
    const void *base;
    struct uld_section *flash_list;
    struct uld_section *mem_list;
    int fnum;
    int mnum;
};

struct do_mem_fixup_userdata {
    struct uld_section *flash_list;
    struct uld_section *mem_list;
    struct uld_section *got_sec;
    struct uld_section *got_plt_sec;
    uint32_t membase;
    int fnum;
    int mnum;
};

struct do_fixup_data {
    uint8_t **fixup_addr;
    struct uld_section *fixup_sec;
    uint8_t **adj_fixup_addr;
    uint8_t *new_adj_fixup_addr;
    void *userdata;
};

typedef int (*do_fixup_t)(struct do_fixup_data *dfd);

static int uld_rofixup_apply_fixups(const struct elf32_ehdr *ehdr,
        const void *base, struct uld_section *sec_list, int num,
        uint32_t need_flag, uint32_t done_flag, do_fixup_t do_fixup,
        void *userdata)
{
    struct do_fixup_data dfd;
    struct uld_section rofixup;
    const struct elf32_shdr *rofixup_shdr;
    const void *min_fixup_lma;
    const void *max_fixup_lma;
    int rofixup_shidx;
    int i;
    int ret;
    int fixup_offset;

    if (!ehdr || !sec_list || num <= 0 || !do_fixup) {
        return -1;
    }

    if (!base) {
        base = (const void *)ehdr;
    }

    dfd.userdata = userdata;

    rofixup_shdr = elf32_get_section_by_name(ehdr, base, ".rofixup");
    if (!rofixup_shdr) {
        // This could be a programming error, however if an application does
        // not need any fixups it will not have a .rofixup section.
        return 1;
    }

    rofixup_shidx = elf32_get_shidx_by_section(ehdr, base, rofixup_shdr);
    uld_load_create_section(&rofixup, ehdr, base, rofixup_shdr, rofixup_shidx,
            NULL);

    // If sec_list was created by uld_load_create_sec_list it should sorted
    // by vma which could be in a different order than lma.  All of the
    // addresses in rofixup will be the lma address stored in the elf header.
    // This will provide a quick limit before iterating the lists.
    min_fixup_lma = (const void *)0xffffffff;
    max_fixup_lma = (const void *)0;
    for (i = 0; i < num; i++) {
        if (sec_list[i].flags & need_flag) {
            min_fixup_lma = MIN(min_fixup_lma, sec_list[i].lma);
            max_fixup_lma = MAX(max_fixup_lma, sec_list[i].lma +
                    sec_list[i].shdr->sh_size);
        }
    }

    for (dfd.fixup_addr = (uint8_t **)rofixup.adjusted_lma, fixup_offset = 0;
            dfd.fixup_addr < (uint8_t **)(rofixup.adjusted_lma +
            rofixup.shdr->sh_size);
            dfd.fixup_addr++, fixup_offset += sizeof(void *)) {

        // Check that the address where the fixup will occur is in a sec_list.
        if (*dfd.fixup_addr < (uint8_t *)min_fixup_lma ||
                *dfd.fixup_addr >= (uint8_t *)max_fixup_lma) {
            continue;
        }

        // Note: This may return NULL where marked sections in sec_list are
        // not contiguous.
        dfd.fixup_sec = uld_section_find_in_list_by_lma_flag(sec_list, num,
                *dfd.fixup_addr, need_flag, need_flag);
        if (!dfd.fixup_sec) {
            printf("Info: could not find marked sec for lma %p\n",
                    *dfd.fixup_addr);
            continue;
        }

        ret = do_fixup(&dfd);

        if (!ret) {
            if (uld_verbose) {
                printf("  %p       %p       %p       %p       %p\n",
                        dfd.fixup_addr, *dfd.fixup_addr, dfd.adj_fixup_addr,
                        *dfd.adj_fixup_addr, dfd.new_adj_fixup_addr);
            }
            *dfd.adj_fixup_addr = dfd.new_adj_fixup_addr;
        } else if (ret < 0) {
            return ret;
        }
        // ret > 0 = OK, no update required.
    }

    for (i = 0; i < num; i++) {
        sec_list[i].flags |= done_flag;
    }

    return 0;
}

static int uld_rofixup_do_flash_fixup(struct do_fixup_data *dfd)
{
    struct do_flash_fixup_userdata *ud = dfd->userdata;
    struct uld_section *fixup_tgt_sec;
    const void *fixup_sec_new_adj_lma;
    const void *fixup_tgt_sec_new_adj_lma;

    // Get the relocated (new) adjusted lma where the fixup will take occur.
    fixup_sec_new_adj_lma = elf32_adjust_lma(dfd->fixup_sec->shdr, ud->base);
    dfd->adj_fixup_addr = (uint8_t **)(*dfd->fixup_addr -
            (uintptr_t)dfd->fixup_sec->lma + (uintptr_t)fixup_sec_new_adj_lma);
    if (*dfd->adj_fixup_addr == 0) {
        // ignore blank entry.
        return 1;
    }

    // Find the fixup target section.  Check both the flash and memory lists.
    // Some sections such as .data are in flash at rest and memory at
    // runtime.  Fixups for these sections are done once in flash.
    fixup_tgt_sec = uld_section_find_in_list_by_adj_lma(ud->flash_list,
            ud->fnum, *dfd->adj_fixup_addr);
    if (!fixup_tgt_sec) {
        fixup_tgt_sec = uld_section_find_in_list_by_adj_lma(ud->mem_list,
                ud->mnum, *dfd->adj_fixup_addr);
    }

    // If fixup_tgt_sec is not found this should be a memory based fixup
    // pointing to memory using the original lma.
    if (!fixup_tgt_sec) {
        if (ud->mem_list && ud->mnum > 0) {
            fixup_tgt_sec = uld_section_find_in_list_by_lma(ud->mem_list,
                    ud->mnum, *dfd->adj_fixup_addr);
        }
        // Flash based fixups always require mem and flash section lists
        // (unlike mem fixups where flash sections may be omitted for
        // performance/mem usage).  Always error out if any fixup is
        // not accounted for.
        if (!fixup_tgt_sec) {
            printf("Error: could not find sec for lma %p\n",
                    *dfd->adj_fixup_addr);
            swbkpt();
        } else if (uld_verbose) {
            printf("Info: memory target fixup %p %p in memory "
                    "section: %p\n", *dfd->fixup_addr,
                    dfd->adj_fixup_addr, *dfd->adj_fixup_addr);
        }
        return 1;
    }

    // Get the target relocated (new) adjusted lma for the fixup.
    fixup_tgt_sec_new_adj_lma = elf32_adjust_lma(fixup_tgt_sec->shdr,
            ud->base);
    dfd->new_adj_fixup_addr = *dfd->adj_fixup_addr -
            (uintptr_t)fixup_tgt_sec->adjusted_vma +
            (uintptr_t)fixup_tgt_sec_new_adj_lma;

    return 0;
}

static int uld_rofixup_do_mem_fixup(struct do_fixup_data *dfd)
{
    struct do_mem_fixup_userdata *ud = dfd->userdata;
    struct uld_section *fixup_tgt_sec;

    dfd->adj_fixup_addr = (uint8_t **)(*dfd->fixup_addr -
            (uintptr_t)dfd->fixup_sec->lma +
            (uintptr_t)dfd->fixup_sec->adjusted_vma);
    if (*dfd->adj_fixup_addr == 0) {
        // ignore blank entry.
        return 1;
    }

    fixup_tgt_sec = uld_section_find_in_list_by_lma(ud->mem_list, ud->mnum,
            *dfd->adj_fixup_addr);
    // Fixup in a memory section may be pointing to a flash section and
    // should already have the fixup and adjustment done.  This is an
    // extra safety check for development if flash sections are provided
    // during memory fixups.  During normal operation if the target
    // section is not found in mem_list return 1 to skip over the fixup.
    if (!fixup_tgt_sec) {
        if (ud->flash_list && ud->fnum > 0) {
            fixup_tgt_sec = uld_section_find_in_list_by_adj_lma(
                    ud->flash_list, ud->fnum, *dfd->adj_fixup_addr);
            if (!fixup_tgt_sec) {
                printf("Error: could not find sec for (adjusted_)lma %p\n",
                        *dfd->adj_fixup_addr);
                swbkpt();
            } else if (uld_verbose) {
                printf("Info: flash target fixup %p %p in memory "
                        "section: %p\n", *dfd->fixup_addr,
                        dfd->adj_fixup_addr, *dfd->adj_fixup_addr);
            }
        }
        return 1;
    }

    // If the fixup address is in .got and the fixup target points to
    // the base lma of the .got.plt section it should be set to the file's
    // membase (fdpic_base).  This is necessary when funcdesc_value is local
    // to the .got table (used when a auto function pointer is set to
    // a externally visible function local to the file).
    if (dfd->fixup_sec == ud->got_sec && ud->got_plt_sec &&
            *dfd->adj_fixup_addr == ud->got_plt_sec->lma) {
        dfd->new_adj_fixup_addr = (uint8_t *)ud->membase;
    } else {
        dfd->new_adj_fixup_addr = *dfd->adj_fixup_addr -
                (uintptr_t)fixup_tgt_sec->lma +
                (uintptr_t)fixup_tgt_sec->adjusted_vma;
    }

    return 0;
}

int uld_rofixup_apply_flash_fixups(const struct elf32_ehdr *ehdr,
        const void *base, struct uld_section *flash_list, int fnum,
        struct uld_section *mem_list, int mnum)
{
    struct do_flash_fixup_userdata ud = {
        .flash_list = flash_list,
        .mem_list = mem_list,
        .fnum = fnum,
        .mnum = mnum
    };
    int ret;

    // Every elf file should at least have a flash (.text) section, but
    // may not have any memory sections.
    if (!ehdr || !flash_list || fnum <= 0 || mnum < 0 || (mnum && !mem_list)) {
        return -1;
    }

    if (!base) {
        base = (const void *)ehdr;
    }
    ud.base = base;

    if (uld_verbose) {
        puts("Applying flash fixups:");
        puts("  fixup_ptr      fixup_val      fixup_lma      fixup_tgt_lma  "
                "rel_fixup_tgt_lma");
    }

    // Process both the flash and memory lists.  Some sections such
    // as .data are in flash at rest and memory at runtime.  Fixups for these
    // sections are done once in flash.
    ret = uld_rofixup_apply_fixups(ehdr, base, flash_list, fnum,
            ULD_SECTION_FLAG_STATUS_FLASH_NEEDS_FIXUP,
            ULD_SECTION_FLAG_STATUS_FLASH_FIXUP_DONE,
            uld_rofixup_do_flash_fixup, &ud);
    if (!ret) {
        ret = uld_rofixup_apply_fixups(ehdr, base, mem_list, mnum,
                ULD_SECTION_FLAG_STATUS_FLASH_NEEDS_FIXUP,
                ULD_SECTION_FLAG_STATUS_FLASH_FIXUP_DONE,
                uld_rofixup_do_flash_fixup, &ud);
    }

    return ret;
}

int uld_rofixup_apply_mem_fixups(const struct elf32_ehdr *ehdr,
        const void *base, struct uld_section *flash_list, int fnum,
        struct uld_section *mem_list, int mnum, uint32_t membase)
{
    struct do_mem_fixup_userdata ud = {
        .flash_list = flash_list,
        .mem_list = mem_list,
        .membase = membase,
        .mnum = mnum,
        .fnum = fnum
    };
    int ret;

    // Elf files may not have memory sections, just check for conflicting/bad
    // parameters.
    if (!ehdr || mnum < 0 || (mnum && !mem_list)) {
        return -1;
    }

    // Return now if there are no memory sections.
    if (!mnum) {
        return 1;
    }

    ud.got_sec = uld_section_find_in_list_by_name(mem_list, mnum, ".got");
    ud.got_plt_sec = uld_section_find_in_list_by_name(mem_list, mnum,
            ".got.plt");

    if (uld_verbose) {
        puts("Applying memory fixups:");
        puts("  fixup_ptr      fixup_val      fixup_vma      fixup_tgt_lma  "
                "fixup_tgt_vma");
    }

    ret = uld_rofixup_apply_fixups(ehdr, base, mem_list, mnum,
            ULD_SECTION_FLAG_STATUS_MEM_NEEDS_FIXUP,
            ULD_SECTION_FLAG_STATUS_MEM_FIXUP_DONE,
            uld_rofixup_do_mem_fixup, &ud);

    return ret;
}
