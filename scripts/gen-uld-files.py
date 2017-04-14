#!/usr/bin/env python

# Copyright (c) 2016, 2017 Joe Vernaci
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import argparse
import commands
import os
import sys
import tempfile
import zlib


DEFAULT_ALIGNMENT = 8
DEFAULT_SEC_PREFIX = '.files'
DEFAULT_SEC_SYM_BASE = '_s_files'
DEFAULT_FS_FILE_COUNT_DEF = '_ULD_FS_FILE_COUNT'
DEFAULT_FS_TABLE_SIZE_DEF = '_ULD_FS_TABLE_SIZE'
DEFAULT_FS_TBL_SIZE = 0x200
DEFAULT_FS_TBL_SYM_DEF = '_ULD_FS_TABLE'

DEFAULT_SEC_FLAGS = 'alloc,contents,load,readonly,code'

_debug = 0


def dprint(s):
    global _debug
    if _debug > 0:
        print(s)


class GenError(Exception):
    pass


class CmdError(Exception):
    pass


def qc(cmd):
    s, o = commands.getstatusoutput(cmd)
    if (s != 0):
        raise CmdError('cmd: \'{}\' exited with code: {}'.format(cmd, s))
    return o


def closerm(fd, path):
    try:
        os.fstat(fd)
        os.close(fd)
    except (OSError, IOError):
        pass
    if os.path.exists(path):
        os.unlink(path)
    dprint('fd:{} ({}) is closed and unlinked'.format(fd, path))


def new_tmp(tmpfiles):
    fd, path = tempfile.mkstemp()
    tmpfiles.append((fd, path))
    dprint('opened tmpfile: {} fd: {}'.format(path, fd))
    return fd, path


def crcfd(fd):
    n = os.fstat(fd).st_size
    crc32 = 0
    while True:
        data = os.read(fd, n)
        if len(data) == 0:
            break
        crc32 = zlib.crc32(data, crc32)
    return crc32


def crcpath(path):
    print(path)
    fd = os.open(path, os.O_RDONLY)
    crc32 = crcfd(fd)
    os.close(fd)
    return crc32


def crcwrite(fd, data, crc32):
    if len(data) == 0:
        return crc32

    crc32 = zlib.crc32(data, crc32)
    while len(data) > 0:
        wn = os.write(fd, data)
        if wn == 0:
            break
        data = data[wn:]

    return crc32


def fdcopy(dest, src, n=0):
    crc32 = 0
    if n == 0:
        n = os.fstat(src).st_size

    while n > 0:
        data = os.read(src, n)
        if len(data) == 0:
            break
        n -= len(data)

        crc32 = crcwrite(dest, data, crc32)

    return crc32


def pad_len(size, align):
    m = (1 << align) - 1
    pl = ((size + m) & ~m) - size
    return pl


# secname: section name.
# inpath: path of file to embed.
# objpath: object file embedding into.
# newpath: (optinoal) objpath is unchanged and result in newpath.
def o_add_file(secname, inpath, objpath, newpath=None):
    objcopy = os.environ.get('OBJCOPY', 'objcopy')
    cmd = '{} --add-section {}={} --set-section-flags {}={} {}'.format(
        objcopy, secname, inpath, secname, DEFAULT_SEC_FLAGS,
        objpath)
    if newpath is not None:
        cmd += ' {}'.format(newpath)
    qc(cmd)


