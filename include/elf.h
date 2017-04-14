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

#ifndef _ELF_H
#define _ELF_H


typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;

#define EI_MAG0                                     0
#define EI_MAG1                                     1
#define EI_MAG2                                     2
#define EI_MAG3                                     3
#define EI_CLASS                                    4
#define EI_DATA                                     5
#define EI_VERSION                                  6
#define EI_OSABI                                    7
#define EI_ABIVERSION                               8
#define EI_PAD                                      9
#define EI_NIDENT                                   16

#define ELFMAG0                                     0x7f
#define ELFMAG1                                     'E'
#define ELFMAG2                                     'L'
#define ELFMAG3                                     'F'

#define ET_NONE                                     0
#define ET_REL                                      1
#define ET_EXEC                                     2
#define ET_DYN                                      3
#define ET_CORE                                     4
#define ET_LOOS                                     0xfe00
#define ET_HIOS                                     0xfeff
#define ET_LOPROC                                   0xff00
#define ET_HIPROC                                   0xffff

#define SHT_NULL                                    0
#define SHT_PROGBITS                                1
#define SHT_SYMTAB                                  2
#define SHT_STRTAB                                  3
#define SHT_RELA                                    4
#define SHT_HASH                                    5
#define SHT_DYNAMIC                                 6
#define SHT_NOTE                                    7
#define SHT_NOBITS                                  8
#define SHT_REL                                     9
#define SHT_SHLIB                                   10
#define SHT_DYNSYM                                  11
#define SHT_INIT_ARRAY                              14
#define SHT_FINI_ARRAY                              15
#define SHT_PREINIT_ARRAY                           16
#define SHT_GROUP                                   17
#define SHT_SYMTAB_SHNDX                            18
#define SHT_LOOS                                    0x60000000
#define SHT_HIOS                                    0x6fffffff
#define SHT_LOPROC                                  0x70000000
#define SHT_HIPROC                                  0x7fffffff
#define SHT_LOUSER                                  0x80000000
#define SHT_HIUSER                                  0xfffffff

#define SHF_WRITE                                   0x00000001
#define SHF_ALLOC                                   0x00000002
#define SHF_EXECINSTR                               0x00000004
#define SHF_MERGE                                   0x00000010
#define SHF_STRINGS                                 0x00000020
#define SHF_INFO_LINK                               0x00000040
#define SHF_LINK_ORDER                              0x00000080
#define SHF_OS_NONCONFORMING                        0x00000100
#define SHF_GROUP                                   0x00000200
#define SHF_TLS                                     0x00000400
#define SHF_MASKOS                                  0xff000000
#define SHF_MASKPROC                                0xf0000000
#define SHF_EXCLUDE                                 0x80000000

#define PT_NULL                                     0
#define PT_LOAD                                     1
#define PT_DYNAMIC                                  2
#define PT_INTERP                                   3
#define PT_NOTE                                     4
#define PT_SHLIB                                    5
#define PT_PHDR                                     6
#define PT_TLS                                      7
#define PT_LOOS                                     0x60000000
#define PT_HIOS                                     0x6fffffff
#define PT_LOPROC                                   0x70000000
#define PT_HIPROC                                   0x7fffffff
#define PT_GNU_EH_FRAME                             (PT_LOOS + 0x0474e550)
#define PT_GNU_STACK                                (PT_LOOS + 0x0474e551)
#define PT_GNU_RELRO                                (PT_LOOS + 0x0474e552)

#define STB_LOCAL                                   0
#define STB_GLOBAL                                  1
#define STB_WEAK                                    2
#define STB_LOOS                                    1
#define STB_GNU_UNIQUE                              10
#define STB_HIOS                                    12
#define STB_LOPROC                                  13
#define STB_HIPROC                                  15

#define STT_NOTYPE                                  0
#define STT_OBJECT                                  1
#define STT_FUNC                                    2
#define STT_SECTION                                 3
#define STT_FILE                                    4
#define STT_COMMON                                  5
#define STT_TLS                                     6
#define STT_RELC                                    8
#define STT_SRELC                                   9
#define STT_LOOS                                    10
#define STT_GNU_IFUNC                               10
#define STT_HIOS                                    12
#define STT_LOPROC                                  13
#define STT_HIPROC                                  15

#define STV_DEFAULT                                 0
#define STV_INTERNAL                                1
#define STV_HIDDEN                                  2
#define STV_PROTECTED                               3

