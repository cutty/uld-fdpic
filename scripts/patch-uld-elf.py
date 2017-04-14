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
import operator
import os
import re
import struct
import sys
import zlib


DEFAULT_FILES_SEC = '.files'
DEFAULT_FS_TABLE_SEC = '.fs_table'
DEFAULT_PSTORE_SEC = '.uld_pdata'
DEFAULT_PSTORE_OFF = 0
PLT_ENTRY_SIZE = 0x14
PLT_ENTRY_GOTOFFFUNCDESC_OFFSET = 0x10
FS_ENTRY_FMT = '<IIIII'
FS_ENTRY_SIZE = struct.calcsize(FS_ENTRY_FMT)
FS_ENTRY_CRC_OFFSET = 0xc
PSTORE_FS_TABLE_CRC_OFFSET = 0x18

ROFIXUP_MEM_SEC_LIST = [
    '.got',
    '.got.plt',
    '.data',
    '.bss'
]

_debug = 0


def dprint(s):
    global _debug
    if _debug > 0:
        print(s)


class CmdError(Exception):
    pass


def qc(cmd):
    s, o = commands.getstatusoutput(cmd)
    if (s != 0):
        print(o)
        raise CmdError('cmd: \'{}\' exited with code: {}'.format(cmd, s))
    return o


def wsclean(s):
    return re.sub(r'\s+', ' ', s.strip())


class ElfSection(object):
    def __init__(self, idx, name, size, vma, lma, file_off, algn, flags):
        self.idx = idx
        self.name = name
        self.size = size
        self.vma = vma
        self.lma = lma
        self.file_off = file_off
        self.algn = algn
        self.flags = flags

        self.file_size = None
        self.type = None

    def __str__(self):
        ret = 'ElfSection(idx={}, name={}, size=0x{:08x}, vma=0x{:08x}, ' \
            'lma=0x{:08x}, file_off=0x{:08x}, algn={}, flags={}, '
        if self.file_size is None:
            ret += 'file_size={}, '
        else:
            ret += 'file_size=0x{:08x}, '
        ret += 'type={})'
        ret = ret.format(self.idx, self.name, self.size, self.vma, self.lma,
                self.file_off, self.algn, self.flags, self.file_size,
                self.type)
        return ret


def get_elf_sections(path):
    ret = []

    objdump = os.environ.get('OBJDUMP', 'objdump')
    readelf = os.environ.get('READELF', 'readelf')

    # Get section data from objdump/readelf and trim headers and footers off.
    oc_out = qc('{} -h {}'.format(objdump, path)).split('\n')[5:]
    re_out = qc('{} -S {}'.format(readelf, path)).split('\n')[5:-4]

    # Data extraction was tested using binutils 2.22 and 2.26.1.
    oc_iter = iter(oc_out)
    for line in oc_iter:
        flags = oc_iter.next().strip()
        if 'ALLOC' not in flags:
            continue
        line = wsclean(line).split(' ')
        idx, name, size, vma, lma, file_off, algn = line
        elf_sec = ElfSection(idx, name, int(size, 16), int(vma, 16),
                int(lma, 16), int(file_off, 16), algn, flags)
        ret.append(elf_sec)

    re_dict = {}
    for line in re_out:
        line = line[line.find('.'):]
        line = wsclean(line).split(' ')
        # Skip entries that do not have the ALLOC flag.
        if 'A' not in line[-4]:
            continue
        name = line[0]
        sec_type = line[1]
        re_dict[name] = sec_type

    for elf_sec in ret:
        elf_sec.type = re_dict.get(elf_sec.name, None)

    sec_list = ret[:]
    sec_list = [x for x in sec_list if x.type == 'NOBITS']
    for elf_sec in sec_list:
        elf_sec.file_size = 0

    sec_list = ret[:]
    sec_list = [x for x in sec_list if x.type != 'NOBITS']

    sec_list.sort(key=operator.attrgetter('file_off'))

    # elf_sec.size (i.e. size during execution) may not always be the same
    # size as section size in the file.
    for index in range(len(sec_list)):
        elf_sec = sec_list[index]

        if index == len(sec_list) - 1:
            # Best guess.
            elf_sec.file_size = elf_sec.size
            break

        next_elf_sec = sec_list[index + 1]
        file_size = next_elf_sec.file_off - elf_sec.file_off

        # Cover case where there may be orphaned data in between sections in
        # the file.
        if file_size > elf_sec.size:
            file_size = elf_sec.size

        elf_sec.file_size = file_size

    return ret


def sec_list_to_dict(sec_list):
    return {x.name: x for x in sec_list}


