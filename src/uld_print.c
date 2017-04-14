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
#include "uld_dyn.h"
#include "uld_file.h"
#include "uld_load.h"
#include "uld_sal.h"
#include "util.h"


#define ULD_PRINT_MAX_SEC_NAME_LEN 20U
#define ULD_PRINT_PHDR_SPACES_LEN 17U
#define ULD_PRINT_MAX_FILE_NAME_LEN 26U


const char uld_print_section_header_str[] =
    " &shdr       shidx name                 "
    "lma      adj_lma  fileoff  faddr\n"
    "             phidx                      "
    "vma      adj_vma  size     uldflag\n";


int uld_fprint_fse_file(FILE *stream, const struct uld_file *ufile)
{
    if (!stream || !ufile || !ufile->fse) {
        return -1;
    }

    fprintf(stream, "[<%p>] %-26s %p %08x %08lx %08lx\n\n", ufile->fse->base,
            ufile->fse->name, ufile->fse->base, ufile->fse->size,
            ufile->fse->crc, ufile->fse->flags);

    return 0;
}

int uld_fprint_section(FILE *stream, const struct uld_section *section)
{
    if (!stream || !section) {
        return -1;
    }

    fprintf(stream, "[<%p>]   %2u  %-20s %p %p %p %p\n", section->shdr,
            section->shidx, section->name, section->lma, section->adjusted_lma,
            (void *)section->shdr->sh_offset, section->faddr);

    fprintf(stream, "               %2d                       "
            "%p %p %08lx %08lx\n", section->phidx,
            (void *)section->shdr->sh_addr, section->adjusted_vma,
            section->shdr->sh_size, section->flags);

    return 0;
}

int uld_fprint_section_list(FILE *stream, const struct uld_section *sec_list,
        int count, int print_hdr)
{
    int i;

    if (!stream || !sec_list) {
        return -1;
    }

    if (print_hdr) {
        fputs(uld_print_section_header_str, stream);
    }

    for (i = 0; i < count; i++) {
        uld_fprint_section(stream, &sec_list[i]);
    }

    return 0;
}

int uld_fprint_section_file(FILE *stream, const struct uld_file *ufile)
{
    unsigned int i;

    if (!stream || !ufile) {
        return -1;
    }

    fputs(uld_print_section_header_str, stream);
    for (i = 0; i < ULD_FILE_SECTION_TYPE_COUNT; i++) {
        if (!ufile->sec.s[i] || !ufile->num.n[i]) {
            continue;
        }
        uld_fprint_section_list(stream, ufile->sec.s[i],
                ufile->num.n[i], 0);
    }
    putc('\n', stream);

    return 0;
}

int uld_fprint_gdb_sym_cmd(FILE *stream, const struct uld_file *ufile)
{
    struct uld_section section;
    const struct elf32_ehdr *ehdr;
    const struct elf32_shdr *shdr;
    const struct elf32_shdr *shstrtab;
    const void *shstrtab_faddr;
    const char *name;
    int sec_type;
    int idx;
    int ret;

    if (!stream || !ufile) {
        return -1;
    }

    ehdr = ufile->fse->base;

    // Get .text section first to error out before printing anything.
    ret = uld_file_copy_sec_by_name(ufile, &section, ".text",
            ULD_SECTION_FLAG_TYPE_FLASH);
    if (ret) {
        return ret;
    }

    // GDB add-symbol-file syntax...
    // add-symbol-file <elfpath> <text_vma> [-s <sec> <vma> -s <sec> <vma> ...]
    fprintf(stream, "add-symbol-file %s 0x%p", ufile->fse->name,
            section.adjusted_vma);

    if (!ufile->sec.flash || !ufile->num.flash) {
        shstrtab = elf32_get_section_by_index(ehdr, NULL,
                (int)ehdr->e_shstrndx);
        shstrtab_faddr = elf32_get_section_flash_addr(shstrtab,
                (const void *)ehdr);

        elf32_for_each_shdr(shdr, idx, ehdr) {
            name = elf32_get_section_name_shstrtab_faddr(shdr, shstrtab_faddr);
            sec_type = uld_section_get_type(shdr, name);
            if (!(sec_type & ULD_SECTION_FLAG_TYPE_FLASH)) {
                continue;
            }
            if (!strcmp(name, ".text")) {
                continue;
            }
            uld_load_create_section(&section, ehdr, NULL, shdr, 0,
                    shstrtab_faddr);
            fprintf(stream, " -s %s 0x%p", section.name, section.adjusted_vma);
        }
    } else {
        for (idx = 0; idx < ufile->num.flash; idx++) {
            if (strcmp(ufile->sec.flash[idx].name, ".text")) {
                fprintf(stream, " -s %s 0x%p", ufile->sec.flash[idx].name,
                        ufile->sec.flash[idx].adjusted_vma);
            }
        }
    }

    for (idx = 0; idx < ufile->num.mem; idx++) {
        fprintf(stream, " -s %s 0x%p", ufile->sec.mem[idx].name,
                ufile->sec.mem[idx].adjusted_vma);
    }

    putc('\n', stream);

    return 0;
}

