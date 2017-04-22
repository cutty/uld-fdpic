# uld-fdpic
A bare-metal dynamic linker/loader/relocator for the ARM Cortex-M3 and M4
architecture using the FDPIC (function descriptor position independent code)
ABI.  The FDPIC ABI requires using a fork of GCC and binutils developed by
[STMicroelectronics](http://www.st.com) which is further modified to add an
embedded target (see [References](#references) forked repositories).  The
loading and linking routines are designed to be as memory efficient as possible
during application runtime.  Full source [debugging](#debugging) is achievable
using the apt package `gdb-multiarch`.

This repository contains several example shared libraries and executables that
are packaged with the dynamic linker into a single ELF file.  Each of these
*loadable modules* are referenced using a singly linked list file system and on
demand can be relocated anywhere in flash.  The dynamic linker entry point can
select between different relocation and linking/execution scenarios based on a
variable stored in flash or eeprom if available.

Currently the only tested target is a STM32F103RB (Cortex-M3) emulated in a
modified QEMU, **no hardware required**.

This software should be considered **proof of concept only**.  While the FDPIC
ABI can support a multi-program environment without an MMU this project only
focuses on a single-program environment.

## Motivation
The goal of this project is to evaluate using dynamic linking as a means to
reduce the size of updates for end devices when the updates themselves are very
expensive.  These costs can include:
* **Downlink throughput:**  Using emerging wireless technologies with
relatively low throughput especially at long range.
* **Spectrum management:**  The same wireless technologies operate in already
crowded unlicensed bands, lowering spectrum utilization can increase device
density for a base station.
* **Battery consumption:**  Many devices advertise an expected battery life
under the assumption of transmitting tens or hundreds of bytes per day.
Receiving a single update could consume weeks or months of battery life.
* **Update time:**  While directly related to the metrics above this is closely
tied to user experience, especially when normal operation is interrupted during
the update.

Reducing the cost of updates increases the likelihood of both developers
pushing updates and users applying them.

Using the premise that a typical embedded program will be made of the following
module classes:

| Update frequency | Size      | Example                    |
| :---             | :---      | :---                       |
| Very Rarely      | Large     | HAL/Newlib/libc            |
| Rarely           | Large     | Off micro drivers          |
| Occasionally     | Med/Large | In-house product libraries |
| Frequently       | Med       | Product application(s)     |

The developer can split the program to reduce the average update size,
including placing large blocks of static data into a separate library.  Care
must be taken to balance ELF, dynamic linking and FDPIC descriptor overhead
with the number of modules.  While there are many ways to reduce update size,
dynamic linking is extremely flexible and can leverage existing code.

## Installation
This was tested on a clean and updated Ubuntu 16.04.2 install.

* Install prerequisites (requires ~1GB).  Installing the `gdb-multiarch`
package does not overwrite the standard GDB.
```
sudo apt-get install acpica-tools autoconf automake bison build-essential \
    chrpath curl debhelper dejagnu device-tree-compiler flex gdb-multiarch \
    git gnutls-dev libaio-dev libasound2-dev libattr1-dev libbluetooth-dev \
    libbrlapi-dev libcacard-dev libcap-dev libcap-ng-dev libcurl4-gnutls-dev \
    libfdt-dev libgmp-dev libiscsi-dev libjpeg-dev libmpc-dev libmpfr-dev \
    libncurses5-dev libnuma-dev libpixman-1-dev libpng-dev libpulse-dev \
    librados-dev librbd-dev libsasl2-dev libsdl-image1.2-dev libsdl1.2-dev \
    libseccomp-dev libspice-protocol-dev libspice-server-dev libusb-1.0-0-dev \
    libusbredirparser-dev libx11-dev libxen-dev m4 quilt texinfo uuid-dev \
    xfslibs-dev zlib1g-dev
```

* Download and build toolchain and QEMU (requires ~5.1GB).  The toolchain and
QEMU are only used locally and will **not** modify the system.
    * After `repo init` you can ignore the error: `fatal: unable to auto-detect
    email address`
    * After `repo sync` you can ignore the warning: `curl: (22) The requested
    URL returned error: 404 Not Found`
    * build-toolchain.sh by default will set the number of jobs to the number
    of cores.  Override this with the `-j` switch.

```
mkdir ~/uld
cd ~/uld
curl https://storage.googleapis.com/git-repo-downloads/repo > ./repo
chmod a+x ./repo
./repo init -u https://github.com/Cutty/uld-manifest.git
./repo sync
cd scripts
./build-toolchain.sh
```

* Build the firmware:
```
cd ~/uld/uld-fdpic
make
```

## Usage
* Start QEMU (use ctrl-c to exit the loop):
```
cd ~/uld/scripts
./run-qemu.sh --loop
```

* Connect GDB:
    * Running GDB from the uld-fdpic directory is required to automatically
    source the helper scripts in uld-fdpic/scripts/uld-gdb.py.
    * When starting GDB if you get the error: `warning: File "..." auto-loading
    has been declined by your 'auto-load safe-path'` enter the following in
    your terminal: `echo "set auto-load safe-path /" >> ~/.gdbinit`
    * Setting the PATH variable is required for the `add-symbol-file` commands
    to work (see [debugging](#debugging)).
```
cd ~/uld/uld-fdpic
PATH=./bin:$PATH gdb-multiarch uld_gdb.elf
(gdb) set confirm off
(gdb) tar rem :1234
```

* Select example and run:

| Example ID  | Move        | Exec               | Num Libs  | Num Argv   | Verbose | Reset Stack |
| :---        | :---        | :---               | :---      | :---       | :---    | :---        |
| 0 (default) |             | hello_world_static | 0         | 0          | 0       | No          |
| 1           |             | hello_world        | 1         | 2          | 1       | No          |
| 2           | hello_world | hello_world        | 1         | 3          | 0       | No          |
| 3           |             | dyn_test           | 4         | 2          | 0       | No          |
| 4           |             | dyn_test           | 4         | 0          | 2       | No          |
| 5           | libexc.so   | dyn_test           | 4         | 2          | 1       | No          |
| 6           |             | dyn_test           | 4         | 2          | 0       | Yes         |

```
(gdb) uld example set <id>
(gdb) uc
```

stdout/stderr in uld and loaded modules is redirected to the QEMU monitor.

### Using GDB
#### uld-gdb.py commands

| Command                | Desc                                                                                     |
| :---                   | :---                                                                                     |
| uld qemu reset         | QEMU system reset and flush registers (does not reload new images)                       |
| uld example set \<id\> | set ULD_PSTORE->boot_action to id                                                        |
| uc                     | if current insn is bkpt step over* and continue                                          |
| un                     | if current insn is bkpt step over* and step program, proceeding through subroutine calls |
| us                     | if current insn is bkpt step over* and step to new source line                           |
| usi                    | if current insn is bkpt step over* and step single instruction                           |

#### GDB commands useful for uld

| Command                | Desc                                                                                  |
| :---                   | :---                                                                                  |
| tar rem :1234          | connect to local QEMU GDB server                                                      |
| k                      | send kill to QEMU; QEMU will exit and restart if --loop is used, reloads new images   |
| add-symbol-file <args> | add symbols for loaded modules with addresses, uld-fdpic will generate these commands |
| file                   | with no args will remove all symbol files                                             |
| file uld_gdb.elf       | after `file` command reload uld symbols (must use uld_gdb.elf**)                      |

\* QEMU does not step through bkpt instructions and the program counter must be
manually incremented.  See
[LP #1383840](https://bugs.launchpad.net/gcc-arm-embedded/+bug/1383840) for a
similar bug/behavior.

\** uld_gdb.elf has the .files section removed.  The .files section will
overlap with sections defined by add-symbol-file which confuses GDB.  Stepping
into functions will no longer work and causes reproducible crashes in GDB.

### Debugging
By default uld will break before loaded module constructors are called and
again before main.  The first break will be preceded by at least one
`add-symbol-file` command.  Copy these commands into GDB to add symbols with
the correct section addresses for the loaded modules.  Use `set confirm off` to
paste multiple commands at a time.

#### Debugging example
* With `run-qemu.sh --loop` running in another terminal:
```
cd ~/uld/uld-fdpic
PATH=./bin:$PATH gdb-multiarch uld_gdb.elf
(gdb) set confirm off
(gdb) tar rem :1234
<gdb> vector_reset () at src/uld_start.S:40
(gdb) uld example set 4
(gdb) uc
<qemu> break before ctor - gdb: uc to continue
<copy all add-symbol-file commands from qemu to gdb>
<gdb> Reading symbols from dyn_test.elf...done.
(gdb) b main
<gdb> Breakpoint 1 at 0x800d5a0: file src/example/ex_app_dyn_test.c, line 187.
(gdb) uc
<qemu> <output from constructors>
<qemu> break before entry - gdb: uc to continue
(gdb) uc
<gdb> Breakpoint 1, main (argc=0, argv=0x200042fc) at src/example/ex_app_dyn_test.c:187
<gdb> 187         uint32_t pc = cpu_get_pc();
(gdb) bt
<gdb> #0  main (argc=0, argv=0x200042fc) at src/example/ex_app_dyn_test.c:187
<gdb> #1  0x08002e76 in uld_exec_elf_call_entry () at src/uld_exec_asm.S:139
<gdb> #2  0x08002dbe in uld_exec_file (ufile=0x20004f20, sp_base=0x0, argc=0, argv=0x0) at src/uld_exec.c:132
<gdb> #3  0x08002ba0 in uld_dyn_exec_fse (fse=0x800655c <_fs_table_pri+216>, sp_base=0x0, argc=0, argv=0x0) at src/uld_dyn.c:1132
<gdb> #4  0x08001752 in uld_main () at src/uld.c:130
<gdb> #5  0x08005340 in vector_reset () at src/uld_start.S:80
<gdb> Backtrace stopped: previous frame identical to this frame (corrupt stack?)
```

Using example 6 will reset the stack pointer and the backtrace will stop at
uld_exec_elf_call_entry.

## Memory map
```
                                  **FLASH**
------------------------------------------------------------------------------
|                                  vectors                                   |
------------------------------------------------------------------------------
|                                    uld                                     |
------------------------------------------------------------------------------
|                             file system table                              |
------------------------------------------------------------------------------
|                               load module 0                                |
------------------------------------------------------------------------------
|                               load module 1                                |
------------------------------------------------------------------------------
|                                    ...                                     |
------------------------------------------------------------------------------
|                               load module n                                |
------------------------------------------------------------------------------

                           **EEPROM/end of FLASH**
------------------------------------------------------------------------------
|                      uld persistent data (uld_pdata)                       |
------------------------------------------------------------------------------
|                                    ...                                     |
------------------------------------------------------------------------------
|                     uld_pdata ptr (end of FLASH only)                      |
------------------------------------------------------------------------------

                                  **MEMORY**
------------------------------------------------------------------------------
|                          uld reserved runtime ptr                          |
------------------------------------------------------------------------------
|                       memory vectors (does not work)                       |
------------------------------------------------------------------------------
|                            uld reserved runtime                            |
------------------------------------------------------------------------------
|                               uld c runtime                                |
------------------------------------------------------------------------------
|                           loaded module 0 memory                           |
|                          .got/.got.plt/.data/.bss                          |
------------------------------------------------------------------------------
|                           loaded module 1 memory                           |
------------------------------------------------------------------------------
|                                    ...                                     |
------------------------------------------------------------------------------
|                           loaded module n memory                           |
------------------------------------------------------------------------------
|                       linker allocated loaded module                       |
|                     variables and function descriptors                     |
------------------------------------------------------------------------------
|      |                    heap (not implemented)                           |
|      |                                                                     |
|      |                                                                     |
|     \|/                                                                    |
------------------------------------------------------------------------------
|                                    ...                                     |
------------------------------------------------------------------------------
|     /|\                                                                    |
|      |                                                                     |
|      |                                                                     |
|      |                            stack                                    |
------------------------------------------------------------------------------
```

## Major TODOs
- [ ] Automatically add debug symbols.
  * Investigate debugger support defined in the
  [FDPIC ABI](https://github.com/mickael-guene/fdpic_doc/blob/master/abi.txt).
  See section "Dynamic Linker Reserve Area" and "Debugger Support - Overview".
    * Test GDB with FDPIC support (currently just building the new
    target with no testing done).  The new target may need work for FDPIC aware
    solib support.
    * If GDB does not work well without Linux present, possibly reuse debug
    structures and load symbols via an integrated Python script.
  * Trigger auto loading with GDB library events.
    * On Linux systems GDB handles this using system tap probes.
        * See [Static Probe Points](
        https://sourceware.org/gdb/onlinedocs/gdb/Static-Probe-Points.html)
        * GDB commands: `info probes` and `set stop-on-solib-events 1`
    * Leverage remote shared library events as possible solution for an
    embedded environment.
        * [Mailing list thread introducing patch](
        https://sourceware.org/ml/gdb-patches/2007-05/msg00145.html)
        * [Final patch](https://sourceware.org/git/gitweb.cgi?p=binutils-gdb.git;a=commit;h=cfa9d6d99135b1d59ecf0756247305cc24869549)
        * [GDB library stop reply description](https://sourceware.org/gdb/onlinedocs/gdb/Stop-Reply-Packets.html#index-shared-library-events_002c-remote-reply)
        * QEMU will have to notify GDB of a library event which hopefully GDB
        will respond with a [qXfer:libraries:read](https://sourceware.org/gdb/onlinedocs/gdb/General-Query-Packets.html#qXfer-library-list-read)
        packet.
        * Add support in QEMU's Angel [(ARM semihosting)](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0040d/Bcedijji.html)
        interface.
            * Modify [do_arm_semihosting](https://github.com/Cutty/qemu-uld/blob/gnuarmeclipse-dev-uld/target-arm/arm-semi.c#L256)
            and [gdb_vm_state_changed](https://github.com/Cutty/qemu-uld/blob/gnuarmeclipse-dev-uld/gdbstub.c#L1248).
            * Add new SWI opcode.
            * Use ADP_Stopped_OSSpecific in angel_SWIreason_ReportException
            (0x18).  This is mislabeled in QEMU as TARGET_SYS_EXIT (0x18).
- [ ] Working exceptions.
    * Current implementation uses configurable redirection via memory.
    Exceptions can get to the correct function but the FDPIC base register (r9)
    may not be setup correctly.
    * SCB_VTOR_TBLOFF will not work for the same reason.
    * Need to implement an exception dispatch that can set the correct FDPIC
    base register before calling the actual handler.  It will also have to save
    and restore r9 since this is not saved by the hardware (ARM DDI0403E
    B1.5.6) and is considered a caller saved register by the FDPIC ABI.
    * uld will have full control of which exceptions get passed to the
    application.
- [ ] Efficient way to use newlib.
    * Multilib mapping is set by [GCC target defs](https://github.com/Cutty/gcc-uld/blob/gcc-linaro-4.7-2013.05-cec-fdpic-uld/gcc/config/arm/t-bpabi-fdpic).
    Show current mapping with `arm-none-eabifdpic-gcc -print-multi-lib`.
    * Compilation looks correct but contains duplicate libraries.  Need to add
    armv7e-m architecture for Cortex-M4.  Same as armv7-m but with DSP.
    * Impractical to create a single monolithic shared library due to size.
    * Compile with `-fdata-sections -ffunction-sections -fvisibility=hidden`
    and place into a single archive.  Create a per-project list of functions
    and variables to change visibility from hidden to default (external).  Link
    the archive into a shared library with `--gc-sections`, ld will discard
    unused hidden functions and variables.  Unfortunately Developers will have
    to anticipate which functions they may need in the future to prevent
    updates of newlib.
    * objcopy does not have support for modifying visibility.
- [ ] Add support for heap/malloc.
    * uld heap
        * Currently only alloca (stack) is called which gets cumbersome when
        used as a replacement for malloc.
        * **Option 1:** Implement movable heap, requires all heap pointers to
        be registered/released so they can be updated on move.
        * **Option 2:** Place heap after C runtime with max size and trash the
        heap before calling into loaded modules including constructors.  An
        additional offset will be required when calculating VMAs to account for
        their final location.  Heap data required after calls must be placed in
        uld reserved runtime.
    * loaded module heap
        * All loaded modules must share a single instance of malloc/free.
        * uld will have to find and define sbrk base; newlib uses
        [linker defined 'end' symbol](https://github.com/Cutty/newlib-uld/blob/newlib-snapshot-20160923-uld/newlib/libc/sys/arm/syscalls.c#L457).
        This would have to be changed to an actual variable located in memory.
- [ ] Investigate binutils/ld not allocating space for some variables.
    * Example and explanation given in [ex_lib_print.c](https://github.com/Cutty/uld-fdpic/blob/master/src/example/ex_lib_print.c#L30).
    * Symbols remain so even though linking is done with `--no-undefined` no
    errors are generated.
    * Not reproducible with newer binutils, though tested with a different
    architecture.
    * Can not inspect variables with GDB due to invalid symbols.
- [ ] Add additional targets.
    * Cortex-M4 target; requires GCC target modification.
    * Cortex-M3 or M4 silicon target.

## References
* STMicroelectronics FDPIC presentation
    * [Presentation (YouTube)](https://www.youtube.com/watch?v=TNRNQNEcwVI)
    * [Slides](https://s3.amazonaws.com/connect.linaro.org/sfo15/Presentations/09-24-Thursday/SFO15-406-%20ARM%20FDPIC%20Toolchains.pdf)
* FDPIC toolchain/QEMU forked repositories
    * [gnuarmeclipse/qemu](https://github.com/gnuarmeclipse/qemu)
    * [mickael-guene/binutils](https://github.com/mickael-guene/binutils/tree/binutils-2.22-fdpic-m)
    * [mickael-guene/gcc](https://github.com/mickael-guene/gcc/tree/gcc-linaro-4.7-2013.05-cec-fdpic)
    * [mickael-guene/gdb](https://github.com/mickael-guene/gdb/tree/gdb-7.5.1-fdpic)
    * [mirror/newlib-cygwin](https://github.com/mirror/newlib-cygwin/tree/newlib-snapshot-20160923)
* FDPIC ABI documentation
    * [ARM](https://github.com/mickael-guene/fdpic_doc/blob/master/abi.txt)
    * [Blackfin](https://docs.blackfin.uclinux.org/doku.php?id=toolchain:application_binary_interface)
    * [FR-V](http://ftp.redhat.com/pub/redhat/gnupro/FRV/FDPIC-ABI.txt)
    * [SH](http://sourcery.mentor.com/public/docs/sh-fdpic/sh-fdpic-abi.txt)
* [ARMv7-M Architecture Reference Manual (requires free account)](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0403e.b/index.html)
* [Procedure Call Standard for the ARM Architecture](http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042f/IHI0042F_aapcs.pdf)
* [ELF format TIS v1.1](http://www.skyfree.org/linux/references/ELF_Format.pdf)
* [System V Application Binary Interface](http://www.sco.com/developers/gabi/latest/contents.html)

## License
See [LICENSE](LICENSE).