def sec_name_in_sec_list(sec_list, name):
    return sec_list_to_dict(sec_list).has_key(name)


def name_to_sec(sec_list, name):
    return sec_list_to_dict(sec_list).get(name, None)


def file_off_to_sec(sec_list, file_off):
    for elf_sec in sec_list:
        if file_off >= elf_sec.file_off and \
                file_off < elf_sec.file_off + elf_sec.file_size:
            return elf_sec
    raise ValueError('Could not find section for file_off: 0x{:08x}'.format(
            file_off))


def lma_to_sec(sec_list, lma):
    for elf_sec in sec_list:
        if lma >= elf_sec.lma and lma < elf_sec.lma + elf_sec.file_size:
            return elf_sec
    raise ValueError('Could not find section for lma: 0x{:08x}'.format(
            lma))


def vma_to_sec(sec_list, vma):
    for elf_sec in sec_list:
        if vma >= elf_sec.vma and vma < elf_sec.vma + elf_sec.size:
            return elf_sec
    raise ValueError('Could not find section for vma: 0x{:08x}'.format(
            vma))

def lma_to_file_off(sec_list, lma):
    elf_sec = lma_to_sec(sec_list, lma)
    return lma - elf_sec.lma + elf_sec.file_off


def extract_sec(fd, sec_list, name):
    fd_off = fd.tell()
    sec_dict = sec_list_to_dict(sec_list)

    fd.seek(sec_dict[name].file_off)
    buf = fd.read(sec_dict[name].file_size)

    fd.seek(fd_off)
    return buf


class FSEntry(object):
    def __init__(self, e_file_off, buf, name_len):
        self.e_file_off = e_file_off

        self.file_base, self.next_e, self.size, self.crc, self.flags, \
                self.name = struct.unpack(
                FS_ENTRY_FMT + '{}s'.format(name_len), buf)

    def __str__(self):
        ret = 'FSEntry(file_base=0x{:08x}, next_e=0x{:08x}, size=0x{:08x}, ' \
                'crc=0x{:08x}, flags=0x{:08x}, name={})'
        ret = ret.format(self.file_base, self.next_e, self.size, self.crc,
                self.flags, self.name)
        return ret


def parse_fs_table(buf, buf_file_off, sec_list):
    ret = []
    e_base = 0

    while True:
        # This will put offset at the first char of fs_entry.name
        offset = e_base + FS_ENTRY_SIZE

        # Find the variable length of fs_entry.name.
        while buf[offset] != '\0':
            offset += 1

        #fse = FSEntry(e_base + buf_file_off, buf[e_base:offset + 1])
        fse = FSEntry(e_base + buf_file_off, buf[e_base:offset],
                offset - FS_ENTRY_SIZE - e_base)
        ret.append(fse)

        if fse.next_e is 0:
            break

        e_base = lma_to_file_off(sec_list, fse.next_e)
        e_base -= buf_file_off
        if e_base < 0 or e_base > len(buf):
            err = 'Next FS entry at lma: 0x{:08x} file_off: 0x{:08x} not ' \
                    'in range of buf'
            raise ValueError(err.format(fse.next_e, lma_to_file_off(sec_list,
                    fse.next_e)))

    return ret