def gen_hdr(args, hdr_info):
    ret = []

    base = 0
    next_e = 0
    flags = 0
    last = len(hdr_info) - 1

    for index, info in enumerate(hdr_info):
        name, size, crc = info

        # Before Python 3.0 zlib.crc32 may return a negative value, this
        # will prevent format from prepending a negative sign without changing
        # the 32 bit value.
        crc &= 0xffffffff

        # For cortex-m align structs to 4 bytes.  All members of struct
        # uld_fs_entry are 4 bytes in size except for the variable sized
        # name.  Pad name to 4 bytes including null char.  Could also use
        # .align directive but this lets us calculate the next pointer.
        name = name + ('\0' * (pad_len(len(name) + 1, 2) + 1))
        next_e += (5 * 4) + len(name)

        ret.append('    .word {} + 0x{:08x} @ .base'.format(
                args.sec_sym_base, base))
        base += size

        if index == last:
            ret.append('    .word 0x00000000 @ .next')
        else:
            ret.append('    .word {} + 0x{:08x} @ .next'.format(
                args.fs_table_sym_def, next_e))

        ret.append('    .word 0x{:08x} @ .size'.format(size))
        ret.append('    .word 0x{:08x} @ .crc'.format(crc))
        ret.append('    .word 0x{:08x} @ .flags'.format(flags))
        ret.append('    .ascii "{}"'.format(name.replace('\0', '\\000')))

    space = args.fs_table_size - next_e
    if space < 0:
        raise GenError('Generated table size {} greater than {}'.format(
                next_e, args.fs_table_size))
    ret.append('    .space {}'.format(space))

    #ret = ['\t{} \\'.format(x) for x in ret[:-1]] \
    #    + ['\t{}'.format(ret[-1]),]

    ret = ['\t{}'.format(x) for x in ret]

    # Change path to ifdef guards.
    guard = '_' + args.hdr.replace('/', '_')
    guard = guard.replace('.', '_')
    guard = guard.upper()

    #guard = '_NEED_{}'.format(args.fs_table_def)
    header = []
    header.append('/* Autogenerated file, DO NOT EDIT manually!')
    header.append('   This file was generated by gen-uld-files')
    header.append('*/\n\n')

    header.append('#ifndef {}'.format(guard))
    header.append('#define {}\n\n'.format(guard))
    header.append('#define {} {}'.format(args.file_count_def, len(hdr_info)))
    header.append('#define {} {}'.format(args.fs_table_size_def,
            args.fs_table_size))

    #header.append('#define {}(__FS_TBL_NAME) \\'.format(args.fs_table_def))
    #header.append('\t#define __INLINE_FS_TABLE__ \\')
    #header.append('\t#define __FS_TBL_NAME__ __FS_TBL_NAME \\')
    #header.append('\t#include {} \\'.format(args.hdr))
    #header.append('\t#undef __FS_TBL_NAME__ \\')
    #header.append('\t#undef __INLINE_FS_TABLE__')

    header.append('\n\n#endif  // {}\n\n'.format(guard))

    header.append('#ifdef __INLINE_FS_TABLE__')
    header.append('#undef __INLINE_FS_TABLE__\n')

    header.append('#ifndef {}'.format(args.fs_table_sym_def))
    hdr_str = '#error Please define {} with the current fs table symbol'
    header.append(hdr_str.format(args.fs_table_sym_def))
    header.append('#endif  // {}\n\n'.format(args.fs_table_sym_def))
    #header.append('#ifndef {}'.format(args.fs_count_def))
    #header.append('#endif  // {}\n\n'.format(args.fs_count_def))
    #header.append('#ifdef {}\n'.format(args.fs_need_table_def))

    #header.append('#define {} {}\n'.format(args.fs_count_def, len(hdr_info)))
    #header.append('#ifndef _{}'.format(guard))
    #header.append('#define _{}\n\n'.format(guard))
    #header.append('#define {}(__TBL_BASE__) \\'.format(args.fs_table_def))

    footer = []
    footer.append('\n#undef {}'.format(args.fs_table_sym_def))
    footer.append('\n\n#endif  // __INLINE_FS_TABLE__\n')
    #footer.append('\n#endif  // {}\n'.format(args.fs_need_table_def))

    ret = '\n'.join(header + ret + footer)
    return ret


