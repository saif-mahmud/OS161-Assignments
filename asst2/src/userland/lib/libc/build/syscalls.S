/* Automatically generated; do not edit */
/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * This file is copied to syscalls.S, and then the actual syscalls are
 * appended as lines of the form
 *    SYSCALL(symbol, number)
 *
 * Warning: gccs before 3.0 run cpp in -traditional mode on .S files.
 * So if you use an older gcc you'll need to change the token pasting
 * in SYSCALL().
 */

#include <kern/syscall.h>
#include <machine/regdefs.h>

/*
 * Definition for each syscall.
 * All we do is load the syscall number into v0, the register the
 * kernel expects to find it in, and jump to the shared syscall code.
 * (Note that the addiu instruction is in the jump's delay slot.)
 */
#define SYSCALL(sym, num) \
   .set noreorder		; \
   .globl sym			; \
   .type sym,@function		; \
   .ent sym			; \
sym:				; \
   j __syscall                  ; \
   addiu v0, $0, SYS_##sym	; \
   .end sym			; \
   .set reorder

/*
 * Now, the shared system call code.
 * The MIPS syscall ABI is as follows:
 *
 *    On entry, call number in v0. The rest is like a normal function
 *    call: four args in a0-a3, the other args on the stack.
 *
 *    On successful return, zero in a3 register; return value in v0
 *    (v0 and v1 for a 64-bit return value).
 *
 *    On error return, nonzero in a3 register; errno value in v0.
 *
 * The use of a3 as a return register to hold the success flag is
 * gross, but I didn't make it up.
 *
 * Note that by longstanding Unix convention and POSIX decree, errno
 * is not to be set unless the call actually fails.
 */

   .set noreorder
   .text
   .type __syscall,@function
   .ent __syscall
__syscall:
   syscall              /* make system call */
   beq a3, $0, 1f       /* if a3 is zero, call succeeded */
   nop			/* delay slot */
   sw v0, errno        /* call failed: store errno */
   li v1, -1		/* and force return value to -1 */
   li v0, -1
1:
   j ra			/* return */
   nop			/* delay slot */
   .end __syscall
   .set reorder

SYSCALL(fork, 0)
SYSCALL(vfork, 1)
SYSCALL(execv, 2)
SYSCALL(_exit, 3)
SYSCALL(waitpid, 4)
SYSCALL(getpid, 5)
SYSCALL(getppid, 6)
SYSCALL(sbrk, 7)
SYSCALL(mmap, 8)
SYSCALL(munmap, 9)
SYSCALL(mprotect, 10)
SYSCALL(umask, 17)
SYSCALL(issetugid, 18)
SYSCALL(getresuid, 19)
SYSCALL(setresuid, 20)
SYSCALL(getresgid, 21)
SYSCALL(setresgid, 22)
SYSCALL(getgroups, 23)
SYSCALL(setgroups, 24)
SYSCALL(__getlogin, 25)
SYSCALL(__setlogin, 26)
SYSCALL(kill, 27)
SYSCALL(sigaction, 28)
SYSCALL(sigpending, 29)
SYSCALL(sigprocmask, 30)
SYSCALL(sigsuspend, 31)
SYSCALL(sigreturn, 32)
SYSCALL(open, 45)
SYSCALL(pipe, 46)
SYSCALL(dup, 47)
SYSCALL(dup2, 48)
SYSCALL(close, 49)
SYSCALL(read, 50)
SYSCALL(pread, 51)
SYSCALL(getdirentry, 54)
SYSCALL(write, 55)
SYSCALL(pwrite, 56)
SYSCALL(lseek, 59)
SYSCALL(flock, 60)
SYSCALL(ftruncate, 61)
SYSCALL(fsync, 62)
SYSCALL(fcntl, 63)
SYSCALL(ioctl, 64)
SYSCALL(select, 65)
SYSCALL(poll, 66)
SYSCALL(link, 67)
SYSCALL(remove, 68)
SYSCALL(mkdir, 69)
SYSCALL(rmdir, 70)
SYSCALL(mkfifo, 71)
SYSCALL(rename, 72)
SYSCALL(access, 73)
SYSCALL(chdir, 74)
SYSCALL(fchdir, 75)
SYSCALL(__getcwd, 76)
SYSCALL(symlink, 77)
SYSCALL(readlink, 78)
SYSCALL(mount, 79)
SYSCALL(unmount, 80)
SYSCALL(stat, 81)
SYSCALL(fstat, 82)
SYSCALL(lstat, 83)
SYSCALL(utimes, 84)
SYSCALL(futimes, 85)
SYSCALL(lutimes, 86)
SYSCALL(chmod, 87)
SYSCALL(chown, 88)
SYSCALL(fchmod, 89)
SYSCALL(fchown, 90)
SYSCALL(lchmod, 91)
SYSCALL(lchown, 92)
SYSCALL(socket, 98)
SYSCALL(bind, 99)
SYSCALL(connect, 100)
SYSCALL(listen, 101)
SYSCALL(accept, 102)
SYSCALL(shutdown, 104)
SYSCALL(getsockname, 105)
SYSCALL(getpeername, 106)
SYSCALL(getsockopt, 107)
SYSCALL(setsockopt, 108)
SYSCALL(__time, 113)
SYSCALL(__settime, 114)
SYSCALL(nanosleep, 115)
SYSCALL(sync, 118)
SYSCALL(reboot, 119)
