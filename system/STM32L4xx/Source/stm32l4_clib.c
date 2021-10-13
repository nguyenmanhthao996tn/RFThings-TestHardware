/*
 * Copyright (c) 2016 Thomas Roell.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimers.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimers in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Thomas Roell, nor the names of its contributors
 *     may be used to endorse or promote products derived from this Software
 *     without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 */

#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

#include "armv7m.h"

int (*stm32l4_stdio_put)(char, FILE*) = NULL;
int (*stm32l4_stdio_get)(FILE*) = NULL;

#undef errno
extern int errno;

extern uint32_t __HeapBase[];
extern uint32_t __StackLimit[];

void * _sbrk (int nbytes)
{
    void *p;

    static void *__HeapCurrent = (void*)(&__HeapBase[0]);

    if (((uint8_t*)__HeapCurrent + nbytes) <= (uint8_t*)(&__StackLimit[0]))
    {
	p = __HeapCurrent;
	
	__HeapCurrent = (void*)((uint8_t*)__HeapCurrent + nbytes);

	return p;
    }
    else
    {
	errno = ENOMEM;

	return  (void *) -1;
    }
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;

    return -1;
}

int _close(int file) {
    return -1;
}

int _isatty(int file) 
{
    switch (file) {
    case STDOUT_FILENO:
    case STDERR_FILENO:
    case STDIN_FILENO:
        return 1;

    default:
	errno = EBADF;
	return 0;
    }
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;

    return 0;
}

int _lseek(int file, int offset, int whence)
{
    return 0;
}

int _read(int file, char *buf, int nbytes)
{
    int c, n;

    switch (file) {
    case STDIN_FILENO:
	n = 0;

	if (nbytes != 0)
	{
	    if (stm32l4_stdio_get != NULL)
	    {
		do
		{
		    c = (*stm32l4_stdio_get)(stdin);
		    
		    if (c == -1)
		    {
			break;
		    }

		    buf[n++] = c;
		    nbytes--;
		}
		while (nbytes != 0);
	    }
	}
	return n;

    default:
        errno = EBADF;
        return -1;
    }
}

int _write(int file, char *buf, int nbytes)
{
    int n;

    switch (file) {
    case STDOUT_FILENO:
    case STDERR_FILENO:
	n = 0;

	if (nbytes != 0)
	{
	    if (stm32l4_stdio_put != NULL)
	    {
		do
		{
		    if (!(*stm32l4_stdio_put)(buf[n], stdout))
		    {
			break;
		    }

		    n++;
		    nbytes--;
		}
		while (nbytes != 0);
	    }
	}
	return n;

    default:
	errno = EBADF;
	return -1;
    }
}

void _exit(int status) 
{
    while (1) { };
}