int uld_fprint_gdb_sym_cmd_list(FILE *stream,
        const struct uld_file *ufile_list, int count)
{
    int i;

    if (!stream || !ufile_list) {
        return -1;
    }

    putc('\n', stream);

    for (i = 0; i < count; i++) {
        if (uld_fprint_gdb_sym_cmd(stream, &ufile_list[i])) {
            return -1;
        }
    }

    putc('\n', stream);

    return 0;
}

static int uld_fprint_rofixups_sec_lists(FILE *stream,
        struct uld_section * const *sec_lists, const int *sec_list_num,
        int list_count)
{
    struct uld_section *rofixup;
    uint8_t **fixup_addr;
    struct uld_section *fixup_tgt_sec;
    const char *fixup_tgt_name;

    rofixup = uld_section_find_in_lists_by_name(sec_lists, sec_list_num,
            list_count, ".rofixup");
    if (!rofixup) {
        return -1;
    }

    fputs("Fixup table:\n", stream);
    fputs("  fixup_addr    *fixup_addr   fixup_sec\n", stream);
    for (fixup_addr = (uint8_t **)rofixup->adjusted_lma;
            fixup_addr < (uint8_t **)(rofixup->adjusted_lma +
            rofixup->shdr->sh_size); fixup_addr++) {
        fixup_tgt_sec = uld_section_find_in_lists_by_lma(sec_lists,
                sec_list_num, list_count, *fixup_addr);
        if (fixup_tgt_sec && fixup_tgt_sec->name) {
            fixup_tgt_name = fixup_tgt_sec->name;
        } else {
            fixup_tgt_name = "UNKNOWN";
        }

        fprintf(stream, "  %p      %p      %s\n", fixup_addr, *fixup_addr,
                fixup_tgt_name);
    }

    return 0;
}

int uld_fprint_rofixups_ufile(FILE *stream, const struct uld_file *ufile)
{

    return uld_fprint_rofixups_sec_lists(stream, ufile->sec.s, ufile->num.n,
            ULD_FILE_SECTION_TYPE_COUNT);
}

int uld_fprint_rofixups_ehdr(FILE *stream, const struct elf32_ehdr *ehdr,
        const void *base)
{
    struct uld_section sec_list[ULD_FILE_SECTION_MAX] = {};
    struct uld_section *sec_lists[1] = {sec_list};
    int snum = sizeof(sec_list) / sizeof(struct uld_section);

    snum = uld_load_create_sec_list(ehdr, base, sec_list, snum,
            ULD_SECTION_FLAG_TYPE_ALL);
    if (snum > 0) {
        uld_fprint_rofixups_sec_lists(stream, sec_lists, &snum, 1);
    }

    return 0;
}