#define SHN_UNDEF                                   0x0000
#define SHN_LORESERVE                               0xff00
#define SHN_LOPROC                                  0xff00
#define SHN_HIPROC                                  0xff1f
#define SHN_LOOS                                    0xff20
#define SHN_HIOS                                    0xff3f
#define SHN_ABS                                     0xfff1
#define SHN_COMMON                                  0xfff2
#define SHN_XINDEX                                  0xffff
#define SHN_HIRESERVE                               0xffff

#define STN_UNDEF                                   0

#define DT_NULL                                     0
#define DT_NEEDED                                   1
#define DT_PLTRELSZ                                 2
#define DT_PLTGOT                                   3
#define DT_HASH                                     4
#define DT_STRTAB                                   5
#define DT_SYMTAB                                   6
#define DT_RELA                                     7
#define DT_RELASZ                                   8
#define DT_RELAENT                                  9
#define DT_STRSZ                                    10
#define DT_SYMENT                                   11
#define DT_INIT                                     12
#define DT_FINI                                     13
#define DT_SONAME                                   14
#define DT_RPATH                                    15
#define DT_SYMBOLIC                                 16
#define DT_REL                                      17
#define DT_RELSZ                                    18
#define DT_RELENT                                   19
#define DT_PLTREL                                   20
#define DT_DEBUG                                    21
#define DT_TEXTREL                                  22
#define DT_JMPREL                                   23
#define DT_BIND_NOW                                 24
#define DT_INIT_ARRAY                               25
#define DT_FINI_ARRAY                               26
#define DT_INIT_ARRAYSZ                             27
#define DT_FINI_ARRAYSZ                             28
#define DT_RUNPATH                                  29
#define DT_FLAGS                                    30
#define DT_ENCODING                                 32
#define DT_PREINIT_ARRAY                            32
#define DT_PREINIT_ARRAYSZ                          33
#define DT_LOOS                                     0x6000000d
#define DT_HIOS                                     0x6ffff000
#define DT_LOPROC                                   0x70000000
#define DT_HIPROC                                   0x7fffffff
#define DT_VALRNGLO                                 0x6ffffd00
#define DT_GNU_PRELINKED                            0x6ffffdf5
#define DT_GNU_CONFLICTSZ                           0x6ffffdf6
#define DT_GNU_LIBLISTSZ                            0x6ffffdf7
#define DT_CHECKSUM                                 0x6ffffdf8
#define DT_PLTPADSZ                                 0x6ffffdf9
#define DT_MOVEENT                                  0x6ffffdfa
#define DT_MOVESZ                                   0x6ffffdfb
#define DT_FEATURE                                  0x6ffffdfc
#define DT_POSFLAG_1                                0x6ffffdfd
#define DT_SYMINSZ                                  0x6ffffdfe
#define DT_SYMINENT                                 0x6ffffdff
#define DT_VALRNGHI                                 0x6ffffdff
#define DT_ADDRRNGLO                                0x6ffffe00
#define DT_GNU_HASH                                 0x6ffffef5
#define DT_TLSDESC_PLT                              0x6ffffef6
#define DT_TLSDESC_GOT                              0x6ffffef7
#define DT_GNU_CONFLICT                             0x6ffffef8
#define DT_GNU_LIBLIST                              0x6ffffef9
#define DT_CONFIG                                   0x6ffffefa
#define DT_DEPAUDIT                                 0x6ffffefb
#define DT_AUDIT                                    0x6ffffefc
#define DT_PLTPAD                                   0x6ffffefd
#define DT_MOVETAB                                  0x6ffffefe
#define DT_SYMINFO                                  0x6ffffeff
#define DT_ADDRRNGHI                                0x6ffffeff
#define DT_RELACOUNT                                0x6ffffff9
#define DT_RELCOUNT                                 0x6ffffffa
#define DT_FLAGS_1                                  0x6ffffffb
#define DT_VERDEF                                   0x6ffffffc
#define DT_VERDEFNUM                                0x6ffffffd
#define DT_VERNEED                                  0x6ffffffe
#define DT_VERNEEDNUM                               0x6fffffff
#define DT_VERSYM                                   0x6ffffff0
#define DT_LOPROC                                   0x70000000
#define DT_HIPROC                                   0x7fffffff
#define DT_AUXILIARY                                0x7ffffffd
#define DT_USED                                     0x7ffffffe
#define DT_FILTER                                   0x7fffffff