def apply_rofixups(uld_sec_list, uld_fd, elf_sec_list, elf_fd, elf_file_lma):
    if not sec_name_in_sec_list(elf_sec_list, '.rofixup'):
        return

    uld_opos = uld_fd.tell()
    elf_opos = elf_fd.tell()

    elf_file_off = lma_to_file_off(uld_sec_list, elf_file_lma)

    fixups = extract_sec(elf_fd, elf_sec_list, '.rofixup')
    fixups = [struct.unpack('<I', fixups[x:x + 4])[0] for x in
            range(0, len(fixups), 4)]

    global _debug
    for addr in fixups:
        sec = lma_to_sec(elf_sec_list, addr)
        file_off = lma_to_file_off(elf_sec_list, addr)

        elf_fd.seek(file_off)
        value = elf_fd.read(4)
        value = struct.unpack('<I', value)[0]

        # If a fixup value does not have a valid lma it is most likely pointing
        # to a NOBITS sections (i.e. .bss) and should already be set as skip.
        try:
            value_sec = lma_to_sec(elf_sec_list, value)
        except ValueError:
            if skip is False:
                raise
            value_sec = None

        # Sections that will be loaded into memory will have fixups applied
        # at runtime.
        if value_sec is None or value_sec.name in ROFIXUP_MEM_SEC_LIST:
            skip = True
        else:
            skip = False

        if _debug > 0:
            if value_sec is None:
                try:
                    value_sec = vma_to_sec(elf_sec_list, value)
                    value_sec_name = value_sec.name + ' (vma)'
                except:
                    value_sec_name = 'UNKNOWN'
            else:
                value_sec_name = value_sec.name

            if skip is True:
                value_sec_name += ' (skip)'
            fmt = '  addr: 0x{:08x}   sec: {:<12s} file_off: 0x{:08x}  ' \
                    'value: 0x{:08x}  value_sec: {}'
            dprint(fmt.format(addr, sec.name, file_off, value, value_sec_name))

        if skip is True:
            continue

        # Sanity check that everything is in the right place.
        uld_fd.seek(elf_file_off + file_off)
        uld_value = uld_fd.read(4)
        uld_value = struct.unpack('<I', uld_value)[0]
        if value != uld_value:
            fmt = 'Incorrect value 0x{:08x} at 0x{:08x} in uld_fd ' \
                    'expected 0x{:08x}'
            raise ValueError(fmt.format(uld_value, elf_file_off + file_off,
                    value))

        fixup_value = lma_to_file_off(elf_sec_list, value) + elf_file_lma

        msg = '    Applying fixup for 0x{:08x} value 0x{:08x}->0x{:08x}'
        dprint(msg.format(addr, value, fixup_value))

        fixup_value = struct.pack('<I', fixup_value)
        uld_fd.seek(-4, 1)
        uld_fd.write(fixup_value)

        uld_fd.seek(uld_opos)
        elf_fd.seek(elf_opos)


def patch_plt_gotofffuncdesc(uld_sec_list, uld_fd, elf_sec_list, elf_fd,
        elf_file_lma):
    plt_sec = name_to_sec(elf_sec_list, '.plt')
    count = 0
    picreg_offset = 0

    uld_opos = uld_fd.tell()

    if plt_sec is not None:
        # A .plt section should not exist without a .got.plt
        got_plt_sec = name_to_sec(elf_sec_list, '.got.plt')
        if got_plt_sec is None:
            raise ValueError('.got.plt section not found when .plt section '
                    'is present')
        # ld is generating the GOTOFFFUNCDESC values as an offset from the
        # base of .got.plt.  During runtime the values in .got.plt are
        # referenced using the pic base register which will point to the
        # base of .got if present (if not it will be the base of .got.plt).
        # Update the GOTOFFFUNCDESC values accordingly.
        got_sec = name_to_sec(elf_sec_list, '.got')
        if got_sec is not None:
            picreg_offset = got_plt_sec.lma - got_sec.lma

    if picreg_offset != 0:
        if plt_sec.size % 20 != 0:
            raise ValueError('.plt size {} is not multiple of 20'.format(
                    plt_sec.size))
        elf_file_off = lma_to_file_off(uld_sec_list, elf_file_lma)
        plt_file_off = lma_to_file_off(elf_sec_list, plt_sec.lma)
        for val_off in range(PLT_ENTRY_GOTOFFFUNCDESC_OFFSET, plt_sec.size,
                PLT_ENTRY_SIZE):
            uld_fd.seek(elf_file_off + plt_file_off + val_off)
            val = uld_fd.read(4)
            val = struct.unpack('<I', val)[0]
            #dprint('.plt + {:08x} GOTOFFFUNCDESC {:08x}->{:08x}'.format(
            #        val_off, val, val + picreg_offset))
            val = struct.pack('<I', val + picreg_offset)
            uld_fd.seek(-4, 1)
            uld_fd.write(val)
            count += 1

    dprint('Updated GOTOFFFUNCDESC values for {} plt entries'.format(count))

    uld_fd.seek(uld_opos)


def find_elf_file(elf_search_path, elf_filename):
    for dir_path in elf_search_path:
        path = os.path.join(dir_path, elf_filename)
        if os.path.exists(path):
            return path
    raise ValueError('{} not found.'.format(elf_filename))


def write_elf_file_crc(uld_sec_list, uld_fd, fse):
    uld_opos = uld_fd.tell()

    elf_file_off = lma_to_file_off(uld_sec_list, fse.file_base)
    uld_fd.seek(elf_file_off)
    data = uld_fd.read(fse.size)
    crc = zlib.crc32(data, 0)
    crc &= 0xffffffff

    dprint('Patching new crc 0x{:08x} for file {}'.format(crc, fse.name))

    crc = struct.pack('<I', crc)
    uld_fd.seek(fse.e_file_off + FS_ENTRY_CRC_OFFSET)
    uld_fd.write(crc)

    uld_fd.seek(uld_opos)