int uld_fprint_dyn(FILE *stream, const struct uld_file *ufile, int all)
{
    struct uld_section *dyn_sec;
    struct uld_section *dynstr_sec;
    const struct elf32_dyn *dyn;
    const char *type_ptr;
    const char *val_ptr;
    char type_hex[UTIL_MIN_UTOHEX_BUF + 1];  // Two extra chars for ).
    char val_hex[UTIL_MIN_UTOHEX_BUF];

    if (!ufile) {
        return -1;
    }

    dyn_sec = uld_file_get_sec_dynamic(ufile);
    dynstr_sec = uld_file_get_sec_dynstr(ufile);

    if (!dyn_sec || !dynstr_sec) {
        return -1;
    }

    fputs("Dynamic table:\n", stream);
    fputs(" dyn          tag       type                  val\n", stream);
    uld_dyn_for_each_dyn_sec(dyn, dyn_sec) {
        switch (dyn->d_tag) {
        case DT_NULL:
            type_ptr = "NULL)";
            val_ptr = "00000000";
            break;

        case DT_NEEDED:
            type_ptr = "NEEDED)";
            val_ptr = ((const char *)dynstr_sec->adjusted_lma) +
                    dyn->d_un.d_val;
            break;

        default:
            if (all) {
                utohex_pad(type_hex, dyn->d_tag, 8);
                type_hex[UTIL_MIN_UTOHEX_BUF - 1] = ')';
                type_hex[UTIL_MIN_UTOHEX_BUF] = '\0';
                utohex_pad(val_hex, dyn->d_un.d_val, 8);
                type_ptr = type_hex;
                val_ptr = val_hex;
            } else {
                type_ptr = NULL;
            }
        }
        if (type_ptr) {
            fprintf(stream, "[<%p>]  %08lx  (%-20s %s\n", dyn, dyn->d_tag,
                    type_ptr, val_ptr);
        }
    }

    return 0;
}

int uld_fprint_dynsym(FILE *stream, const struct uld_file *ufile)
{
    struct uld_section *dynsym_sec;
    struct uld_section *dynstr_sec;
    const struct elf32_sym *sym;
    const char *type_ptr;
    const char *bind_ptr;
    const char *vis_ptr;
    const char *ndx_ptr;
    unsigned int idx;
    int bytes;
    char type_dec[UTIL_MIN_UITOA_BUF];
    char bind_dec[UTIL_MIN_UITOA_BUF];
    char vis_dec[UTIL_MIN_UITOA_BUF];
    char ndx_dec[UTIL_MIN_UITOA_BUF];

    if (!ufile) {
        return -1;
    }

    dynsym_sec = uld_file_get_sec_dynsym(ufile);
    dynstr_sec = uld_file_get_sec_dynstr(ufile);

    if (!dynsym_sec || !dynstr_sec) {
        return -1;
    }

    fputs("Dynamic symbol table:\n", stream);
    fputs("  idx: addr      size type    bind   vis      ndx name\n", stream);
    uld_dyn_for_each_dynsym_sec(sym, idx, dynsym_sec) {
        switch (ELF32_ST_TYPE(sym->st_info)) {
        case STT_NOTYPE:
            type_ptr = "NOTYPE";
            break;

        case STT_OBJECT:
            type_ptr = "OBJECT";
            break;

        case STT_FUNC:
            type_ptr = "FUNC";
            break;

        case STT_SECTION:
            type_ptr = "SECTION";
            break;

        default:
            bytes = uitoa(type_dec, ELF32_ST_TYPE(sym->st_info), 0);
            type_ptr = &type_dec[UTIL_MIN_UITOA_BUF - bytes - 1];
            break;
        }

        switch (ELF32_ST_BIND(sym->st_info)) {
        case STB_LOCAL:
            bind_ptr = "LOCAL";
            break;

        case STB_GLOBAL:
            bind_ptr = "GLOBAL";
            break;

        default:
            bytes = uitoa(bind_dec, ELF32_ST_BIND(sym->st_info), 0);
            bind_ptr = &bind_dec[UTIL_MIN_UITOA_BUF - bytes - 1];
            break;
        }

        switch (ELF32_ST_VISIBILITY(sym->st_other)) {
        case STV_DEFAULT:
            vis_ptr = "DEFAULT";
            break;

        default:
            bytes = uitoa(vis_dec, ELF32_ST_VISIBILITY(sym->st_other), 0);
            vis_ptr = &vis_dec[UTIL_MIN_UITOA_BUF - bytes - 1];
            break;
        }

        switch (sym->st_shndx) {
        case SHN_UNDEF:
            ndx_ptr = "UND";
            break;

        case SHN_ABS:
            ndx_ptr = "ABS";
            break;

        default:
            bytes = uitoa(ndx_dec, sym->st_shndx, 0);
            ndx_ptr = &ndx_dec[UTIL_MIN_UITOA_BUF - bytes - 1];
            break;
        }

        fprintf(stream, "%5d: %08lx %5d %-7s %-6s %-8s %3s %s\n",
                idx, sym->st_value, (int)sym->st_size, type_ptr, bind_ptr,
                vis_ptr, ndx_ptr,
                uld_dyn_get_sym_name(sym, dynstr_sec));
    }

    return 0;
}

