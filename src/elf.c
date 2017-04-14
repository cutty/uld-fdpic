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


const struct elf32_phdr *elf32_get_segment_by_index(
        const struct elf32_ehdr *ehdr, const void *base, int index)
{
    const uint8_t *p;

    if (!ehdr || index >= ehdr->e_phnum || index < 0) {
        return NULL;
    }

    if (base) {
        p = (const uint8_t *)base;
    } else {
        p = (const uint8_t *)ehdr;
    }

    p += ehdr->e_phoff + (ehdr->e_phentsize * index);
    return (const struct elf32_phdr *)p;
}


const struct elf32_shdr *elf32_get_section_by_index(
        const struct elf32_ehdr *ehdr, const void *base, int index)
{
    const uint8_t *p;

    if (!ehdr || index >= ehdr->e_shnum || index < 0) {
        return NULL;
    }

    if (base) {
        p = (const uint8_t *)base;
    } else {
        p = (const uint8_t *)ehdr;
    }

    p += ehdr->e_shoff + (ehdr->e_shentsize * index);
    return (const struct elf32_shdr *)p;
}

const struct elf32_shdr *elf32_get_section_by_vma(
        const struct elf32_ehdr *ehdr, const void *base, const void *vma)
{
    const struct elf32_shdr *shdr;
    int idx;

    if (!ehdr) {
        return NULL;
    }

    elf32_for_each_shdr_base(shdr, idx, ehdr, base) {
        if (vma >= (void *)shdr->sh_addr &&
                vma < (void *)(shdr->sh_addr + shdr->sh_size)) {
            return shdr;
        }
    }

    return NULL;
}

int elf32_get_shidx_by_section(const struct elf32_ehdr *ehdr,
        const void *base, const struct elf32_shdr *shdr)
{
    const uint8_t *p;

    if (!ehdr || !shdr) {
        return -1;
    }

    if (base) {
        p = (const uint8_t *)base;
    } else {
        p = (const uint8_t *)ehdr;
    }

    return (int)((const uint8_t *)shdr - p - ehdr->e_shoff) /
            ehdr->e_shentsize;
}

const void *elf32_get_section_offset(const struct elf32_shdr *shdr)
{
    if (shdr) {
        return (const void *)shdr->sh_offset;
    }

    return NULL;
}

const void *elf32_get_section_flash_addr(const struct elf32_shdr *shdr,
        const void *base)
{
    const uint8_t *p;

    if (shdr && base) {
        p = (const uint8_t *)base;
        return (const void *)(p + shdr->sh_offset);
    }

    return NULL;
}

int elf32_get_segment_by_section(const struct elf32_ehdr *ehdr,
        const struct elf32_shdr *shdr, int *phidx,
        const struct elf32_phdr **phdr)
{
    const struct elf32_phdr *pos;
    int idx;
    int ret = 1;

    if (!ehdr || !shdr) {
        return -1;
    }

    if (!phidx && !phdr) {
        return -1;
    }

    if (shdr->sh_flags & SHF_ALLOC) {
        elf32_for_each_phdr(pos, idx, ehdr) {
            // Currently only matching with loadable segments.
            if (pos->p_type != PT_LOAD) {
                continue;
            }

            // Check section VMA is outside segment VMA.
            if ((shdr->sh_addr < pos->p_vaddr) ||
                    ((shdr->sh_addr + shdr->sh_size) >
                    (pos->p_vaddr + pos->p_memsz))) {
                continue;
            }

            // If section type is not NOBITS check if section file offset is
            // outside segment file offset.
            if (shdr->sh_type != SHT_NOBITS &&
                    (shdr->sh_offset < pos->p_offset) &&
                    ((shdr->sh_offset + shdr->sh_size) >
                    (pos->p_offset + pos->p_filesz))) {
                continue;
            }

            // Segment found.
            ret = 0;
            break;
        }
    }

    // Segment not found.
    if (ret) {
        idx = ELF_SEC_HAS_NO_SEGMENT;
        pos = NULL;
    }

    if (phidx) {
        *phidx = idx;
    }
    if (phdr) {
        *phdr = pos;
    }

    return ret;
}

int elf32_get_section_lma(const struct elf32_ehdr *ehdr,
        const struct elf32_shdr *shdr, const void **lma, int *phidx)
{
    const struct elf32_phdr *phdr;
    int idx;
    int ret;

    if (!ehdr || !shdr || !lma) {
        return -1;
    }

    ret = elf32_get_segment_by_section(ehdr, shdr, &idx, &phdr);

    switch (ret) {
    // Error.
    case -1:
        return -1;
        break;

    // Segment found, find lma based on section type.
    case 0:
        if (shdr->sh_type != SHT_NOBITS) {
            // For loadable sections (has contents in file)
            // lma = section file offset - segment file offset +
            //   segment physical addr.
            *lma = (const void *)(shdr->sh_offset - phdr->p_offset +
                    phdr->p_paddr);
        } else {
            // For non-loadable sections (has no contents in file)
            // lma = section virtual addr - segment virtual addr +
            //   segment physical addr.
            *lma = (const void *)(shdr->sh_addr - phdr->p_vaddr +
                    phdr->p_paddr);
        }
        break;

    // Segment not found, set lma to section virtual address.
    case 1:
        *lma = (const void *)shdr->sh_addr;
        break;

    default:
        return -1;
        break;
    }

    if (phidx) {
        *phidx = idx;
    }

    return ret;
}

