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


FILE _SWI_STDIN __export = {
    .write_f = swi_write,
    .dputchar_f = swi_dputchar,
    .fd = SWI_STDIN_FILENO
};

FILE _SWI_STDOUT __export = {
    .write_f = swi_write,
    .dputchar_f = swi_dputchar,
    .fd = SWI_STDOUT_FILENO
};

FILE _SWI_STDERR __export = {
    .write_f = swi_write,
    .dputchar_f = swi_dputchar,
    .fd = SWI_STDERR_FILENO
};


// SWI general information - see ARM DUI0040D
// 13.3.4 Angel task management:
//      Simple/Complex SWI description.
// 13.7.1 Angel task management and SWIs:
//      Simple/Complex SWI corrupted registers.
// 13.7.2 SYS_OPEN (0x01):
//      stdin/stdout are opened automatically>
int swi_write(int fd, const void *buf, size_t count)
{
    int32_t centisec;
    int block[3] = {fd, (int)buf, (int)count};
    int retries = 5;
    int iter;
    int ret;

    if ((fd != 1 && fd != 2) || !buf) {
        return SWI_EOF;
    }
    if (!count) {
        return 0;
    }

    do {
        // 13.7.6 SYS_WRITE (0x05): Entry/Return values.
        // Entry:
        // R0 = SWI_WRITE 0x05
        // R1 = Pointer to block
        // block[0] = file handle.
        // block[1] = write data pointer.
        // block[2] = number of bytes to write.
        // Return: 0 on success, number of bytes not written on error.
        asm volatile ("mov r0, #0x05\n\t"
                "mov r1, %1\n\t"
                "bkpt #0x00ab\n\t"
                "mov %0, r0\n\t"
                : "=r" (ret)
                : "r" (block)
                : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc");

        // While not part of the Angel spec QEMU will return -1 if SYS_WRITE
        // is called too fast.  In that case wait and retry.
        if (ret == -1) {
            centisec = swi_clock();
            // Wait at least 200ms before trying to write again.
            while ((swi_clock() - centisec) < 20) {
#ifdef QEMU
                iter = 100000;
                while (iter--) {
                    ret = (int)cpu_get_pc();
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 100;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 200;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 300;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 400;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 500;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 600;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 700;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 800;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                    ret /= 900;
                    asm volatile ("mov %0, %0\n\t" : "+r" (ret) ::);
                }
#else  // QEMU
#error Real hardware should use a different delay method
#endif  // QEMU
            }
        }
    } while (--retries && ret == -1);

    if (ret == -1) {
        ret = SWI_EOF;
    } else {
        ret = count - ret;
    }

    return ret;
}

int swi_dputchar(int fd, int c) {
    char buf = (char)c;
    if (swi_write(fd, &buf, 1) == 1) {
        return (unsigned char)c;
    } else {
        return SWI_EOF;
    }
}

int32_t swi_clock(void)
{
    int32_t ret;

    // 13.7.17 SYS_CLOCK (0x10): Entry/Return values.
    // Entry:
    // R0 = SWI_CLOCK 0x10
    // R1 = 0
    // Return: number of centiseconds since support code execution.
    asm volatile ("mov r0, #0x10\n\t"
            "mov r1, $0\n\t"
            "bkpt #0x00ab\n\t"
            "mov %0, r0\n\t"
            : "=r" (ret)
            :
            : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc");
    return ret;
}

time_t swi_time(void)
{
    int32_t ret;

    // 13.7.17 SYS_TIME (0x11): Entry/Return values.
    // Entry:
    // R0 = SWI_TIME 0x11
    // R1 = 0
    // Return: number of seconds.
    asm volatile ("mov r0, #0x11\n\t"
            "mov r1, $0\n\t"
            "bkpt #0x00ab\n\t"
            "mov %0, r0\n\t"
            : "=r" (ret)
            :
            : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc");
    return (time_t)ret;
}

void swi_exit(void)
{
    // Entry:
    // R0 = angel_SWIreason_ReportException (0x18) (13.8.2).
    // R1 = ADP_Stopped_ApplicationExit 0x20026 (Table 13.6).
    // Return: Does not return.
    asm volatile ("mov r0, #0x18\n\t"
            "movw r1, #0x0026\n\t"
            "movt r1, #0x0002\n\t"
            "bkpt #0x00ab\n\t");

    // Never reached, suppress warning.
    while (1);
}

int swi_printf(const char *format, ...)
{
    va_list ap;
    int ret;
    va_start(ap, format);
    ret = _uld_vfprintf(SWI_STDOUT, format, ap);
    va_end(ap);
    return ret;
}

int swi_dprintf(int fd, const char *format, ...)
{
    va_list ap;
    int ret;
    FILE stream;
    _uld_memcpy(&stream, SWI_STDOUT, sizeof(FILE));
    stream.fd = fd;
    va_start(ap, format);
    ret = _uld_vfprintf(SWI_STDOUT, format, ap);
    va_end(ap);
    return ret;
}

int swi_vdprintf(int fd, const char *format, va_list ap)
{
    FILE stream;
    _uld_memcpy(&stream, SWI_STDOUT, sizeof(FILE));
    stream.fd = fd;
    return _uld_vfprintf(&stream, format, ap);
}