#define R_ARM_NONE                                  0
#define R_ARM_PC24                                  1
#define R_ARM_ABS32                                 2
#define R_ARM_REL32                                 3
#define R_ARM_LDR_PC_G0                             4
#define R_ARM_ABS16                                 5
#define R_ARM_ABS12                                 6
#define R_ARM_THM_ABS5                              7
#define R_ARM_ABS8                                  8
#define R_ARM_SBREL32                               9
#define R_ARM_THM_CALL                              10
#define R_ARM_THM_PC8                               11
#define R_ARM_BREL_ADJ                              12
#define R_ARM_TLS_DESC                              13
#define R_ARM_THM_SWI8                              14
#define R_ARM_XPC25                                 15
#define R_ARM_THM_XPC22                             16
#define R_ARM_TLS_DTPMOD32                          17
#define R_ARM_TLS_DTPOFF32                          18
#define R_ARM_TLS_TPOFF32                           19
#define R_ARM_COPY                                  20
#define R_ARM_GLOB_DAT                              21
#define R_ARM_JUMP_SLOT                             22
#define R_ARM_RELATIVE                              23
#define R_ARM_GOTOFF32                              24
#define R_ARM_BASE_PREL                             25
#define R_ARM_GOT_BREL                              26
#define R_ARM_PLT32                                 27
#define R_ARM_CALL                                  28
#define R_ARM_JUMP24                                29
#define R_ARM_THM_JUMP24                            30
#define R_ARM_BASE_ABS                              31
#define R_ARM_ALU_PCREL7_0                          32
#define R_ARM_ALU_PCREL15_8                         33
#define R_ARM_ALU_PCREL23_15                        34
#define R_ARM_LDR_SBREL_11_0                        35
#define R_ARM_ALU_SBREL_19_12                       36
#define R_ARM_ALU_SBREL_27_20                       37
#define R_ARM_TARGET1                               38
#define R_ARM_SBREL31                               39
#define R_ARM_V4BX                                  40
#define R_ARM_TARGET2                               41
#define R_ARM_PREL31                                42
#define R_ARM_MOVW_ABS_NC                           43
#define R_ARM_MOVT_ABS                              44
#define R_ARM_MOVW_PREL_NC                          45
#define R_ARM_MOVT_PREL                             46
#define R_ARM_THM_MOVW_ABS_NC                       47
#define R_ARM_THM_MOVT_ABS                          48
#define R_ARM_THM_MOVW_PREL_NC                      49
#define R_ARM_THM_MOVT_PREL                         50
#define R_ARM_THM_JUMP19                            51
#define R_ARM_THM_JUMP6                             52
#define R_ARM_THM_ALU_PREL_11_0                     53
#define R_ARM_THM_PC12                              54
#define R_ARM_ABS32_NOI                             55
#define R_ARM_REL32_NOI                             56
#define R_ARM_ALU_PC_G0_NC                          57
#define R_ARM_ALU_PC_G0                             58
#define R_ARM_ALU_PC_G1_NC                          59
#define R_ARM_ALU_PC_G1                             60
#define R_ARM_ALU_PC_G2                             61
#define R_ARM_LDR_PC_G1                             62
#define R_ARM_LDR_PC_G2                             63
#define R_ARM_LDRS_PC_G0                            64
#define R_ARM_LDRS_PC_G1                            65
#define R_ARM_LDRS_PC_G2                            66
#define R_ARM_LDC_PC_G0                             67
#define R_ARM_LDC_PC_G1                             68
#define R_ARM_LDC_PC_G2                             69
#define R_ARM_ALU_SB_G0_NC                          70
#define R_ARM_ALU_SB_G0                             71
#define R_ARM_ALU_SB_G1_NC                          72
#define R_ARM_ALU_SB_G1                             73
#define R_ARM_ALU_SB_G2                             74
#define R_ARM_LDR_SB_G0                             75
#define R_ARM_LDR_SB_G1                             76
#define R_ARM_LDR_SB_G2                             77
#define R_ARM_LDRS_SB_G0                            78
#define R_ARM_LDRS_SB_G1                            79
#define R_ARM_LDRS_SB_G2                            80
#define R_ARM_LDC_SB_G0                             81
#define R_ARM_LDC_SB_G1                             82
#define R_ARM_LDC_SB_G2                             83
#define R_ARM_MOVW_BREL_NC                          84
#define R_ARM_MOVT_BREL                             85
#define R_ARM_MOVW_BREL                             86
#define R_ARM_THM_MOVW_BREL_NC                      87
#define R_ARM_THM_MOVT_BREL                         88
#define R_ARM_THM_MOVW_BREL                         89
#define R_ARM_TLS_GOTDESC                           90
#define R_ARM_TLS_CALL                              91
#define R_ARM_TLS_DESCSEQ                           92
#define R_ARM_THM_TLS_CALL                          93
#define R_ARM_PLT32_ABS                             94
#define R_ARM_GOT_ABS                               95
#define R_ARM_GOT_PREL                              96
#define R_ARM_GOT_BREL12                            97
#define R_ARM_GOTOFF12                              98
#define R_ARM_GOTRELAX                              99
#define R_ARM_GNU_VTENTRY                           100
#define R_ARM_GNU_VTINHERIT                         101
#define R_ARM_THM_JUMP11                            102
#define R_ARM_THM_JUMP8                             103
#define R_ARM_TLS_GD32                              104
#define R_ARM_TLS_LDM32                             105
#define R_ARM_TLS_LDO32                             106
#define R_ARM_TLS_IE32                              107
#define R_ARM_TLS_LE32                              108
#define R_ARM_TLS_LDO12                             109
#define R_ARM_TLS_LE12                              110
#define R_ARM_TLS_IE12GP                            111
#define R_ARM_ME_TOO                                128
#define R_ARM_THM_TLS_DESCSEQ                       129
#define R_ARM_IRELATIVE                             160
#define R_ARM_GOTFUNCDESC                           161
#define R_ARM_GOTOFFFUNCDESC                        162
#define R_ARM_FUNCDESC                              163
#define R_ARM_FUNCDESC_VALUE                        164
#define R_ARM_TLS_GD32_FDPIC                        165
#define R_ARM_TLS_LDM32_FDPIC                       166
#define R_ARM_TLS_IE32_FDPIC                        167
#define R_ARM_RXPC25                                249
#define R_ARM_RSBREL32                              250
#define R_ARM_THM_RPC22                             251
#define R_ARM_RREL32                                252
#define R_ARM_RABS32                                253
#define R_ARM_RPC24                                 254
#define R_ARM_RBASE                                 255


