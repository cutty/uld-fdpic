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

#include <alloca.h>

#include "uld.h"
#include "uld_dyn.h"
#include "uld_exec.h"
#include "uld_file.h"
#include "uld_fs.h"
#include "uld_load.h"
#include "uld_sal.h"


#if ULD_DYN_VERBOSE == 1
int uld_dyn_verbose;
#define DYN_VERBOSE_ENABLE() \
    do { \
        uld_dyn_verbose = 1; \
    } while (0)
#define DYN_VERBOSE_DISABLE() \
    do { \
        uld_dyn_verbose = 0; \
    } while (0)
#define uprintf(format, ...) \
    do { \
        if (uld_dyn_verbose) { \
            printf((format), ##__VA_ARGS__); \
        } \
    } while (0)
#else
#define DYN_VERBOSE_ENABLE()
#define DYN_VERBOSE_DISABLE()
#define uprintf(format, ...)
#endif

#define NSWBKPT_DYN
#ifndef NSWBKPT_DYN
#define swbkpt_dyn() swbkpt()
#else
#define swbkpt_dyn()
#endif


// Resolution table: (D/C = don't care)
// rel type        ptr         membase  found by  result
// ABS32           NULL        D/C      N/A       not found
// ABS32           ptr         NULL     rel       ptr to addr
// ABS32           ptr         ptr      sym       ptr to addr
// GLOB_DAT        NULL        D/C      N/A       not found (may be allocated)
// GLOB_DAT        ptr         NULL     rel       ptr to var
// GLOB_DAT        ptr         ptr      sym       ptr to var
// FUNCDESC        NULL        D/C      N/A       not found
// FUNCDESC        fdptr       NULL     rel       ptr to funcdesc_val
// FUNCDESC        fptr        ptr      sym       fptr to func / file base set
// FUNCDESC_VALUE  NULL        D/C      N/A       not found
// FUNCDESC_VALUE  fdptr       NULL     rel       ptr to funcdesc_val
// FUNCDESC_VALUE  fptr        ptr      sym       fptr to func / file base set
// FUNCDESC_VALUE  fptr        ptr      sym       when sym type STT_SECTION rel
//                                                offset is from section lma,
//                                                and adjusted by
//                                                uld_dyn_resolve_rel
//                                                resolution can be consumed
//                                                same as above.
struct uld_dyn_resolution {
    void *ptr;
    uint8_t *membase;
    const struct elf32_sym *match_sym;
    int file_idx;
};


unsigned long uld_dyn_elf_hash(const unsigned char *name)
{
    unsigned long h = 0;
    unsigned long g;

    while (*name)
    {
        h = (h << 4) + *name++;
        if ((g = h & 0xf0000000)) {
            h ^= g >> 24;
        }
        h &= ~g;
    }
    return h;
}

const struct elf32_sym *uld_dyn_find_dynsym_elf_hash_sec(const char *name,
        const struct uld_section *hash_sec,
        const struct uld_section *dynstr_sec,
        const struct uld_section *dynsym_sec)
{
    const struct elf32_sym *sym;
    Elf32_Word *hash_table;
    Elf32_Word *chains;
    const char *str;
    unsigned long hash;
    Elf32_Word nbucket;
    Elf32_Word idx;

    if (!name || !hash_sec || !dynstr_sec || !dynsym_sec) {
        return NULL;
    }

    hash_table = (Elf32_Word *)hash_sec->adjusted_lma;
    hash = uld_dyn_elf_hash((const unsigned char *)name);

    nbucket = hash_table[0];
    idx = hash % nbucket;
    chains = hash_table + nbucket + 2;
    sym = (const struct elf32_sym *)dynsym_sec->adjusted_lma;

    for (idx = hash_table[idx + 2]; idx != STN_UNDEF; idx = chains[idx]) {
        sym = ((const struct elf32_sym *)dynsym_sec->adjusted_lma) + idx;
        str = ((const char *)dynstr_sec->adjusted_lma) + sym->st_name;
        if (!strcmp(name, str)) {
            return sym;
        }
    }

    return NULL;
}

const struct elf32_sym *uld_dyn_find_dynsym_elf_hash_file(const char *name,
        const struct uld_file *ufile)
{
    const struct uld_section *hash_sec;
    const struct uld_section *dynstr_sec;
    const struct uld_section *dynsym_sec;

    if (!ufile) {
        return NULL;
    }

    hash_sec = uld_file_get_sec_hash(ufile);
    dynstr_sec = uld_file_get_sec_dynstr(ufile);
    dynsym_sec = uld_file_get_sec_dynsym(ufile);

    return uld_dyn_find_dynsym_elf_hash_sec(name, hash_sec, dynstr_sec,
            dynsym_sec);
}

const struct elf32_sym *uld_dyn_find_dynsym_linear_sec(const char *name,
        const struct uld_section *dynstr_sec,
        const struct uld_section *dynsym_sec)
{
    const struct elf32_sym *sym;
    unsigned int idx;
    const char *str;

    if (!name || !dynstr_sec || !dynsym_sec) {
        return NULL;
    }

    uld_dyn_for_each_dynsym_sec(sym, idx, dynsym_sec) {
        str = ((const char *)dynstr_sec->adjusted_lma) + sym->st_name;
        if (!strcmp(name, str)) {
            return sym;
        }
    }

    return NULL;
}

const struct elf32_sym *uld_dyn_find_dynsym_linear_file(const char *name,
        const struct uld_file *ufile)
{
    const struct uld_section *dynstr_sec;
    const struct uld_section *dynsym_sec;

    if (!ufile) {
        return NULL;
    }

    dynstr_sec = uld_file_get_sec_dynstr(ufile);
    dynsym_sec = uld_file_get_sec_dynsym(ufile);

    return uld_dyn_find_dynsym_linear_sec(name, dynstr_sec, dynsym_sec);
}

static int uld_dyn_create_fse_dep_list_get_sections(
        const struct uld_fs_entry *fse,
        struct uld_section *dyn_sec,
        struct uld_section *dynstr_sec)
{
    const struct elf32_ehdr *ehdr = fse->base;
    const struct elf32_shdr *shdr;
    const struct elf32_shdr *shstrtab;
    const void *shstrtab_faddr;
    int ret = 0;

    shstrtab = elf32_get_section_by_index(ehdr, NULL, (int)ehdr->e_shstrndx);
    shstrtab_faddr = elf32_get_section_flash_addr(shstrtab,
            (const void *)ehdr);

    shdr = elf32_get_section_by_name_shstrtab_faddr(ehdr, NULL, ".dynamic",
            shstrtab_faddr);
    if (!shdr) {
        // A .dynamic section is not necessary if the file has no DT_NEEDED
        // entries.  If this is an error it will fail later during symbol
        // resolution.
        return 1;
    }

    ret = uld_load_create_section(dyn_sec, ehdr, NULL, shdr, 0,
            shstrtab_faddr);
    if (ret) {
        return ret;
    }

    shdr = elf32_get_section_by_name_shstrtab_faddr(ehdr, NULL, ".dynstr",
            shstrtab_faddr);
    if (!shdr) {
        return -1;
    }

    ret = uld_load_create_section(dynstr_sec, ehdr, NULL, shdr, 0,
            shstrtab_faddr);

    return ret;
}

int uld_dyn_create_fse_dep_list(const struct uld_fs_entry *fse,
        const struct uld_fs_entry **dep_list, int *idx, int list_size)
{
    struct uld_section dyn_sec;
    struct uld_section dynstr_sec;
    const struct elf32_dyn *dyn;
    const char *dep_name;
    const struct uld_fs_entry *dep_fse;
    int i;
    int ret;

    // See if fse is already in dep_list early to return before iterating the
    // dynamic section.
    i = *idx;
    while (i--) {
        if (dep_list[i] == fse) {
            return 0;
        }
    }

    ret = uld_dyn_create_fse_dep_list_get_sections(fse, &dyn_sec, &dynstr_sec);
    if (ret < 0) {
        return ret;
    }

    // This file does not have a .dynamic section, skip finding dependencies.
    if (ret != 1) {
        uld_dyn_for_each_dt_needed(dyn, &dyn_sec) {
            dep_name = ((const char *)dynstr_sec.adjusted_lma) +
                    dyn->d_un.d_val;
            dep_fse = uld_fs_get_file_by_name(ULD_PSTORE->fs_table_pri.head,
                    dep_name);
            if (!dep_fse) {
                return -1;
            }

            ret = uld_dyn_create_fse_dep_list(dep_fse, dep_list, idx,
                    list_size);
            if (ret) {
                return ret;
            }
        }
    }

    // About to add this fse, check for room.
    if (*idx == list_size) {
        return -2;
    }

    dep_list[(*idx)++] = fse;

    return 0;
}

int uld_dyn_get_fse_dep_count(const struct uld_fs_entry *fse)
{
    const struct uld_fs_entry **dep_list;
    int fse_count;
    int idx;
    int ret;

    if (!fse) {
        return -1;
    }

    // Use total number of files as a simple upper bound.
    fse_count = uld_fs_get_file_count(ULD_PSTORE->fs_table_pri.head);
    dep_list = alloca(sizeof(struct uld_fs_entry) * fse_count);
    idx = 0;

    ret = uld_dyn_create_fse_dep_list(fse, dep_list, &idx, fse_count);
    if (ret) {
        return ret;
    }

    return idx;
}

int uld_dyn_get_dep_list_sec_count(const struct uld_fs_entry **dep_list,
        int dep_count, uint32_t type_mask)
{
    int count = 0;

    while (dep_count--) {
        count += uld_load_get_sec_count(dep_list[dep_count]->base, NULL,
                type_mask);
    }

    return count;
}

int uld_dyn_load_fse_dep_list(const struct uld_fs_entry **dep_list,
        int dep_count, struct uld_file *ufile_list,
        struct uld_section *sec_list, int sec_count, uint8_t **membase,
        size_t *allocated)
{
    uint8_t *load_file_membase;
    size_t load_file_allocated;
    int i;
    int sec_idx;
    int ret;

    if (!dep_list || dep_count < 0 || !ufile_list || !sec_list ||
            sec_count < 0 || !membase || !allocated) {
        return -1;
    }

    memset(ufile_list, 0, sizeof(struct uld_file) * dep_count);
    memset(sec_list, 0, sizeof(struct uld_section) * sec_count);

    load_file_allocated = 0;
    // Init/align membase for the first file to load.
    *membase = uld_load_get_next_membase(*membase, 0);
    load_file_membase = *membase;

    sec_idx = 0;
    for (i = 0; i < dep_count; i++) {
        ret = uld_load_file(dep_list[i], &sec_list[sec_idx],
                sec_count - sec_idx, ULD_DYN_LOAD_SECTION_TYPE_MASK,
                &load_file_membase, &load_file_allocated, &ufile_list[i]);

        if (ret) {
            swbkpt();
            return ret;
        }

        printf("loaded: %-16s mem base: 0x%p mem size: %d\n",
                dep_list[i]->name, load_file_membase, load_file_allocated);

        sec_idx += uld_file_get_sec_count(&ufile_list[i]);
    }

    *allocated = load_file_membase - *membase + load_file_allocated;

    return 0;
}

static int uld_dyn_update_relative(const struct uld_file *ufile,
        const struct elf32_rel *rel)
{
    void **rel_ptr;
    void *ptr;
    void *adj_ptr;

    rel_ptr = (void **)uld_file_lma_to_adjusted_vma(ufile,
            (void *)rel->r_offset);
    if (!rel_ptr) {
        uprintf("  Could not find vma for rel %p offset %p\n",
                rel, (void *)rel->r_offset);
        swbkpt();
        return -1;
    }

    ptr = *rel_ptr;
    adj_ptr = (void *)uld_file_lma_to_adjusted_vma(ufile, ptr);
    *rel_ptr = adj_ptr;

    uprintf("  Updating RELATIVE %p -> %p at %p\n", ptr, adj_ptr, rel_ptr);

    return 0;
}

static void uld_dyn_get_link_sections(const struct uld_file *ufile,
    struct uld_section **hash_sec, struct uld_section **rel_dyn_sec,
    struct uld_section **dynsym_sec, struct uld_section **dynstr_sec)
{
    if (hash_sec) {
        *hash_sec = uld_file_get_sec_hash(ufile);
    }
    if (rel_dyn_sec) {
        *rel_dyn_sec = uld_file_get_sec_rel_dyn(ufile);
    }
    if (dynsym_sec) {
        *dynsym_sec = uld_file_get_sec_dynsym(ufile);
    }
    if (dynstr_sec) {
        *dynstr_sec = uld_file_get_sec_dynstr(ufile);
    }
}

static int uld_dyn_resolve_rel(const struct uld_file *ufile_list,
        const struct elf32_rel *rel, int file_idx, int rd_idx,
        struct uld_dyn_resolution *res)
{
    const struct uld_file *ufile;
    struct uld_section *hash_sec;
    struct uld_section *rel_dyn_sec;
    struct uld_section *dynsym_sec;
    struct uld_section *dynstr_sec;
    struct uld_section *ret_sec;
    const struct elf32_sym *rel_sym;
    const struct elf32_sym *match_sym;
    const struct elf32_rel *search_rel;
    const char *rel_sym_name;
    void *adj_vma;
    unsigned int rel_type;
    unsigned int match_sym_type;
    unsigned int match_sym_idx;
    unsigned int search_type;

    // Initialize resolution ptr.  This can be used to defer FUNCDESC matching
    // with a symbol or existing relocation in the same file and checked
    // before returning.
    res->ptr = NULL;

    // Initialize search variables for file containing the relocation before
    // entering search the loop.  Allows searching in reverse starting
    // at the relocation directly before the one to resolve (excluding
    // the FUNCDESC exception below).
    ufile = &ufile_list[file_idx];
    uld_dyn_get_link_sections(ufile, &hash_sec, &rel_dyn_sec, &dynsym_sec,
            &dynstr_sec);

    // Get relocation sym and name to search for.
    rel_sym = uld_dyn_get_dynsym_by_index_sec(ELF32_R_SYM(rel->r_info),
            dynsym_sec);
    rel_sym_name = uld_dyn_get_sym_name(rel_sym, dynstr_sec);

    uprintf("  Attempting to resolve %s::%s rd_idx: %02d\n", ufile->fse->name,
            rel_sym_name, rd_idx);

    // Already know this is the matching dynamic symbol for this relocation.
    match_sym = rel_sym;

    // If relocation type is FUNCDESC search all relocationss in the current
    // A FUNCDESC relocation may be followed by an equivalent, but unresolved
    // FUNCDESC_VALUE.  By setting the FUNCDESC pointer to this FUNCDESC_VALUE
    // a func ptr/pic base will not need to be allocated into the dl_alloc
    // pool.
    rel_type = ELF32_R_TYPE(rel->r_info);
    if (rel_type == R_ARM_FUNCDESC) {
        uprintf("  Resetting search limits for FUNCDESC rel %p\n",
                rel);
        rd_idx = -1;
    }

    // By searching in reverse starting with the relocation before the one
    // to resolve any previous relocation or file referencing this symbol
    // will have a complete resolution if it exists.

    // Note: If a resolution for an object does not exist the dynamic linker
    // may allocate space and create one (outside of this function).  See
    // uld_dyn_write_reso_glob_dat.
    for (; file_idx >= 0; file_idx--) {
        ufile = &ufile_list[file_idx];

        // If rd_idx < 0, search entire file at file_idx in ufile_list.
        // This is may be set above to search the entire file containing
        // the relocation.
        if (rd_idx < 0) {
            uld_dyn_get_link_sections(ufile, &hash_sec, &rel_dyn_sec,
                    &dynsym_sec, &dynstr_sec);

            // A file should have these section if it is either producing
            // or consuming symbols.  If not it must have been linked
            // incorrectly at build time.  Issue a warning and skip
            // to next file.
            if (!hash_sec || !dynsym_sec || !dynstr_sec) {
                printf("  Warning: skipping %s due to missing dynamic "
                        "tables\n", ufile->fse->name);
                continue;
            }

            match_sym = uld_dyn_find_dynsym_elf_hash_sec(rel_sym_name,
                    hash_sec, dynstr_sec, dynsym_sec);

            // This file does not have a matching symbol, skip to next file.
            if (!match_sym) {
                continue;
            }
        }

        // Should not be required but ld may create a symbol reference to an
        // incorrect symbol.  Currently this error always points to
        // a SECTION type symbol. This will catch the bug before it
        // causes a problem.
        match_sym_type = ELF32_ST_TYPE(match_sym->st_info);
        switch (rel_type) {
        case R_ARM_ABS32:
            // No restriction.
            break;

        case R_ARM_GLOB_DAT:
            if (match_sym_type != STT_OBJECT) {
                printf("  Error: Expected sym type STT_OBJECT; sym %p has "
                        "%02x\n", match_sym, match_sym_type);
                swbkpt();
                return -1;
            }
            break;

        //case R_ARM_GOTFUNCDESC:
        //case R_ARM_GOTOFFFUNCDESC:
        case R_ARM_FUNCDESC:
        case R_ARM_FUNCDESC_VALUE:
            if (match_sym_type == STT_SECTION &&
                    rel_type == R_ARM_FUNCDESC_VALUE) {
                ret_sec = uld_file_get_sec_by_index(ufile,
                        match_sym->st_shndx, ULD_SECTION_FLAG_TYPE_ALL);
                if (!ret_sec) {
                    printf("  Error: invalid section index %d\n",
                        match_sym->st_shndx);
                    swbkpt();
                    return -1;
                } else {
                    // Create a resolution for
                    // uld_dyn_write_reso_funcdesc_value to consume.
                    adj_vma = (void *)uld_file_lma_to_adjusted_vma(ufile,
                            (void *)rel->r_offset);
                    res->ptr = *(uint8_t **)adj_vma +
                            (uintptr_t)ret_sec->lma;
                    res->membase = ufile->membase;
                    res->file_idx = file_idx;
                    return 0;
                }
            } else if (match_sym_type != STT_FUNC) {
                printf("  Error: Expected type STT_FUNC or STT_SECTION sym; "
                        "%p has %02x\n", match_sym, match_sym_type);
                swbkpt();
                return -1;
            }
            break;

        default:
            printf("Invalid rel type: 0x%02x\n", rel_type);
            swbkpt();
            break;
        }

        // First check if the dynamic symbol can provide a resolution.
        if (elf32_sym_has_section_index(match_sym)) {
            // Per the resolution struct values will be the same for all
            // relocation types (membase for GLOB_DAT is D/C).
            //
            // Note: in the case of FUNCDESC the caller should search
            // for these values in the dl_alloc pool before creating
            // a new one.  For simplicity this function returns first
            // found.
            uprintf("  Resolution %p - %p found via sym %p file_idx: %02d "
                    "(%s)\n", (void *)match_sym->st_value, ufile->membase,
                    match_sym, file_idx, ufile->fse->name);
            res->membase = ufile->membase;
            res->ptr = (void *)match_sym->st_value;
            res->file_idx = file_idx;

            if (rel_type == R_ARM_FUNCDESC) {
                uprintf("  Deferring resolution for FUNCDESC to search for "
                        "existing FUNCDESC_VALUE\n");
            } else {
                swbkpt_dyn();
                return 0;
            }
        } else if (rel_type == R_ARM_GLOB_DAT) {
            // For objects track the last file that had it in its dynamic
            // symbol table.
            res->file_idx = file_idx;
            res->match_sym = match_sym;
        }

        // If needed set rd_idx to size of the .rel.dyn section.
        if (rd_idx < 0) {
            if (rel_dyn_sec) {
                rd_idx = uld_dyn_get_rel_dyn_count_sec(rel_dyn_sec);
            } else {
                rd_idx = 0;
            }
        }

        // match_sym did not provide a resolution, check if another
        // relocation can.  A file can have a many-to-one relationship
        // of relocations to dynamic symbols.
        // Note: ideally we would not have to iterate other relocation tables
        // even for dl allocated values since the resolution could be
        // written into the dynamic symbol table (and found via the hash
        // table).  In this case the table is stored in flash and considered
        // read-only.
        match_sym_idx = uld_dyn_get_sym_index(match_sym, dynsym_sec);
        uprintf("  Testing relocations in file_idx: %02d starting "
                "rd_idx: %02d\n", file_idx, rd_idx - 1);
        for (rd_idx--; rd_idx >= 0; rd_idx--) {
            //uprintf("  Testing relocation file_idx: %02d rd_idx: %02d\n",
            //        file_idx, rd_idx);

            search_rel = uld_dyn_get_rel_dyn_by_index_sec(rd_idx, rel_dyn_sec);

            // Symbol reference in search relocation does not match, move on
            // to next.
            if (ELF32_R_SYM(search_rel->r_info) != match_sym_idx) {
                continue;
            }

            search_type = ELF32_R_TYPE(search_rel->r_info);

            // Additional matching rules required if searching the same file.
            if (match_sym == rel_sym) {
                // If relocation limits were reset (when relocation type is
                // FUNCDESC) and the search relocation has a greater or equal
                // index (ptr) than the relocation currently being resolved.
                if (search_rel >= rel) {
                    if (rel_type != R_ARM_FUNCDESC) {
                        printf("  Expected rel type FUNCDESC; rel %p is "
                                "%02x\n", rel, rel_type);
                        swbkpt();
                    }
                }

                // In the case stated above FUNCDESC can match against an
                // unresolved FUNCDESC_VALUE since it will be updated later
                // and never placed in the dl_alloc pool.
                //
                // To make sure the FUNCDESC_VALUE will resolved do not
                // match it against any other FUNCDESC relocations which
                // may be referencing back to this FUNCDESC_VALUE.
                if ((rel_type == R_ARM_FUNCDESC &&
                        search_type != R_ARM_FUNCDESC_VALUE) ||
                        (rel_type == R_ARM_FUNCDESC_VALUE &&
                        search_type == R_ARM_FUNCDESC)) {
                    uprintf("  Skip matching rel %p type %02x with "
                            "search_rel %p type %02x\n", rel, rel_type,
                            search_rel, search_type);
                    continue;
                }
            }

            // Per the resolution struct table define (fd)ptr and set membase
            // to NULL.
            res->membase = NULL;
            res->file_idx = file_idx;

            // For a relocation resolution we should only have to find the
            // adjusted_vma in a mem section.
            ret_sec = uld_section_find_in_list_by_lma(ufile->sec.mem,
                    ufile->num.mem, (void *)search_rel->r_offset);
            if (!ret_sec) {
                uprintf("  Could not find mem section for rel %p offset %p\n",
                        search_rel, (void *)search_rel->r_offset);
                swbkpt();
            }
            adj_vma = (void *)uld_section_lma_to_adjusted_vma(ret_sec,
                    (void *)search_rel->r_offset);

            // Use the values pointed to by the relocation not the
            // referenced symbol.
            switch (search_type) {
            case R_ARM_ABS32:
            case R_ARM_GLOB_DAT:
                // Relocation offset points to a GLOB_DAT ptr.
                res->ptr = *(void **)adj_vma;
                break;

            case R_ARM_FUNCDESC:
                // Relocation offset points to a FUNCDESC ptr.
                res->ptr = *(void **)adj_vma;
                break;

            case R_ARM_FUNCDESC_VALUE:
                // Relocation offset points to a FUNCDESC_VALUE.
                res->ptr = (void *)adj_vma;
                break;

            default:
                uprintf("  Unexpected search_rel %p type %02x\n",
                        search_rel, search_type);
                swbkpt();
                break;
            }

            uprintf("  Resolution %p found via rel %p file_idx: %02d (%s) "
                    "rd_idx: %02x\n", res->ptr, search_rel, file_idx,
                    ufile->fse->name, rd_idx);
            swbkpt_dyn();
            return 0;
        }
    }

    // A FUNCDESC search may get here with a symbol based resolution since it
    // will continue to search for an existing FUNCDESC_VALUE to spare
    // an allocation.
    if (!res->ptr) {
        // Defer error message for GLOB_DAT which may the dynamic linker
        // to allocate space for it preventing a resolution.
        // This may not be an issue due to unused variables being garbage
        // collected by the linker.  See uld_dyn_write_reso_glob_dat.
        if (rel_type != R_ARM_GLOB_DAT) {
            uprintf("  Resolution not found for %s\n", rel_sym_name);
            swbkpt();
        }
        return -1;
    }

    swbkpt_dyn();
    return 0;
}

static int uld_dyn_write_reso_abs32(const struct uld_file *ufile_list,
        int file_idx, const struct elf32_rel *rel,
        const struct uld_dyn_resolution *res)
{
    const struct uld_file *rel_ufile;
    const struct uld_file *res_ufile;
    void **rel_dst;
    void *ptr;

    rel_ufile = &ufile_list[file_idx];
    rel_dst = (void **)uld_file_lma_to_adjusted_vma(rel_ufile,
            (void *)rel->r_offset);
    if (!rel_dst) {
        uprintf("  Could not find vma for rel %p offset %p\n",
                rel, (void *)rel->r_offset);
        swbkpt();
        return -1;
    }

    res_ufile = &ufile_list[res->file_idx];
    if (res->membase) {
        ptr = (void *)uld_file_lma_to_adjusted_vma(res_ufile, res->ptr);
    } else {
        ptr = res->ptr;
    }

    *rel_dst = ptr;
    uprintf("  Wrote ABS32 %p to %p\n", ptr, rel_dst);

    return 0;
}

static int uld_dyn_write_reso_glob_dat(const struct uld_file *ufile_list,
        int file_idx, const struct elf32_rel *rel,
        const struct uld_dyn_resolution *res,
        uint8_t **dl_alloc_base, size_t *dl_alloc_size)
{
    const struct uld_file *rel_ufile;
    const struct uld_file *res_ufile;
    void **rel_dst;
    void *ptr;

    rel_ufile = &ufile_list[file_idx];
    rel_dst = (void **)uld_file_lma_to_adjusted_vma(rel_ufile,
            (void *)rel->r_offset);
    if (!rel_dst) {
        uprintf("  Could not find vma for rel %p offset %p\n",
                rel, (void *)rel->r_offset);
        swbkpt();
        return -1;
    }

    res_ufile = &ufile_list[res->file_idx];

    // The combination of -fdata-sections and --gc-sections seems to remove
    // space allocated for variables unused internally to the shared library
    // but leaves the dynamic symbols allowing linking to succeed. This only
    // seems to happen if none of the variables are referenced.  If one is
    // referenced space for all will be allocated.
    // Unfortunately this does not happen on later compilers which can not be
    // used.
    // Since everything should be linked with --no-undefined it will still
    // enforce correct that the symbol exists.
    if (!res->ptr) {
        // uld_dyn_resolve_rel tracks the last file that had a matching
        // symbol.  Check that the file index is not equal (i.e. lower) than
        // the resolution file index meaning this was linked correctly.
        if (res->file_idx == file_idx) {
            uprintf("  Undefined GLOB_DAT symbol for rel %p only referenced "
                    "by same file", rel);
            swbkpt();
        }
        // Set the resolution pointer in the dl_alloc pool and allocate
        // space for it.
        ptr = *dl_alloc_base;
        *dl_alloc_base = ALIGN_PTR(ptr + res->match_sym->st_size,
                ULD_DYN_ALLOC_ALIGNMENT);
        // Take alignment into account for real alloc size.
        *dl_alloc_size += *dl_alloc_base - (uint8_t *)ptr;

        // Zero out allocated this memory (this behavior only seems to
        // happened for bss).  Would be more serious for .data since
        // we couldn't initialize the memory correctly.
        memset(ptr, 0, (size_t)res->match_sym->st_size);

        uprintf("  Allocated %d bytes for object\n",
                (int)res->match_sym->st_size);
    } else {
        if (res->membase) {
            ptr = (void *)uld_file_lma_to_adjusted_vma(res_ufile, res->ptr);
        } else {
            ptr = res->ptr;
        }
    }

    *rel_dst = ptr;
    uprintf("  Wrote GLOB_DAT %p to %p\n", ptr, rel_dst);
    swbkpt_dyn();

    return 0;
}

static void uld_dyn_write_reso_funcdesc_value_dst(void **dst,
        const struct uld_file *ufile_list,
        const struct uld_dyn_resolution *res)
{
    const struct uld_file *res_ufile;
    void *ptr;

    res_ufile = &ufile_list[res->file_idx];

    // Going to use vma in case functions exist in memory.  This will be
    // the same as the lma for .text.
    ptr = (void *)uld_file_lma_to_adjusted_vma(res_ufile, res->ptr);
    *dst = ptr;
    *(dst + 1) = res->membase;
    uprintf("  Wrote FUNCDESC_VALUE %p - %p to %p\n", ptr, res->membase,
            dst);
    swbkpt_dyn();
}

static int uld_dyn_write_reso_funcdesc(const struct uld_file *ufile_list,
        int file_idx, const struct elf32_rel *rel,
        const struct uld_dyn_resolution *res,
        uint8_t **dl_alloc_base, size_t *dl_alloc_size)
{
    const struct uld_file *rel_ufile;
    void **rel_dst;
    void *ptr;

    rel_ufile = &ufile_list[file_idx];
    rel_dst = (void **)uld_file_lma_to_adjusted_vma(rel_ufile,
            (void *)rel->r_offset);
    if (!rel_dst) {
        uprintf("  Could not find vma for rel %p offset %p\n",
                rel, (void *)rel->r_offset);
        swbkpt();
        return -1;
    }

    if (res->membase) {
        ptr = *dl_alloc_base;

        // Assumes alignment requirement <= 8 bytes.
        *dl_alloc_base += 8;
        *dl_alloc_size += 8;
        uprintf("  Allocated 8 bytes for FUNCDESC_VALUE\n");

        uld_dyn_write_reso_funcdesc_value_dst((void **)ptr, ufile_list, res);
    } else {
        ptr = res->ptr;
    }

    *rel_dst = ptr;
    uprintf("  Wrote FUNCDESC %p to %p\n", ptr, rel_dst);
    swbkpt_dyn();

    return 0;
}

static int uld_dyn_write_reso_funcdesc_value(const struct uld_file *ufile_list,
        int file_idx, const struct elf32_rel *rel,
        const struct uld_dyn_resolution *res)
{
    const struct uld_file *rel_ufile;
    //const struct uld_file *res_ufile;
    void **rel_dst;
    //void *ptr;
    void **res_src;

    rel_ufile = &ufile_list[file_idx];
    //res_ufile = &ufile_list[res->file_idx];

    rel_dst = (void **)uld_file_lma_to_adjusted_vma(rel_ufile,
            (void *)rel->r_offset);
    if (!rel_dst) {
        uprintf("  Could not find vma for rel %p offset %p\n",
                rel, (void *)rel->r_offset);
        swbkpt();
        return -1;
    }

    if (res->membase) {
        // Going to use vma in case functions exist in memory.  This will be
        // the same as the lma for .text.
        //ptr = (void *)uld_file_lma_to_adjusted_vma(res_ufile, res->ptr);
        //*rel_dst = ptr;
        //*(rel_dst + 1) = res->membase;
        //uprintf("  Wrote FUNCDESC_VALUE %p - %p to %p\n", ptr, res->membase,
        //        rel_dst);
        uld_dyn_write_reso_funcdesc_value_dst(rel_dst, ufile_list, res);
        swbkpt_dyn();
    } else {
        res_src = (void **)res->ptr;
        *rel_dst = *res_src;
        *(rel_dst + 1) = *(res_src + 1);
        uprintf("  Wrote FUNCDESC_VALUE %p - %p to %p\n", *rel_dst,
                *(rel_dst + 1), rel_dst);
        swbkpt_dyn();
    }

    return 0;
}

int uld_dyn_link_file_list(const struct uld_file *ufile_list, int file_count,
        uint8_t *dl_alloc_base, size_t *dl_alloc_size)
{
    struct uld_dyn_resolution res;
    const struct uld_file *ufile;
    struct uld_section *rel_dyn_sec;
    struct uld_section *dynsym_sec;
    struct uld_section *dynstr_sec;
    const struct elf32_rel *rel;
    size_t dla_size;
    int file_idx;
    int ret;
    unsigned int rd_idx;

    if (!ufile_list || file_count <= 0 || !dl_alloc_base || !dl_alloc_size) {
        return -1;
    }

    dla_size = 0;

    for (file_idx = 0; file_idx < file_count; file_idx++) {
        ufile = &ufile_list[file_idx];

        uld_dyn_get_link_sections(ufile, NULL, &rel_dyn_sec, &dynsym_sec,
                &dynstr_sec);
        // File does not need any dynamic relocations, move on to next file.
        if (!rel_dyn_sec) {
            continue;
        }

        // File does have dynamic relocation, require dynamic symbol and
        // string tables.
        if (!dynsym_sec || !dynstr_sec) {
            return -1;
        }

        uld_dyn_for_each_rel_dyn_sec(rel, rd_idx, rel_dyn_sec) {
            switch (ELF32_R_TYPE(rel->r_info)) {
            case R_ARM_ABS32:
                uprintf("[<%p>] resolving ABS32          %02d:%02d\n",
                        rel, file_idx, rd_idx);
                ret = uld_dyn_resolve_rel(ufile_list, rel, file_idx,
                        rd_idx, &res);
                if (!ret) {
                    uld_dyn_write_reso_abs32(ufile_list, file_idx, rel, &res);
                }
                break;

            case R_ARM_RELATIVE:
                uprintf("[<%p>] resolving RELATIVE       %02d:%02d\n",
                        rel, file_idx, rd_idx);
                ret = uld_dyn_update_relative(ufile, rel);
                break;

            case R_ARM_GLOB_DAT:
                uprintf("[<%p>] resolving GLOB_DAT       %02d:%02d\n",
                        rel, file_idx, rd_idx);
                ret = uld_dyn_resolve_rel(ufile_list, rel, file_idx,
                        rd_idx, &res);
                ret = uld_dyn_write_reso_glob_dat(ufile_list, file_idx,
                        rel, &res, &dl_alloc_base, &dla_size);
                break;

            case R_ARM_FUNCDESC:
                uprintf("[<%p>] resolving FUNCDESC       %02d:%02d\n",
                        rel, file_idx, rd_idx);
                ret = uld_dyn_resolve_rel(ufile_list, rel, file_idx,
                        rd_idx, &res);
                if (!ret) {
                    uld_dyn_write_reso_funcdesc(ufile_list, file_idx,
                            rel, &res, &dl_alloc_base, &dla_size);
                }
                break;

            case R_ARM_FUNCDESC_VALUE:
                uprintf("[<%p>] resolving FUNCDESC_VALUE %02d:%02d\n",
                        rel, file_idx, rd_idx);
                ret = uld_dyn_resolve_rel(ufile_list, rel, file_idx,
                        rd_idx, &res);
                if (!ret) {
                    uld_dyn_write_reso_funcdesc_value(ufile_list, file_idx,
                            rel, &res);
                }
                break;

            default:
                printf("unsupported relocation type: %d\n",
                        ELF32_R_TYPE(rel->r_info));
                swbkpt();
                return -1;
                break;
            }

            // Catch all for failed resolution.
            if (ret) {
                printf("[<%p>] resolution failed %02d:%02d\n", rel, file_idx,
                        rd_idx);
                swbkpt();
                return -1;
            }
        }
    }

    swbkpt_dyn();

    return 0;
}

int uld_dyn_exec_fse(const struct uld_fs_entry *fse, void *sp_base, int argc,
        const char **argv)
{
    const struct uld_fs_entry **dep_list;
    struct uld_file *ufile_list;
    struct uld_section *sec_list;
    uint8_t *membase;
    uint8_t *dl_alloc_base;
    size_t dl_alloc_size;
    size_t allocated;
    int dep_count;
    int idx;
    int ret;
    int sec_count;
    int i;

    if (uld_verbose >= 2) {
        DYN_VERBOSE_ENABLE();
    } else {
        DYN_VERBOSE_DISABLE();
    }

    dep_count = uld_dyn_get_fse_dep_count(fse);

    dep_list = alloca(sizeof(struct uld_fs_entry *) * dep_count);
    ufile_list = alloca(sizeof(struct uld_file) * dep_count);

    idx = 0;
    ret = uld_dyn_create_fse_dep_list(fse, dep_list, &idx, dep_count);
    if (uld_verbose) {
        printf("create_fse_dep_list: %d\n", ret);
    }

    sec_count = uld_dyn_get_dep_list_sec_count(dep_list, dep_count,
        (ULD_SECTION_FLAG_TYPE_FLASH | ULD_SECTION_FLAG_TYPE_MEM |
        ULD_SECTION_FLAG_TYPE_DYNAMIC));

    sec_list = alloca(sizeof(struct uld_section) * sec_count);

    if (uld_verbose) {
        printf("processing %d sections for %d files\n", sec_count, dep_count);
    }

    membase = NULL;
    allocated = 0;
    ret = uld_dyn_load_fse_dep_list(dep_list, dep_count, ufile_list, sec_list,
        sec_count, &membase, &allocated);
    if (uld_verbose) {
        printf("load_fse_dep_list: %d\n", ret);
    }
    putchar('\n');

    if (uld_verbose) {
        for (i = 0; i < dep_count; i++) {
            printf("\nDumping %s tables:\n", ufile_list[i].fse->name);
            if (uld_verbose >= 2) {
                uld_print_section_file(&ufile_list[i]);
                uld_print_dyn(&ufile_list[i], 1);
                uld_print_dynsym(&ufile_list[i]);
            }
            uld_print_rel_dyn(&ufile_list[i]);
        }
        puts("\n");
    }

    dl_alloc_base = membase + allocated;
    ret = uld_dyn_link_file_list(ufile_list, dep_count, dl_alloc_base,
            &dl_alloc_size);
    if (uld_verbose) {
        printf("uld_dyn_link_file_list: %d\n\n", ret);
    }

    uld_print_gdb_sym_cmd_list(ufile_list, idx);

#ifdef ULD_BREAK_BEFORE_CTOR
    puts("break before ctor - gdb: uc to continue");
    swbkpt();
#endif

    for (i = 0; i < dep_count; i++) {
        uld_exec_elf_call_init_funcs(&ufile_list[i]);
    }

    uld_exec_file(&ufile_list[idx - 1], sp_base, argc, argv);

    return 0;
}
