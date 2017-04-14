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
#include "uld_rofixup.h"
#include "uld_sal.h"
#include "util.h"


extern uint8_t __bss_end__;


int uld_load_create_section(struct uld_section *section,
        const struct elf32_ehdr *ehdr, const void *base,
        const struct elf32_shdr *shdr, int shidx,
        const void *shstrtab_faddr)
{
    int ret;

    if (!section || !ehdr || !shdr) {
        return -1;
    }

    section->flags = ULD_SECTION_FLAG_NONE;

    if (!base) {
        base = (const void *)ehdr;
    }

    if (shidx < 0 || shidx >= ehdr->e_shnum) {
        shidx = elf32_get_shidx_by_section(ehdr, base, shdr);
        if (shidx < 0) {
            return -1;
        }
    }
    section->shidx = shidx;

    if (!shstrtab_faddr) {
        section->name = elf32_get_section_name(ehdr, base, shdr);
    } else {
        section->name = elf32_get_section_name_shstrtab_faddr(shdr,
                shstrtab_faddr);
    }
    if (!section->name) {
        return -1;
    }

    ret = elf32_get_segment_by_section(ehdr, shdr, &section->phidx,
            &section->phdr);
    if (ret < 0) {
        return ret;
    }

    ret = elf32_get_section_lma(ehdr, shdr, &section->lma, NULL);
    if (ret < 0) {
        return ret;
    }

    section->adjusted_lma = elf32_adjust_lma(shdr, base);
    section->adjusted_vma = elf32_adjust_vma(shdr, base);

    section->faddr = elf32_get_section_flash_addr(shdr, base);

    section->ehdr = ehdr;
    section->shdr = shdr;

    section->flags |= ULD_SECTION_FLAG_VALID |
            uld_section_get_type(section->shdr, section->name);

    if (uld_section_is_mem_sec_fixup_by_name(section->name)) {
        section->flags |= ULD_SECTION_FLAG_STATUS_MEM_NEEDS_FIXUP;
    }

    return 0;
}

int uld_load_get_sec_count(const struct elf32_ehdr *ehdr,
        const void *base, uint32_t type_mask)
{
    const struct elf32_shdr *shstrtab;
    const void *shstrtab_faddr;
    const struct elf32_shdr *shdr;
    const char *name;
    int idx;
    int count = 0;

    if (!ehdr) {
        return -1;
    }

    shstrtab = elf32_get_section_by_index(ehdr, base, (int)ehdr->e_shstrndx);
    shstrtab_faddr = elf32_get_section_flash_addr(shstrtab,
            (const void *)ehdr);

    elf32_for_each_shdr_base(shdr, idx, ehdr, base) {
        name = elf32_get_section_name_shstrtab_faddr(shdr, shstrtab_faddr);
        if (uld_section_get_type(shdr, name) & type_mask) {
            count++;
        }
    }

    return count;
}

