/* $XConsortium: FSlibos.h,v 1.18 94/01/31 11:47:19 mor Exp $ */

/* @(#)FSlibos.h	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * FSlib networking & os include file
 */

#include <X11/Xfuncs.h>
#include <X11/Xosdefs.h>

#ifndef WIN32

/*
 * makedepend screws up on #undef OPEN_MAX, so we define a new symbol
 */

#ifndef FS_OPEN_MAX

#ifndef X_NOT_POSIX
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif
#endif
#ifndef OPEN_MAX
#ifdef SVR4
#define OPEN_MAX 256
#else
#include <sys/param.h>
#ifndef OPEN_MAX
#ifdef NOFILE
#define OPEN_MAX NOFILE
#else
#define OPEN_MAX NOFILES_MAX
#endif
#endif
#endif
#endif

#if OPEN_MAX > 256
#define FS_OPEN_MAX 256
#else
#define FS_OPEN_MAX OPEN_MAX
#endif

#endif /* FS_OPEN_MAX */

/* Utek leaves kernel macros around in include files (bleah) */

#ifdef dirty
#undef dirty
#endif

#ifdef WORD64
#define NMSKBITS 64
#else
#define NMSKBITS 32
#endif

#define MSKCNT ((FS_OPEN_MAX + NMSKBITS - 1) / NMSKBITS)

#ifdef LONG64
typedef unsigned int FdSet[MSKCNT];
#else
typedef unsigned long FdSet[MSKCNT];
#endif

#if (MSKCNT==1)
#define BITMASK(i) (1 << (i))
#define MASKIDX(i) 0
#endif

#if (MSKCNT>1)
#define BITMASK(i) (1 << ((i) & (NMSKBITS - 1)))
#define MASKIDX(i) ((i) / NMSKBITS)
#endif

#define MASKWORD(buf, i) buf[MASKIDX(i)]
#define BITSET(buf, i) MASKWORD(buf, i) |= BITMASK(i)
#define BITCLEAR(buf, i) MASKWORD(buf, i) &= ~BITMASK(i)
#define GETBIT(buf, i) (MASKWORD(buf, i) & BITMASK(i))

#if (MSKCNT==1)
#define COPYBITS(src, dst) dst[0] = src[0]
#define CLEARBITS(buf) buf[0] = 0
#define MASKANDSETBITS(dst, b1, b2) dst[0] = (b1[0] & b2[0])
#define ORBITS(dst, b1, b2) dst[0] = (b1[0] | b2[0])
#define UNSETBITS(dst, b1) (dst[0] &= ~b1[0])
#define _FSANYSET(src) (src[0])
#endif

#if (MSKCNT==2)
#define COPYBITS(src, dst) { dst[0] = src[0]; dst[1] = src[1]; }
#define CLEARBITS(buf) { buf[0] = 0; buf[1] = 0; }
#define MASKANDSETBITS(dst, b1, b2)  {\
		      dst[0] = (b1[0] & b2[0]);\
		      dst[1] = (b1[1] & b2[1]); }
#define ORBITS(dst, b1, b2)  {\
		      dst[0] = (b1[0] | b2[0]);\
		      dst[1] = (b1[1] | b2[1]); }
#define UNSETBITS(dst, b1) {\
                      dst[0] &= ~b1[0]; \
                      dst[1] &= ~b1[1]; }
#define _FSANYSET(src) (src[0] || src[1])
#endif

#if (MSKCNT==3)
#define COPYBITS(src, dst) { dst[0] = src[0]; dst[1] = src[1]; \
			     dst[2] = src[2]; }
#define CLEARBITS(buf) { buf[0] = 0; buf[1] = 0; buf[2] = 0; }
#define MASKANDSETBITS(dst, b1, b2)  {\
		      dst[0] = (b1[0] & b2[0]);\
		      dst[1] = (b1[1] & b2[1]);\
		      dst[2] = (b1[2] & b2[2]); }
#define ORBITS(dst, b1, b2)  {\
		      dst[0] = (b1[0] | b2[0]);\
		      dst[1] = (b1[1] | b2[1]);\
		      dst[2] = (b1[2] | b2[2]); }
#define UNSETBITS(dst, b1) {\
                      dst[0] &= ~b1[0]; \
                      dst[1] &= ~b1[1]; \
                      dst[2] &= ~b1[2]; }
#define _FSANYSET(src) (src[0] || src[1] || src[2])
#endif

#if (MSKCNT==4)
#define COPYBITS(src, dst) dst[0] = src[0]; dst[1] = src[1]; \
			   dst[2] = src[2]; dst[3] = src[3]