def gen_uld_files(args, tmpfiles):
    dprint('args: {}'.format(args))

    path_strip = args.file_path_strip
    if path_strip is not None:
        path_strip = path_strip.split()
    else:
        path_strip = list()

    out_obj_tfd, out_obj_tpath = new_tmp(tmpfiles)
    # mkstemp will return fd(s) so use them here too for consistency.
    # Open everything at once to fail early.
    in_obj_fd = os.open(args.obj, os.O_RDONLY)
    fdcopy(out_obj_tfd, in_obj_fd)
    os.close(out_obj_tfd)
    os.close(in_obj_fd)

    # List to preserve order.
    hdr_info = []

    for path in args.file:
        if path.find('=') != -1:
            secname, path = path.split('=', 1)
            fd = os.open(path, os.O_RDONLY)
        else:
            secname = path
            # open input file before prefix strip.
            fd = os.open(path, os.O_RDONLY)
            # match first strip prefix off of input files.
            for ps in path_strip:
                if path.startswith(ps):
                    secname = path[len(ps):]
                    break

        size = os.fstat(fd).st_size

        # objcopy does not support changing alignment on ELF sections.  Pad
        # the input files here and they will be aligned together on final
        # link.
        pad = pad_len(size, args.file_align)
        if pad != 0:
            tfd, tpath = new_tmp(tmpfiles)
            crc32 = fdcopy(tfd, fd)
            crc32 = crcwrite(tfd, '\0' * pad, crc32)
            os.close(tfd)
            opath = tpath  # objcopy from padded file.
        else:
            # Input file fd is still at pos 0 and we will not use it later.
            crc32 = crcfd(fd)
            opath = path

        hdr_info.append((secname, size + pad, crc32))
        os.close(fd)

        secname = '{}.{}'.format(args.file_section, secname)
        o_add_file(secname, opath, out_obj_tpath)

    hdr = gen_hdr(args, hdr_info)

    # Remove args.obj file to ensure make will always rebuild this target.
    os.unlink(args.obj)

    hdr_fd = os.open(args.hdr, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0664)
    os.write(hdr_fd, hdr)

    os.rename(out_obj_tpath, args.obj)


def main(argv=None):
    if argv is not None:
        prog = os.path.basename(argv[0])
    else:
        prog = 'gen-uld-files.py'

    parser = argparse.ArgumentParser(prog=prog,
            formatter_class=argparse.RawDescriptionHelpFormatter,
            description='Generate uld files header/object',
            epilog='\nIf OBJCOPY is not present in environment \'objcopy\' '
            'will be used.')

    parser.add_argument('--file-align', type=int, default=DEFAULT_ALIGNMENT,
            help='File alignment in low-order zero bytes '
            '(default: {})'.format(DEFAULT_ALIGNMENT))

    parser.add_argument('--file-path-strip', type=str,
            help='Comma separated path(s) to strip off files when creating '
            'fs table')

    parser.add_argument('--file-section', type=str, default=DEFAULT_SEC_PREFIX,
            help='Section prefix to for files (default: {})'.format(
            DEFAULT_SEC_PREFIX))

    parser.add_argument('--sec-sym-base', type=str,
            default=DEFAULT_SEC_SYM_BASE,
            help='Symbol base added to file pointers in fs table '
            '(default: {})'.format(DEFAULT_SEC_SYM_BASE))

    parser.add_argument('--file-count-def', type=str,
            default=DEFAULT_FS_FILE_COUNT_DEF,
            help='fs table count define name (default: {})'.format(
            DEFAULT_FS_FILE_COUNT_DEF))

    parser.add_argument('--fs-table-size', type=int,
            default=DEFAULT_FS_TBL_SIZE,
            help='fs table size, err if over, pad if under '
            '(default: {})'.format(DEFAULT_FS_TBL_SIZE))

    parser.add_argument('--fs-table-sym-def', type=str,
            default=DEFAULT_FS_TBL_SYM_DEF,
            help='fs table base define name (default: {})'.format(
            DEFAULT_FS_TBL_SYM_DEF))

    parser.add_argument('--fs-table-size-def', type=str,
            default=DEFAULT_FS_TABLE_SIZE_DEF,
            help='fs table size define name (default: {})'.format(
            DEFAULT_FS_TABLE_SIZE_DEF))

    parser.add_argument('--verbose', action='store_true')

    parser.add_argument('hdr', type=str,
            help='Path of fs header file to create')

    parser.add_argument('obj', type=str,
            help='Path to existing object to add files to')

    parser.add_argument('file', type=str, nargs='*',
            help='Path to input file path or name=path to change default '
            'name (derived from path and file-path-strip)')

    args = parser.parse_args()

    global _debug
    if args.verbose is True:
        _debug = 1

    tmpfiles = []
    try:
        gen_uld_files(args, tmpfiles)
    finally:
        for fd, path in tmpfiles:
            closerm(fd, path)


if __name__ == '__main__':
    main()