int uld_load_create_sec_list(const struct elf32_ehdr *ehdr,
        const void *base, struct uld_section *sec_list, int snum,
        uint32_t type_mask)
{
    const struct elf32_shdr *shdr;
    const struct elf32_shdr *shstrtab;
    const void *shstrtab_faddr;
    const char *name;
    int sec_type;
    int sec_count = 0;
    int idx;
    int ins_at;
    int ins_at_type;

    if (!ehdr || !sec_list || snum <= 0) {
        return -1;
    }

    shstrtab = elf32_get_section_by_index(ehdr, NULL, (int)ehdr->e_shstrndx);
    shstrtab_faddr = elf32_get_section_flash_addr(shstrtab,
            (const void *)ehdr);

    elf32_for_each_shdr(shdr, idx, ehdr) {
        name = elf32_get_section_name_shstrtab_faddr(shdr, shstrtab_faddr);
        sec_type = uld_section_get_type(shdr, name);

        if (!(sec_type & type_mask)) {
            continue;
        }

        // Find the insertion index for the new section to keep the section
        // type then vma in ascending order.  This is just using the vma as
        // defined in the section header (i.e. not adjusted for flash/memory
        // base).  If the section's vma is 0 (typ non-alloc sections) it
        // will be added to the end of it's type list and added in ascending
        // order by offset.
        for (ins_at = 0; ins_at < sec_count; ins_at++) {
            ins_at_type = sec_list[ins_at].flags & ULD_SECTION_FLAG_TYPE_MASK;
            if (sec_type < ins_at_type) {
                break;
            } else if (sec_type == ins_at_type) {
                if (shdr->sh_addr &&
                        (shdr->sh_addr < sec_list[ins_at].shdr->sh_addr)) {
                    break;
                } else if (!shdr->sh_addr &&
                        (shdr->sh_offset < sec_list[ins_at].shdr->sh_offset)) {
                    break;
                }
            }
        }

        if (ins_at < sec_count) {
            memmove(&sec_list[ins_at + 1], &sec_list[ins_at],
                    sizeof(struct uld_section) * (sec_count - ins_at));
        }

        uld_load_create_section(&sec_list[ins_at], ehdr, base, shdr,
                idx, shstrtab_faddr);

        sec_count++;

        // sec_list is full.
        if (sec_count == snum) {
            break;
        }
    }

    if (sec_count) {
        sec_list[sec_count - 1].flags |= ULD_SECTION_FLAG_LAST;
    }

    return sec_count;
}

uint8_t *uld_load_get_next_membase(uint8_t *last_membase, size_t last_memsize)
{
    if (last_membase >= &__bss_end__) {
        last_membase += last_memsize;
    } else {
        last_membase = &__bss_end__;
    }

    last_membase = ALIGN_PTR(last_membase, ULD_LOAD_MEMBASE_ALIGNMENT);

    return last_membase;
}

int uld_load_alloc_mem_sections(const struct elf32_ehdr *ehdr,
        const void *base, uint8_t *membase, size_t *allocated,
        struct uld_section *mem_list, int mnum)
{
    uint8_t *ptr;
    long diff;
    int i;

    if (!ehdr || !membase || !allocated || !mem_list || mnum <= 0) {
        return -1;
    }

    ptr = membase;

    for (i = 0; i < mnum; i++) {
        if (!(mem_list[i].flags & ULD_SECTION_FLAG_TYPE_MEM)) {
            printf("Warning: %s in alloc list but mem flag not set\n",
                    mem_list[i].name);
            continue;
        }

        // For simplicity require all mem section to be in the same segment.
        // If not we could potentially have different vma_offets per section.
        if (mem_list[i].phidx != mem_list[0].phidx) {
            printf("Error: expected segment: %d section %s in %d\n",
                    mem_list[0].phidx, mem_list[i].name,
                    mem_list[i].phidx);
            swbkpt();
        }

        if (i) {
            // NOBITS sections will always be last in a segment but will
            // have the same fileoff (and 0 adjusted_lma).  If a CONTENTS
            // section follows NOBITS ld will place it in a new segment
            // which is restricted above.
            if (mem_list[i - 1].shdr->sh_type != SHT_NOBITS) {
                // Use file offsets to determine gap/overop of adjacent
                // sections.
                diff = mem_list[i].shdr->sh_offset -
                        mem_list[i - 1].shdr->sh_offset -
                        mem_list[i - 1].shdr->sh_size;

                if (diff > 0) {
                    // Warn and adjust if the mem sections are not adjacent to
                    // each other.
                    membase += diff;
                    printf("Warning %u unused bytes between sections "
                            "%s and %s\n", (unsigned int)diff,
                            mem_list[i - 1].name, mem_list[i].name);
                } else if (diff < 0) {
                    printf("Error sections %s and %s overlap\n",
                            mem_list[i - 1].name, mem_list[i].name);
                    swbkpt();
                }
            }
        }

        // All sections not type NOBITS contain data that needs
        // to be copied into memory.
        if (mem_list[i].shdr->sh_type != SHT_NOBITS) {
            memcpy(ptr, mem_list[i].adjusted_lma,
                    mem_list[i].shdr->sh_size);
        } else {
            // For NOBITS (.bss like) sections C and ELF expects them
            // to be zeroed out.
            memset(ptr, 0, mem_list[i].shdr->sh_size);
        }

        mem_list[i].adjusted_vma = ptr;
        mem_list[i].flags |= ULD_SECTION_FLAG_STATUS_MEM_LOADED;

        ptr += mem_list[i].shdr->sh_size;

    }