#define CLEARBITS(buf) buf[0] = 0; buf[1] = 0; buf[2] = 0; buf[3] = 0
#define MASKANDSETBITS(dst, b1, b2)  \
                      dst[0] = (b1[0] & b2[0]);\
                      dst[1] = (b1[1] & b2[1]);\
                      dst[2] = (b1[2] & b2[2]);\
                      dst[3] = (b1[3] & b2[3])
#define ORBITS(dst, b1, b2)  \
                      dst[0] = (b1[0] | b2[0]);\
                      dst[1] = (b1[1] | b2[1]);\
                      dst[2] = (b1[2] | b2[2]);\
                      dst[3] = (b1[3] | b2[3])
#define UNSETBITS(dst, b1) \
                      dst[0] &= ~b1[0]; \
                      dst[1] &= ~b1[1]; \
                      dst[2] &= ~b1[2]; \
                      dst[3] &= ~b1[3]
#define _FSANYSET(src) (src[0] || src[1] || src[2] || src[3])
#endif

#if (MSKCNT>4)
#define COPYBITS(src, dst) bcopy((caddr_t) src, (caddr_t) dst, sizeof(FdSet))
#define CLEARBITS(buf) bzero((caddr_t) buf, sizeof(FdSet))
#define MASKANDSETBITS(dst, b1, b2)  \
		      { int cri;			\
			for (cri=0; cri<MSKCNT; cri++)	\
		          dst[cri] = (b1[cri] & b2[cri]) }
#define ORBITS(dst, b1, b2)  \
		      { int cri;			\
		      for (cri=0; cri<MSKCNT; cri++)	\
		          dst[cri] = (b1[cri] | b2[cri]) }
#define UNSETBITS(dst, b1) \
		      { int cri;			\
		      for (cri=0; cri<MSKCNT; cri++)	\
		          dst[cri] &= ~b1[cri];  }
#if (MSKCNT==8)
#define _FSANYSET(src) (src[0] || src[1] || src[2] || src[3] || \
			src[4] || src[5] || src[6] || src[7])
#endif
/*
 * If MSKCNT>4 and not 8, then _FSANYSET is a routine defined in FSlibInt.c.
 *
 * #define _FSANYSET(src) (src[0] || src[1] || src[2] || src[3] || src[4] ...)
 */
#endif


#else

#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#include <X11/Xw32defs.h>

typedef fd_set FdSet;

#define CLEARBITS(set) FD_ZERO(&set)
#define BITSET(set,s) FD_SET(s,&set)
#define _FSANYSET(set) set.fd_count

#endif

#include <X11/Xtrans.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#include <string.h>
#else
char *malloc(), *realloc(), *calloc();
void exit();
#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif
#endif

/*
 * The following definitions can be used for locking requests in multi-threaded
 * address spaces.
 */
#define LockDisplay(dis)
#define LockMutex(mutex)
#define UnlockMutex(mutex)
#define UnlockDisplay(dis)
#define FSfree(ptr) free((ptr))


/*
 * Note that some machines do not return a valid pointer for malloc(0), in
 * which case we provide an alternate under the control of the
 * define MALLOC_0_RETURNS_NULL.  This is necessary because some
 * FSlib code expects malloc(0) to return a valid pointer to storage.
 */

#ifdef MALLOC_0_RETURNS_NULL
#define FSmalloc(size) malloc(((size) > 0 ? (size) : 1))
#define FSrealloc(ptr, size) realloc((ptr), ((size) > 0 ? (size) : 1))
#define FScalloc(nelem, elsize) calloc(((nelem) > 0 ? (nelem) : 1), (elsize))

#else

#define FSmalloc(size) malloc((size))
#define FSrealloc(ptr, size) realloc((ptr), (size))
#define FScalloc(nelem, elsize) calloc((nelem), (elsize))
#endif

/*
 *	ReadvFromServer and WritevToSever use struct iovec, normally found
 *	in Berkeley systems in <sys/uio.h>.  See the readv(2) and writev(2)
 *	manual pages for details.
 *
 *	struct iovec {
 *		caddr_t iov_base;
 *		int iov_len;
 *	};
 */

#ifdef USG

#if defined(USG) && !defined(CRAY) && !defined(umips) && !defined(MOTOROLA)
#ifndef __TIMEVAL__
#define __TIMEVAL__
struct timeval {		/* BSD has in <sys/time.h> */
    long        tv_sec;
    long        tv_usec;
};

#endif				/* __TIMEVAL__ */

#endif				/* not CRAY or umips */

#endif				/* USG */


#define SearchString(string, char) index((string), (char))
