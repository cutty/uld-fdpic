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
#include "uld_fs.h"


int uld_fs_get_file_count(const struct uld_fs_entry *head)
{
    const struct uld_fs_entry *pos;
    int count = 0;

    fst_for_each_entry(pos, head) {
        count++;
    }
    return count;
}

const struct uld_fs_entry *uld_fs_get_file_by_index(
        const struct uld_fs_entry *head, int index)
{
    const struct uld_fs_entry *pos;
    fst_for_each_entry(pos, head) {
        if (!index--) {
            return pos;
        }
    }
    return NULL;
}

const struct uld_fs_entry *uld_fs_get_file_by_name(
        const struct uld_fs_entry *head, const char *name)
{
    const struct uld_fs_entry *pos;
    fst_for_each_entry(pos, head) {
        if (!strcmp(pos->name, name)) {
            return pos;
        }
    }
    return NULL;
}


const void *uld_fs_find_free_space(struct uld_pstore *pstore, size_t size)
{
    struct uld_fs_entry *pos;
    struct uld_fs_entry **fse_list;
    const uint8_t *ptr;
    int fse_count;
    int fst_idx;
    int i;

    if (!pstore || size == 0) {
        return NULL;
    }

    size = ALIGN(size, 3);

    fse_count = uld_fs_get_file_count(pstore->fs_table_pri.head);

    fse_list = alloca(sizeof(struct uld_fs_entry *) * fse_count);

    // Sort all fs entries in ascending order by base address.
    fst_idx = 0;
    fst_for_each_entry(pos, pstore->fs_table_pri.head) {
        for (i = 0; i < fst_idx; i++) {
            if (pos->base < fse_list[i]->base) {
                break;
            }
        }

        if (i < fst_idx) {
            memmove(&fse_list[i + 1], &fse_list[i],
                    sizeof(struct uld_fs_entry *) * (fst_idx - i));
        }

        fse_list[i] = pos;

        fst_idx++;
    }

    ptr = pstore->files_base;
    for (i = 0; i < fse_count; i++) {
        if ((uint8_t *)fse_list[i]->base - ptr >= (ssize_t)size) {
            return ptr;
        }
        ptr = fse_list[i]->base + fse_list[i]->size;
    }

    if ((uint8_t *)pstore->files_base + pstore->files_size - ptr >=
            (ssize_t)size) {
        return ptr;
    }

    return NULL;
}