typedef struct elf32_ehdr {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct elf32_shdr {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct elf32_phdr {
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;

typedef struct elf32_sym {
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half st_shndx;
} Elf32_Sym;

#define ELF32_ST_BIND(i) ((i) >> 4)
#define ELF32_ST_TYPE(i) ((i) & 0xf)
#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t) & 0xf))

#define ELF32_ST_VISIBILITY(o) ((o) & 0x3)
#define ELF32_ST_OTHER(v) ((v) & 0x3)

typedef struct elf32_rel {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
} Elf32_Rel;

typedef struct elf32_rela {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
    Elf32_Sword r_addend;
} Elf32_Rela;

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s) << 8) + (unsigned char)(t))

typedef struct elf32_dyn {
    Elf32_Sword d_tag;
    union {
        Elf32_Word d_val;
        Elf32_Addr d_ptr;
    } d_un;
} Elf32_Dyn;


#define ELF_SEC_HAS_NO_SEGMENT                      (-1)

// Note: Where possible NULL is used to express error.  For the current
// target (STM32F103RB) this is a valid memory address but not ever used.

// Get pointer to phdr based on index, returns NULL on error.  If base is
// NULL ehdr is used as image base.
const struct elf32_phdr *elf32_get_segment_by_index(
        const struct elf32_ehdr *ehdr, const void *base, int index);

// Get pointer to shdr based on index, returns NULL on error.  If base is
// NULL ehdr is used as image base.
const struct elf32_shdr *elf32_get_section_by_index(
        const struct elf32_ehdr *ehdr, const void *base, int index);
// Get pointer to shdr based on raw vma address, returns NULL on error or not
// found.  If base is NULL ehdr is used as image base.
const struct elf32_shdr *elf32_get_section_by_vma(
        const struct elf32_ehdr *ehdr, const void *base, const void *vma);

// Get index based on shdr pointer.  Returns -1 on error.
int elf32_get_shidx_by_section(const struct elf32_ehdr *ehdr,
        const void *base, const struct elf32_shdr *shdr);