int uld_fprint_rel_dyn(FILE *stream, const struct uld_file *ufile)
{
    struct uld_section *rel_dyn_sec;
    struct uld_section *dynsym_sec;
    struct uld_section *dynstr_sec;
    struct uld_section *section;
    const struct elf32_rel *rel;
    const struct elf32_sym *sym;
    const char *type_ptr;
    const char *sym_sec_name;
    const uint32_t *off_vma;
    uint32_t off_val;
    unsigned int idx;
    char type_hex[UTIL_MIN_UTOHEX_BUF];

    if (!ufile) {
        return -1;
    }

    rel_dyn_sec = uld_file_get_sec_rel_dyn(ufile);
    dynsym_sec = uld_file_get_sec_dynsym(ufile);
    dynstr_sec = uld_file_get_sec_dynstr(ufile);

    if (!rel_dyn_sec || !dynsym_sec || !dynstr_sec) {
        return -1;
    }

    fputs("Relocation table:\n", stream);
    fputs(" rel         info     type              name (sec)\n", stream);
    fputs("             off      off_vma  value    off_sec\n", stream);
    uld_dyn_for_each_rel_dyn_sec(rel, idx, rel_dyn_sec) {
        switch (ELF32_R_TYPE(rel->r_info)) {
        case R_ARM_ABS32:
            type_ptr = "ABS32";
            break;

        case R_ARM_RELATIVE:
            type_ptr = "RELATIVE";
            break;

        case R_ARM_GLOB_DAT:
            type_ptr = "GLOB_DAT";
            break;

        case R_ARM_GOTFUNCDESC:
            type_ptr = "GOTFUNCDESC";
            break;

        case R_ARM_GOTOFFFUNCDESC:
            type_ptr = "GOTOFFFUNCDESC";
            break;

        case R_ARM_FUNCDESC:
            type_ptr = "FUNCDESC";
            break;

        case R_ARM_FUNCDESC_VALUE:
            type_ptr = "FUNCDESC_VALUE";
            break;

        default:
            utohex_pad(type_hex, ELF32_R_TYPE(rel->r_info), 8);
            type_ptr = type_hex;
            break;
        }

        sym = uld_dyn_get_dynsym_by_index_sec(ELF32_R_SYM(rel->r_info),
                dynsym_sec);

        switch (sym->st_shndx) {
        case SHN_UNDEF:
            sym_sec_name = "UND";
            break;

        case SHN_ABS:
            sym_sec_name = "ABS";
            break;

        default:
            section = uld_file_get_sec_by_index(ufile, sym->st_shndx,
                    ULD_SECTION_FLAG_TYPE_ALL);
            if (section) {
                sym_sec_name = section->name;
            } else {
                sym_sec_name = "UNKNOWN";
            }
            break;
        }

        fprintf(stream, "[<%p>] %08lx %-17s %s (%s)\n", rel,
                rel->r_info, type_ptr, uld_dyn_get_sym_name(sym, dynstr_sec),
                sym_sec_name);


        section = uld_section_find_in_lists_by_lma(ufile->sec.s, ufile->num.n,
            ULD_FILE_SECTION_TYPE_COUNT, (const void *)rel->r_offset);
        //if (section &&
        //        ((section->shdr->sh_type != SHT_NOBITS) ||
        //        (section->flags & ULD_SECTION_FLAG_STATUS_MEM_LOADED))) {
        //    // NOBITS and section not loaded value must be zero.  Otherwise
        //    // current vma should be currently correct if section is loaded
        //    // or not.
        //    off_vma = ((const void *)rel->r_offset -
        //            section->lma + section->adjusted_vma);
        //    off_val = *off_vma;
        //} else {
        //    off_vma = 0;
        //    off_val = 0;
        //}
        if (section) {
            off_vma = uld_section_lma_to_adjusted_vma(section,
                    (const void *)rel->r_offset);
        } else {
            off_vma = 0;
        }
        off_val = off_vma ? *off_vma : 0;

        fprintf(stream, "             %08lx %p %08lx %s\n", rel->r_offset,
                off_vma, off_val, section ? section->name : "UNKNOWN");
    }

    return 0;
}