int elf32_get_section_adjusted_lma(const struct elf32_ehdr *ehdr,
        const void *base, const struct elf32_shdr *shdr, const void **lma,
        int *phidx)
{
    const uint8_t *p;
    int ret;

    if (base) {
        p = (const uint8_t *)base;
    } else {
        p = (const uint8_t *)ehdr;
    }

    ret = elf32_get_section_lma(ehdr, shdr, lma, phidx);
    if (ret != 0) {
        return ret;
    }

    *lma = elf32_adjust_lma(shdr, p);

    return 0;
}

const char *elf32_get_section_name_shstrtab_faddr(
        const struct elf32_shdr *shdr, const void *shstrtab_faddr)
{
    if (!shdr || !shstrtab_faddr) {
        return NULL;
    }

    return (const char *)shstrtab_faddr + shdr->sh_name;
}

const char *elf32_get_section_name_shstrtab(const void *base,
        const struct elf32_shdr *shdr, const struct elf32_shdr *shstrtab)
{
    const void *shstrtab_faddr;

    shstrtab_faddr = elf32_get_section_flash_addr(shstrtab, base);
    if (!shstrtab_faddr) {
        return NULL;
    }
    return elf32_get_section_name_shstrtab_faddr(shdr, shstrtab_faddr);
}

const char *elf32_get_section_name(const struct elf32_ehdr *ehdr,
        const void *base, const struct elf32_shdr *shdr)
{
    const struct elf32_shdr *shstrtab;
    const void *shstrtab_faddr;

    if (!ehdr) {
        return NULL;
    }

    if (!base) {
        base = (const void *)ehdr;
    }

    // Will perform the other error checks.
    shstrtab = elf32_get_section_by_index(ehdr, base, ehdr->e_shstrndx);
    if (!shstrtab) {
        return NULL;
    }

    shstrtab_faddr = elf32_get_section_flash_addr(shstrtab, base);
    if (!shstrtab_faddr) {
        return NULL;
    }

    return elf32_get_section_name_shstrtab_faddr(shdr, shstrtab_faddr);
}

const char *elf32_get_section_name_by_index(const struct elf32_ehdr *ehdr,
        const void *base, int index)
{
    const struct elf32_shdr *shstrtab;
    const struct elf32_shdr *shdr;
    const void *shstrtab_faddr;

    if (!ehdr) {
        return NULL;
    }

    if (!base) {
        base = (const void *)ehdr;
    }

    // Will perform the other error checks.
    shdr = elf32_get_section_by_index(ehdr, base, index);
    shstrtab = elf32_get_section_by_index(ehdr, base, ehdr->e_shstrndx);
    if (!shdr || !shstrtab) {
        return NULL;
    }

    shstrtab_faddr = elf32_get_section_flash_addr(shstrtab, base);
    if (!shstrtab_faddr) {
        return NULL;
    }

    return elf32_get_section_name_shstrtab_faddr(shdr, shstrtab_faddr);
}

const struct elf32_shdr *elf32_get_section_by_name_shstrtab_faddr(
        const struct elf32_ehdr *ehdr, const void *base, const char *name,
        const void *shstrtab_faddr)
{
    const struct elf32_shdr *pos;
    const char *s;
    int idx;

    if (!name) {
        return NULL;
    }

    elf32_for_each_shdr_base(pos, idx, ehdr, base) {
        s = elf32_get_section_name_shstrtab_faddr(pos, shstrtab_faddr);
        if (!s) {
            continue;
        }
        if (!strcmp(s, name)) {
            return pos;
        }
    }

    return NULL;
}

const struct elf32_shdr *elf32_get_section_by_name_shstrtab(
        const struct elf32_ehdr *ehdr, const void *base, const char *name,
        const struct elf32_shdr *shstrtab)
{
    const struct elf32_shdr *pos;
    const char *s;
    int idx;

    if (!name) {
        return NULL;
    }

    elf32_for_each_shdr_base(pos, idx, ehdr, base) {
        s = elf32_get_section_name_shstrtab(ehdr, pos, shstrtab);
        if (!s) {
            continue;
        }
        if (!strcmp(s, name)) {
            return pos;
        }
    }

    return NULL;
}

const struct elf32_shdr *elf32_get_section_by_name(
        const struct elf32_ehdr *ehdr, const void *base, const char *name)
{
    const struct elf32_shdr *pos;
    const char *s;
    int idx;

    if (!name) {
        return NULL;
    }

    elf32_for_each_shdr_base(pos, idx, ehdr, base) {
        s = elf32_get_section_name(ehdr, NULL, pos);
        if (!s) {
            continue;
        }
        if (!strcmp(s, name)) {
            return pos;
        }
    }

    return NULL;
}

const void *elf32_get_adjusted_entry(const struct elf32_ehdr *ehdr,
        const void *base)
{
    const struct elf32_shdr *shdr;
    const void *entry = NULL;

    if (!ehdr) {
        return NULL;
    }

    if (!base) {
        base = (const void *)ehdr;
    }

    shdr = elf32_get_section_by_vma(ehdr, base, (void *)ehdr->e_entry);
    if (shdr) {
        entry = elf32_adjust_vma(shdr, base) + ehdr->e_entry - shdr->sh_addr;
    }

    return entry;
}
