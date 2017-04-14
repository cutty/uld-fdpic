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

#ifndef _ULD_TYPES_H
#define _ULD_TYPES_H


#include <sys/types.h>

#include "elf.h"


// NOTE: If changing this structure update patch-uld-elf.py and
// gen-uld-files.py.
struct uld_fs_entry;
struct uld_fs_entry {
    void *base;
    struct uld_fs_entry *next;
    size_t size;
    uint32_t crc;
    uint32_t flags;
    char name[];
};

// NOTE: If changing this structure update patch-uld-elf.py and uld_data.S.
struct uld_fs_table {
    struct uld_fs_entry *head;
    const void *table_base;
    uint32_t table_size;
    uint32_t crc;
};

// NOTE: If changing this structure update patch-uld-elf.py and uld_data.S.
struct uld_pstore {
    uint32_t boot_action;
    const void *files_base;
    uint32_t files_size;
    struct uld_fs_table fs_table_pri;
};


// Note: This structure makes the assumption segment/section mappings are
// 1 to 1 which is not always true but will work for this application.
struct uld_section {
    const struct elf32_ehdr *ehdr;
    const struct elf32_phdr *phdr;
    const struct elf32_shdr *shdr;
    const char *name;
    const void *lma;
    const void *adjusted_lma;
    const void *faddr;
    const void *adjusted_vma;
    uint32_t flags;
    int phidx;
    int shidx;
};

// ULD_SECTION_FLAG_TYPE_* are in order of how section normally appear
// in elf files.
#define ULD_SECTION_FLAG_NONE                       0x00000000
#define ULD_SECTION_FLAG_VALID                      0x00000001
#define ULD_SECTION_FLAG_LAST                       0x00000002
#define ULD_SECTION_FLAG_TYPE_MASK                  0x00000ff0
#define ULD_SECTION_FLAG_TYPE_FLASH                 0x00000010
#define ULD_SECTION_FLAG_TYPE_MEM                   0x00000020
#define ULD_SECTION_FLAG_TYPE_DYNAMIC               ULD_SECTION_FLAG_TYPE_FLASH
#define ULD_SECTION_FLAG_TYPE_OTHER                 0x00000040
#define ULD_SECTION_FLAG_TYPE_ALL \
    (ULD_SECTION_FLAG_TYPE_FLASH | \
     ULD_SECTION_FLAG_TYPE_MEM | \
     ULD_SECTION_FLAG_TYPE_DYNAMIC | \
     ULD_SECTION_FLAG_TYPE_OTHER)
#define ULD_SECTION_FLAG_STATUS_MASK                0x000ff000
#define ULD_SECTION_FLAG_STATUS_MEM_NEEDS_FIXUP     0x00001000
#define ULD_SECTION_FLAG_STATUS_MEM_FIXUP_DONE      0x00002000
#define ULD_SECTION_FLAG_STATUS_MEM_LOADED          0x00004000
#define ULD_SECTION_FLAG_STATUS_FLASH_NEEDS_FIXUP   0x00010000
#define ULD_SECTION_FLAG_STATUS_FLASH_FIXUP_DONE    0x00020000

#define ULD_FILE_SECTION_TYPE_COUNT                 3
#define ULD_FILE_SECTION_IDX_FLASH                  0
#define ULD_FILE_SECTION_IDX_MEM                    1
#define ULD_FILE_SECTION_IDX_OTHER                  2

union uld_file_sections {
    struct uld_section *s[ULD_FILE_SECTION_TYPE_COUNT];
    struct {
        struct uld_section *flash;
        struct uld_section *mem;
        struct uld_section *other;
    };
};

union uld_file_section_num {
    int n[ULD_FILE_SECTION_TYPE_COUNT];
    struct {
        int flash;
        int mem;
        int other;
    };
};

struct uld_file {
    const struct uld_fs_entry *fse;
    union uld_file_sections sec;
    const void *adjusted_entry;
    uint8_t *membase;
    size_t memsz;
    uint32_t flags;
    union uld_file_section_num num;
};

#define ULD_FILE_SECTION_MAX                        20

#define ULD_FILE_FLAG_NONE                          0x00000000
#define ULD_FILE_FLAG_EXEC                          0x00000001


#endif  // _ULD_TYPES_H
