/* $XConsortium: Xlibnet.h,v 1.36 94/01/11 12:35:09 mor Exp $ */

/*
Copyright 1991 Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/
/*
 * Xlibnet.h - Xlib networking include files for UNIX Systems.
 *
 * Note: Lots of the stuff originally found in this file moved
 *       to the common transport library.
 */

#ifndef WIN32

/*
 * If your BytesReadable correctly detects broken connections, then
 * you should NOT define XCONN_CHECK_FREQ.
 */
#define XCONN_CHECK_FREQ 256

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
#undef OPEN_MAX
#define OPEN_MAX 256
#endif

#ifdef USE_POLL
#include <sys/poll.h>
#endif

#if !defined(USE_POLL) || defined(STREAMSCONN)

#ifdef WORD64
#define NMSKBITS 64
#else
#define NMSKBITS 32
#endif

#define MSKCNT ((OPEN_MAX + NMSKBITS - 1) / NMSKBITS)

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
#define _XANYSET(src) (src[0])
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
#define _XANYSET(src) (src[0] || src[1])
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
#define _XANYSET(src) (src[0] || src[1] || src[2])
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
#define _XANYSET(src) (src[0] || src[1] || src[2] || src[3])
#endif

#if (MSKCNT>4)
#define COPYBITS(src, dst) memcpy((char *) dst, (char *) src, sizeof(FdSet))
#define CLEARBITS(buf) bzero((char *) buf, sizeof(FdSet))
#define MASKANDSETBITS(dst, b1, b2)  \
		      { int cri;			\
			for (cri=MSKCNT; --cri>=0; )	\
		          dst[cri] = (b1[cri] & b2[cri]); }
#define ORBITS(dst, b1, b2)  \
		      { int cri;			\
		      for (cri=MSKCNT; --cri>=0; )	\
		          dst[cri] = (b1[cri] | b2[cri]); }
#define UNSETBITS(dst, b1) \
		      { int cri;			\
		      for (cri=MSKCNT; --cri>=0; )	\
		          dst[cri] &= ~b1[cri];  }
/*
 * If MSKCNT>4, then _XANYSET is a routine defined in XlibInt.c.
 *
 * #define _XANYSET(src) (src[0] || src[1] || src[2] || src[3] || src[4] ...)
 */
#endif
#endif

#else /* not WIN32 */

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
#define GETBIT(set,s) FD_ISSET(s,&set)
#define _XANYSET(set) set.fd_count

#endif /* WIN32 */
