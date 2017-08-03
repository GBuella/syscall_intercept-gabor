/*
 * Copyright 2016-2017, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "intercept_util.h"
#include "intercept.h"

#include <assert.h>
#include <inttypes.h>
#include <ctype.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sched.h>
#include <linux/limits.h>

static long log_fd = -1;

void *
xmmap_anon(size_t size)
{
	void *addr = (void *) syscall_no_intercept(SYS_mmap,
				NULL, size,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANON, -1, 0);

	if (addr == MAP_FAILED)
		xabort("xmmap_anon");

	return addr;
}

void *
xmremap(void *addr, size_t old, size_t new)
{
	void *new_addr;

#ifdef SYS_mremap
	new_addr = (void *) syscall_no_intercept(SYS_mremap, addr,
				old, new, MREMAP_MAYMOVE);

	if (new_addr == MAP_FAILED)
		xabort("xmremap");

#else
	void *new_addr = xmmap_anon(new);
	if (new_addr == MAP_FAILED)
		xabort("xmremap");
	memcpy(new_addr, addr, (new > old) ? old : new);
	xmunmap(addr, old);

#endif

	return new_addr;
}

void
xmunmap(void *addr, size_t len)
{
	if (syscall_no_intercept(SYS_munmap, addr, len) != 0)
		xabort("xmunmap");
}

long
xlseek(long fd, unsigned long off, int whence)
{
	long result = syscall_no_intercept(SYS_lseek, fd, off, whence);

	if (result < 0)
		xabort("xlseek");

	return result;
}

void
xread(long fd, void *buffer, size_t size)
{
	if (syscall_no_intercept(SYS_read, fd,
	    (long)buffer, (long)size) != (long)size)
		xabort("xread");
}