    i--;
    *allocated = ptr - membase;

    diff = (mem_list[0].phdr->p_vaddr + mem_list[0].phdr->p_memsz) -
            (mem_list[0].shdr->sh_addr + *allocated);
    if (diff) {
        printf("Warning: end of phdr is %d bytes ahead of end of "
                "mem sections\n", (int)diff);
    }

    return 0;
}

int uld_load_create_file(const struct uld_fs_entry *fse,
        struct uld_section *sec_list, int snum, uint32_t type_mask,
        struct uld_file *ufile)
{
    const struct elf32_ehdr *ehdr;
    static const int sec_idx_lut[ULD_FILE_SECTION_TYPE_COUNT] = {
        ULD_FILE_SECTION_IDX_MEM,
        ULD_FILE_SECTION_IDX_FLASH,
        ULD_FILE_SECTION_IDX_OTHER,
    };
    int i;
    int sec_idx;
    int rnum;

    if (!fse || !sec_list || snum <= 0 || !ufile) {
        return -1;
    }

    memset(ufile, 0, sizeof(struct uld_file));

    ehdr = (const struct elf32_ehdr*)fse->base;

    ufile->fse = fse;
    ufile->flags = ULD_FILE_FLAG_NONE;

    switch (ehdr->e_type) {
    case ET_EXEC:
        ufile->flags |= ULD_FILE_FLAG_EXEC;
        break;

    default:
        break;
    }

    // Use sec_idx_lut to create mem sections first as they are required to
    // load the file.
    for (i = 0; i < ULD_FILE_SECTION_TYPE_COUNT; i++) {
        sec_idx = sec_idx_lut[i];
        if (!(type_mask & uld_file_idx_to_sec_type(sec_idx))) {
            continue;
        }

        rnum = uld_load_create_sec_list(ehdr, NULL, sec_list, snum,
                uld_file_idx_to_sec_type(sec_idx));
        if (rnum > 0) {
            ufile->sec.s[sec_idx] = sec_list;
            ufile->num.n[sec_idx] = rnum;
            snum -= rnum;
            sec_list += rnum;
        }
    }

    return 0;
}

int uld_load_file(const struct uld_fs_entry *fse,
        struct uld_section *sec_list, int snum, uint32_t type_mask,
        uint8_t **membase, size_t *allocated, struct uld_file *ufile)
{
    int ret;

    if (!membase || !allocated ) {
        return -1;
    }

    ret = uld_load_create_file(fse, sec_list, snum, type_mask, ufile);
    if (ret) {
        return ret;
    }

    if (ufile->num.mem) {
        ufile->membase = uld_load_get_next_membase(*membase, *allocated);
        *membase = ufile->membase;

        ret = uld_load_alloc_mem_sections((const struct elf32_ehdr*)fse->base,
                NULL, *membase, allocated, ufile->sec.mem, ufile->num.mem);
        if (ret) {
            return ret;
        }
        ufile->memsz = *allocated;

        ret = uld_rofixup_apply_mem_fixups((const struct elf32_ehdr*)fse->base,
                NULL, ufile->sec.flash, ufile->num.flash,
                ufile->sec.mem, ufile->num.mem, (uint32_t)ufile->membase);

        if (ret) {
            return ret;
        }
    }

    return ret;
}
