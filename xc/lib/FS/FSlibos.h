#include	"copyright.h"
/* @(#)FSlibos.h	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

/*
 * FSlib networking & os include file
 */

#include <X11/Xfuncs.h>
#include <X11/Xosdefs.h>

#ifdef att
/*
 * UNIX System V Release 3.2
 */
#include <sys/stropts.h>
#define BytesReadable(fd,ptr) (_FSBytesReadable ((fd), (ptr)))
#define MALLOC_0_RETURNS_NULL
#include <sys/ioctl.h>
#include <sys/param.h>
#define MAXSOCKS (NOFILES_MAX)
#define MSKCNT ((MAXSOCKS + 31) / 32)

#else
/*
 * 4.2BSD-based systems
 */
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/uio.h>		/* needed for FSlibInt.c */
#include <sys/param.h>		/* needed for FSConnDis.c */

#define BytesReadable(fd, ptr) ioctl ((fd), FIONREAD, (ptr))
#define MSKCNT ((NOFILE + 31) / 32)	/* size of bit array */
#endif				/* att else bsdish */


/* Utek leaves kernel macros around in include files (bleah) */

#ifdef dirty
#undef dirty
#endif

#ifdef CRAY
#define WORD64
#define MALLOC_0_RETURNS_NULL
#endif

#if (MSKCNT==1)
#define BITMASK(i) (1 << (i))
#define MASKIDX(i) 0
#endif

#if (MSKCNT>1)
#define BITMASK(i) (1 << ((i) & 31))
#define MASKIDX(i) ((i) >> 5)
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
#define ANYSET(src) (src[0])
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
#define ANYSET(src) (src[0] || src[1])
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
#define ANYSET(src) (src[0] || src[1] || src[2])
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
#define ANYSET(src) (src[0] || src[1] || src[2] || src[3])
#endif

#if (MSKCNT>4)
#define COPYBITS(src, dst) bcopy((caddr_t) src, (caddr_t) dst,\
				 MSKCNT*sizeof(long))
#define CLEARBITS(buf) bzero((caddr_t) buf, MSKCNT*sizeof(long))
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
/*
 * If MSKCNT>4, then ANYSET is a routine defined in FSlibInt.c.
 *
 * #define ANYSET(src) (src[0] || src[1] || src[2] || src[3] || src[4] ...)
 */
#endif

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
#if defined(macII) && !defined(__STDC__)  /* stdlib.h fails to define these */
char *malloc(), *realloc(), *calloc();
#endif /* macII */

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

#if !defined(CRAY) && !defined(umips)
struct iovec {
    caddr_t     iov_base;
    int         iov_len;
};

#ifndef __TIMEVAL__
#define __TIMEVAL__
struct timeval {		/* BSD has in <sys/time.h> */
    long        tv_sec;
    long        tv_usec;
};

#endif				/* __TIMEVAL__ */

#endif				/* not CRAY or umips */

#endif				/* USG */


#ifdef STREAMSCONN
#include "FSstreams.h"

#if (!defined(EWOULDBLOCK)) && defined(EAGAIN)
#define EWOULDBLOCK EAGAIN
#endif

extern char _FSsTypeOfStream[];
extern FSstream _FSsStream[];

#define ReadFromServer(svr, data, size) \
	(*_FSsStream[_FSsTypeOfStream[svr]].ReadFromStream)((svr), (data), (size), \
						     BUFFERING)
#define WriteToServer(svr, bufind, size) \
	(*_FSsStream[_FSsTypeOfStream[svr]].WriteToStream)((svr), (bufind), (size))

#else				/* else not STREAMSCONN */

/*
 * bsd can read from sockets directly
 */
#define ReadFromServer(svr, data, size) read((svr), (data), (size))
#define WriteToServer(svr, bufind, size) write((svr), (bufind), (size))
#endif				/* STREAMSCONN */


#ifndef USG
#define _FSReadV readv
#define _FSWriteV writev
#endif

#define ReadvFromServer(svr, iov, iovcnt) _FSReadV((svr), (iov), (iovcnt))
#define WritevToServer(svr, iov, iovcnt) _FSWriteV((svr), (iov), (iovcnt))

#ifndef sgi
extern char *index();

#endif

#define SearchString(string, char) index((string), (char))
