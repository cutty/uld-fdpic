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

import sys

import gdb


if hasattr(__builtins__, 'long') is False:
    class long(int):
        pass


_is_qemu = None


def reset_qemu_handler(event):
    global _is_qemu
    _is_qemu = None
    print('_is_qemu reset')
    gdb.events.exited.disconnect(reset_qemu_handler)


def set_qemu(value):
    print('setting _is_qemu to {}'.format(value))
    global _is_qemu
    if _is_qemu is None and hasattr(gdb, 'events'):
        gdb.events.exited.connect(reset_qemu_handler)
    _is_qemu = bool(value)


def check_qemu():
    try:
        ret = gdb.execute('monitor help', to_string=True)
        set_qemu(ret.find('\nqemu-io ') != -1)
    except gdb.error as e:
        if str(e).find('"monitor" command not supported by this target') == 0:
            set_qemu(False)
        else:
            raise e


def is_qemu():
    global _is_qemu
    if _is_qemu is None:
        check_qemu()
    return _is_qemu


def arm_pc_at_bkpt():
    frame = gdb.newest_frame()
    pc = long(frame.read_register('pc'))
    insn = frame.architecture().disassemble(pc)[0]
    return insn['asm'].startswith('bkpt')


def arm_step_past_bkpt():
    if arm_pc_at_bkpt() is True:
        print('at bkpt')
        gdb.execute('p $pc = $pc + 2', to_string=True)


class CmdUld(gdb.Command):
    def __init__(self):
        super(CmdUld, self).__init__('uld',
                gdb.COMMAND_NONE, gdb.COMPLETE_NONE, True)

    def invoke(self, argument, from_tty):
        gdb.execute('help uld')


class CmdUldQEMU(gdb.Command):
    def __init__(self):
        super(CmdUldQEMU, self).__init__('uld qemu',
                gdb.COMMAND_NONE, gdb.COMPLETE_NONE, True)

    def invoke(self, argument, from_tty):
        gdb.execute('help uld qemu')


class CmdUldQEMUReset(gdb.Command):
    def __init__(self):
        super(CmdUldQEMUReset, self).__init__('uld qemu reset',
                gdb.COMMAND_NONE)

    def invoke(self, argument, from_tty):
        if is_qemu() is False:
            print('monitor is not QEMU')
            return
        gdb.execute('monitor system_reset')
        gdb.execute('flushregs')


class CmdUldQEMUCheck(gdb.Command):
    def __init__(self):
        super(CmdUldQEMUCheck, self).__init__('uld qemu check',
                gdb.COMMAND_NONE)

    def invoke(self, argument, from_tty):
        check_qemu()


class CmdUldQEMUContinue(gdb.Command):
    # QEMU's gdbserver does not advance the program counter when stepping or
    # continuing on a bkpt instruction.  It seems this varies among debuggers
    # See: https://bugs.launchpad.net/gcc-arm-embedded/+bug/1383840 for a
    # similar bug.
    def __init__(self, cmd='uld qemu continue'):
        super(CmdUldQEMUContinue, self).__init__(cmd, gdb.COMMAND_NONE)

    def invoke(self, argument, from_tty):
        if is_qemu() is True:
            arm_step_past_bkpt()
        gdb.execute('c')


class CmdUldQEMUNext(gdb.Command):
    # See CmdUldQEMUContinue.
    def __init__(self, cmd='uld qemu next'):
        super(CmdUldQEMUNext, self).__init__(cmd, gdb.COMMAND_NONE)

    def invoke(self, argument, from_tty):
        if is_qemu() is True:
            arm_step_past_bkpt()
        gdb.execute('n')


class CmdUldQEMUStep(gdb.Command):
    # See CmdUldQEMUContinue.
    def __init__(self, cmd='uld qemu step'):
        super(CmdUldQEMUStep, self).__init__(cmd, gdb.COMMAND_NONE)

    def invoke(self, argument, from_tty):
        if is_qemu() is True:
            arm_step_past_bkpt()
        gdb.execute('s')


class CmdUldQEMUStepi(gdb.Command):
    # See CmdUldQEMUContinue.
    def __init__(self, cmd='uld qemu stepi'):
        super(CmdUldQEMUStepi, self).__init__(cmd,
                gdb.COMMAND_NONE)

    def invoke(self, argument, from_tty):
        if is_qemu() is True:
            arm_step_past_bkpt()
        else:
            gdb.execute('si')


class CmdUldExample(gdb.Command):
    def __init__(self):
        super(CmdUldExample, self).__init__('uld example',
                gdb.COMMAND_NONE, gdb.COMPLETE_NONE, True)

    def invoke(self, argument, from_tty):
        gdb.execute('help uld example')


class CmdUldExampleSet(gdb.Command):
    """set uld example (int)"""
    MAX_EXAMPLE_INDEX = 6

    def __init__(self):
        super(CmdUldExampleSet, self).__init__('uld example set',
                gdb.COMMAND_NONE, gdb.COMPLETE_NONE, True)
        self.__doc__ = 'foo bar'

    def invoke(self, argument, from_tty):
        try:
            val = int(argument)
        except ValueError:
            print('argument must be int')
            return
        if val < 0 or val > self.MAX_EXAMPLE_INDEX:
            print('argument must be between 0 and {}'.format(
                    self.MAX_EXAMPLE_INDEX))
            return
        cmd = 'p ((struct uld_pstore *)&_uld_pstore)->boot_action = {}'
        gdb.execute(cmd.format(val),
                to_string=True)
        print('uld example set to \'{}\''.format(val))


def uld_load():
    print('loading {}'.format(sys.version))
    CmdUld()
    CmdUldQEMU()
    CmdUldQEMUReset()
    CmdUldQEMUCheck()
    CmdUldQEMUContinue()
    CmdUldQEMUContinue('uld qemu c')
    CmdUldQEMUContinue('uc')
    CmdUldQEMUNext()
    CmdUldQEMUNext('uld qemu n')
    CmdUldQEMUNext('un')
    CmdUldQEMUStep()
    CmdUldQEMUStep('uld qemu s')
    CmdUldQEMUStep('us')
    CmdUldQEMUStepi()
    CmdUldQEMUStepi('uld qemu si')
    CmdUldQEMUStepi('usi')
    CmdUldExample()
    CmdUldExampleSet()


if __name__ == '__main__':
    uld_load()
