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
#include "uld_load.h"
#include "uld_rofixup.h"
#include "uld_sal.h"
#include "util.h"


static const char * const uld_reloc_flash_fixup_mem_sections[] = {
    ".got",
    ".got.plt",
    ".data"
};


static int uld_reloc_prepare_flash_fixups(struct uld_file *ufile) {
    int i;

    // A binary must have flash sections but not memory sections.
    if (!ufile || !ufile->sec.flash || ufile->num.flash <= 0 ||
            ufile->num.mem < 0) {
        return -1;
    }

    for (i = 0; i < ufile->num.flash; i++) {
        ufile->sec.flash[i].flags |=
                ULD_SECTION_FLAG_STATUS_FLASH_NEEDS_FIXUP;
    }

    for (i = 0; i < ufile->num.mem; i++) {
        //if (!strcmp(ufile->sec.mem[i].name, ".data")) {
        if (str_in_list(uld_reloc_flash_fixup_mem_sections,
                sizeof(uld_reloc_flash_fixup_mem_sections) / sizeof(char *),
                ufile->sec.mem[i].name)) {
            ufile->sec.mem[i].flags |=
                    ULD_SECTION_FLAG_STATUS_FLASH_NEEDS_FIXUP;
        }
    }

    return 0;
}

int uld_reloc_move_fse(const struct uld_fs_entry *fse, const void *new_base)
{
    struct uld_section sec_list[ULD_FILE_SECTION_MAX];
    struct uld_file ufile;
    const void *new_end;
    const void *fse_end;
    int was_locked = 0;
    int ret = -1;

    if (!fse || !new_base) {
        return -1;
    }

    if (new_base != ALIGN_PTR(new_base, 3)) {
        swbkpt();
    }

    new_end = (uint8_t *)new_base + fse->size - 1;
    fse_end = (uint8_t *)fse->base + fse->size - 1;

    // Only simple check done for move to ensure src and dest do not overlap.
    if ((new_base >= fse->base && new_base <= fse_end) ||
            (new_end >= fse->base && new_end <= fse_end)) {
        return -1;
    }

    // A ufile structure of the file before relocation is required for
    // applying flash fixups.
    ret = uld_load_create_file(fse, sec_list,
            sizeof(sec_list) / sizeof(struct uld_section),
            ULD_SECTION_FLAG_TYPE_ALL, &ufile);
    if (ret) {
        goto done;
    }

    was_locked = cpu_flash_is_locked();
    if (was_locked) {
        cpu_flash_unlock();
    }

    ret = cpu_flash_write((void *)new_base, fse->base, fse->size);
    if (ret) {
        goto done;
    }

    ret = uld_reloc_prepare_flash_fixups(&ufile);
    if (ret) {
        goto done;
    }

    ret = uld_rofixup_apply_flash_fixups((const struct elf32_ehdr *)new_base,
            NULL, ufile.sec.flash, ufile.num.flash,
            ufile.sec.mem, ufile.num.mem);
    if (ret) {
        goto done;
    }

    ret = cpu_flash_write((void *)&fse->base, &new_base, sizeof(void *));
    if (ret) {
        goto done;
    }

    uld_crc_update_fse((struct uld_fs_entry *)fse, NULL);
    uld_crc_update_fst(NULL);

done:
    if (was_locked) {
        cpu_flash_unlock();
    }

    return ret;
}