// Get pointer/offset to section base relative to image or adjusted for
// actual location in flash, returns NULL on error.
const void *elf32_get_section_offset(const struct elf32_shdr *shdr);
const void *elf32_get_section_flash_addr(const struct elf32_shdr *shdr,
        const void *base);

// Get segment for a given section, phidx and phdr are optional but one must
// be set.
// Note: This structure makes the assumption segment/section mappings are
// 1 to 1 which is not always true but will work for this application.
// 0: Success.
// 1: Section does not have a corresponding segment
//   (*phidx == -1, *phdr = NULL).
// -1: Error (phidx and phdr untouched)
int elf32_get_segment_by_section(const struct elf32_ehdr *ehdr,
        const struct elf32_shdr *shdr, int *phidx,
        const struct elf32_phdr **phdr);

// 0: Success.
// 1: Section does not have a corresponding segment
//   (*phidx == -1, *lma = shdr->sh_addr (adjusted based on function)).
// -1: Error (phidx and phdr untouched)
int elf32_get_section_lma(const struct elf32_ehdr *ehdr,
        const struct elf32_shdr *shdr, const void **lma, int *phidx);
int elf32_get_section_adjusted_lma(const struct elf32_ehdr *ehdr,
        const void *base, const struct elf32_shdr *shdr, const void **lma,
        int *phidx);

// Get section name functions, requires shstrtab section to be present.
const char *elf32_get_section_name_shstrtab_faddr(
        const struct elf32_shdr *shdr, const void *shstrtab_faddr);
const char *elf32_get_section_name_shstrtab(const void *base,
        const struct elf32_shdr *shdr, const struct elf32_shdr *shstrtab);
const char *elf32_get_section_name(const struct elf32_ehdr *ehdr,
        const void *base, const struct elf32_shdr *shdr);
const char *elf32_get_section_name_by_index(const struct elf32_ehdr *ehdr,
        const void *base, int index);

// Get section name functions, requires shstrtab section to be present.
const struct elf32_shdr *elf32_get_section_by_name_shstrtab_faddr(
        const struct elf32_ehdr *ehdr, const void *base, const char *name,
        const void *shstrtab_faddr);
const struct elf32_shdr *elf32_get_section_by_name_shstrtab(
        const struct elf32_ehdr *ehdr, const void *base, const char *name,
        const struct elf32_shdr *shstrtab);
const struct elf32_shdr *elf32_get_section_by_name(
        const struct elf32_ehdr *ehdr, const void *base, const char *name);

// Get adjusted entry pointer, returns NULL on error or not
// found.  If base is NULL ehdr is used as image base.
const void *elf32_get_adjusted_entry(const struct elf32_ehdr *ehdr,
        const void *base);

// Loop over segments.
#define elf32_for_each_phdr_base(pos, idx, ehdr, base) \
    for ((pos) = elf32_get_segment_by_index((ehdr), (base), 0), (idx) = 0; \
        (idx) < (ehdr)->e_phnum; (idx)++, \
        (pos) = (struct elf32_phdr *)((uint8_t *)(pos) + (ehdr)->e_phentsize))

#define elf32_for_each_phdr(pos, idx, ehdr) \
    elf32_for_each_phdr_base((pos), (idx), (ehdr), NULL)

// Loop over sections.  Start section indexing at 1 to skip NULL section at 0.
#define elf32_for_each_shdr_base(pos, idx, ehdr, base) \
    for ((pos) = elf32_get_section_by_index((ehdr), (base), 1), (idx) = 1; \
        (idx) < (ehdr)->e_shnum; (idx)++, \
        (pos) = (struct elf32_shdr *)((uint8_t *)(pos) + (ehdr)->e_shentsize))

#define elf32_for_each_shdr(pos, idx, ehdr) \
    elf32_for_each_shdr_base((pos), (idx), (ehdr), NULL)

#define elf32_adjust_lma(shdr, base) \
    ((shdr)->sh_type != SHT_NOBITS && (shdr)->sh_flags & SHF_ALLOC) ? \
        (base) + (shdr)->sh_offset : 0

#define elf32_adjust_vma(shdr, base) \
        (base) + (shdr)->sh_offset

// elf32_sym.st_shndx can have special values indicated by SHN_* defines
// or is the index of a section header.
#define elf32_sym_has_section_index(sym) \
    ((((sym)->st_shndx) != 0) && \
    ((((sym)->st_shndx) & 0x8000) == 0))


#endif  // _ELF_H