def write_fs_table_crc(uld_fd, fs_table_off, fs_table_size, pstore_off):
    uld_opos = uld_fd.tell()

    uld_fd.seek(fs_table_off)
    data = uld_fd.read(fs_table_size)
    crc = zlib.crc32(data, 0)
    crc &= 0xffffffff

    dprint('Patching new crc 0x{:08x} for fs_table'.format(crc))
    crc = struct.pack('<I', crc)
    uld_fd.seek(pstore_off + PSTORE_FS_TABLE_CRC_OFFSET)
    uld_fd.write(crc)

    uld_fd.seek(uld_opos)


def patch_uld_elf(args):
    global _debug
    dprint('args: {}'.format(args))

    uld_sec_list = get_elf_sections(args.uld_path)
    uld_fd = open(args.uld_path, 'r+')

    uld_sec_dict = sec_list_to_dict(uld_sec_list)
    fs_table_sec = uld_sec_dict[args.fs_table_section]
    uld_fd.seek(fs_table_sec.file_off)

    # Detect dev case where no files are embedded.
    if uld_sec_dict.has_key(args.file_section):
        fs_table = uld_fd.read(fs_table_sec.file_size)
        fs_table = parse_fs_table(fs_table, fs_table_sec.file_off,
                uld_sec_list)
    else:
        fs_table = []

    if _debug > 0:
        dprint('Read {} FSEntries from {}'.format(len(fs_table),
                args.uld_path))
        for x in fs_table:
            dprint('  {}'.format(x))

    uld_dir = os.path.split(args.uld_path)[0]
    if args.elf_search_path is not None:
        elf_search_path = args.elf_search_path + [uld_dir,]
    else:
        elf_search_path = [uld_dir,]

    pstore_off = uld_sec_dict[args.pstore_section].file_off
    pstore_off += args.pstore_offset

    for fse in fs_table:
        elf_path = find_elf_file(elf_search_path, fse.name)
        dprint('Processing file {}'.format(elf_path))
        elf_fd = open(elf_path, 'r')
        elf_sec_list = get_elf_sections(elf_path)

        apply_rofixups(uld_sec_list, uld_fd, elf_sec_list, elf_fd,
                fse.file_base)

        patch_plt_gotofffuncdesc(uld_sec_list, uld_fd, elf_sec_list, elf_fd,
                fse.file_base)

        write_elf_file_crc(uld_sec_list, uld_fd, fse)

        elf_fd.close()

    write_fs_table_crc(uld_fd, fs_table_sec.file_off, fs_table_sec.size,
            pstore_off)

    uld_fd.close()


def main(argv=None):
    if argv is not None:
        prog = os.path.basename(argv[0])
    else:
        prog = 'patch-uld-elf.py'

    epilog='\nIf OBJCOPY/READELF is not present in environment ' \
    '\'objcopy\' and \'readelf\' will be used.\n' \
    'Usage for --elf-search-path:\n' \
    '  {} /path/to/uld-elf --elf-search-path /path/to/search-1 ' \
    '/path/to/search-2'
    epilog = epilog.format(prog)

    parser = argparse.ArgumentParser(prog=prog,
            formatter_class=argparse.RawDescriptionHelpFormatter,
            description='Apply rofixups for files contained within uld '
            'firmware and recalculate crc32 checksums for files and fs_table',
            epilog=epilog)

    parser.add_argument('--file-section', type=str, default=DEFAULT_FILES_SEC,
            help='Section prefix to for files (default: {})'.format(
            DEFAULT_FILES_SEC))

    parser.add_argument('--fs-table-section', type=str,
            default=DEFAULT_FS_TABLE_SEC,
            help='Section prefix to for fs_table (default: {})'.format(
            DEFAULT_FS_TABLE_SEC))

    parser.add_argument('--pstore-section', type=str,
            default=DEFAULT_PSTORE_SEC,
            help='Section prefix to for pstore (default: {})'.format(
            DEFAULT_PSTORE_SEC))

    parser.add_argument('--pstore-offset', type=int,
            default=DEFAULT_PSTORE_OFF,
            help='Offset from section base for pstore (default: {})'.format(
            DEFAULT_PSTORE_OFF))

    parser.add_argument('--elf-search-path', type=str, nargs='+',
            help='Search path for elf files, each path is processed in order '
            'from the command line followed by the base directory of '
            'uld-path')

    parser.add_argument('--verbose', action='store_true')

    parser.add_argument('uld_path', type=str, metavar='uld-path',
            help='Path to input file path or name=path to change default '
            'name (derived from path and file-path-strip)')

    args = parser.parse_args()

    global _debug
    if args.verbose is True:
        _debug = 1

    patch_uld_elf(args)


if __name__ == '__main__':
    main()
