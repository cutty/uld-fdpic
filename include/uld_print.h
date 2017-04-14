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

#ifndef _ULD_PRINT_H
#define _ULD_PRINT_H


extern const char uld_print_section_header_str[];


int uld_fprint_fse_file(FILE *stream, const struct uld_file *ufile);

int uld_fprint_section(FILE *stream, const struct uld_section *section);
int uld_fprint_section_list(FILE *stream, const struct uld_section *sec_list,
        int count, int print_hdr);
int uld_fprint_section_file(FILE *stream, const struct uld_file *ufile);

int uld_fprint_gdb_sym_cmd(FILE *stream, const struct uld_file *ufile);
int uld_fprint_gdb_sym_cmd_list(FILE *stream,
        const struct uld_file *ufile_list, int count);

int uld_fprint_rofixups_ufile(FILE *stream, const struct uld_file *ufile);
int uld_fprint_rofixups_ehdr(FILE *stream, const struct elf32_ehdr *ehdr,
        const void *base);

int uld_fprint_dyn(FILE *stream, const struct uld_file *ufile, int all);
int uld_fprint_dynsym(FILE *stream, const struct uld_file *ufile);
int uld_fprint_rel_dyn(FILE *stream, const struct uld_file *ufile);


#define uld_print_fse_file(ufile) \
    uld_fprint_fse_file(stdout, (ufile))

#define uld_print_section(section) \
    uld_fprint_section(stdout, (section))
#define uld_print_section_list(sec_list, count, print_hdr) \
    uld_fprint_section_list(stdout, sec_list, count, print_hdr)
#define uld_print_section_file(ufile) \
    uld_fprint_section_file(stdout, (ufile))

#define uld_print_file(ufile, full) \
    uld_fprint_file(stdout, (ufile), (full))

#define uld_print_gdb_sym_cmd(ufile) \
    uld_fprint_gdb_sym_cmd(stdout, (ufile))
#define uld_print_gdb_sym_cmd_list(ufile_list, count) \
    uld_fprint_gdb_sym_cmd_list(stdout, (ufile_list), (count))

#define uld_print_rofixups_ufile(ufile) \
    uld_fprint_rofixups_ufile(stdout, (ufile))
#define uld_print_rofixups_ehdr(ehdr, base) \
    uld_fprint_rofixups_ehdr(stdout, (ehdr), (base))

#define uld_print_dyn(ufile, all) \
    uld_fprint_dyn(stdout, (ufile), (all))
#define uld_print_dynsym(ufile) \
    uld_fprint_dynsym(stdout, (ufile))
#define uld_print_rel_dyn(ufile) \
    uld_fprint_rel_dyn(stdout, (ufile))


#endif  // _ULD_PRINT_H
