/* $XConsortium: Xlibnet.h,v 1.34 93/09/25 13:54:40 rws Exp $ */

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
 */

#ifdef LONG64
typedef int BytesReadable_t;
#else
typedef long BytesReadable_t;
#endif

#ifndef WIN32

#ifndef X_UNIX_PATH
#ifdef hpux
#define X_UNIX_PATH "/usr/spool/sockets/X11/"
#define OLD_UNIX_PATH "/tmp/.X11-unix/X"
#else
#define X_UNIX_PATH "/tmp/.X11-unix/X"
#endif
#endif /* X_UNIX_PATH */

#ifdef STREAMSCONN
#ifdef SYSV
/*
 * UNIX System V Release 3.2
 */
#define BytesReadable(fd,ptr) (_XBytesReadable ((fd), (ptr)))
#include <sys/ioctl.h>

#endif /* SYSV */
#ifdef SVR4
/*
 * TLI (Streams-based) networking
 */
#define BytesReadable(fd,ptr) (_XBytesReadable ((fd), (ptr)))
#include <sys/uio.h>		/* define struct iovec */

#endif /* SVR4 */
#else /* not STREAMSCONN */
/*
 * socket-based systems
 */
#if defined(TCPCONN) || defined(UNIXCONN) || defined(DNETCONN)
#include <netinet/in.h>
#else
#ifdef ESIX
#include <lan/in.h>
#endif
#endif
#include <sys/ioctl.h>
#if defined(TCPCONN) || defined(UNIXCONN) || defined(DNETCONN)
#include <netdb.h>
#endif
#include <sys/uio.h>	/* needed for XlibInt.c */
#ifdef SVR4
#include <sys/filio.h>
#endif

#if (defined(SYSV386) && defined(SYSV)) || defined(_SEQUENT_)
#if !defined(_SEQUENT_) && !defined(ESIX)
#include <net/errno.h>
#endif /* _SEQUENT_  || ESIX */
#include <sys/stropts.h>
#define BytesReadable(fd,ptr) ioctl((fd), I_NREAD, (char *)(ptr))
#else
#define BytesReadable(fd, ptr) ioctl ((fd), FIONREAD, (char *)(ptr))
#endif

#endif /* STREAMSCONN else */

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
#if defined(USG) && !defined(CRAY) && !defined(umips) && !defined(MOTOROLA) && !defined(uniosu)
struct iovec {
    caddr_t iov_base;
    int iov_len;
};
#endif /* USG */


#ifdef STREAMSCONN
#include "Xstreams.h"

extern char _XsTypeOfStream[];
extern Xstream _XsStream[];

#define ReadFromServer(dpy, data, size) \
	(*_XsStream[_XsTypeOfStream[dpy]].ReadFromStream)((dpy), (data), (size), \
						     BUFFERING)
#define WriteToServer(dpy, bufind, size) \
	(*_XsStream[_XsTypeOfStream[dpy]].WriteToStream)((dpy), (bufind), (size))

#else /* else not STREAMSCONN */

/*
 * bsd can read from sockets directly
 */
#define ReadFromServer(dpy, data, size) read((dpy), (data), (size))
#define WriteToServer(dpy, bufind, size) write((dpy), (bufind), (size))

#ifndef USL_COMPAT
#if !defined(USG) || defined(MOTOROLA) || defined(uniosu)
#if !(defined(SYSV) && defined(SYSV386))
#define _XReadV readv
#endif
#define _XWriteV writev
#endif
#endif /* !USL_COMPAT */

#endif /* STREAMSCONN */

#else /* not WIN32 */

#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#include <X11/Xw32defs.h>

#define BytesReadable(fd,ptr) ioctlsocket((SOCKET)fd, FIONREAD, (u_long *)ptr)
#define ReadFromServer(dpy,data,size) recv((SOCKET)(dpy), data, size, 0)
#define WriteToServer(dpy,data,size) send((SOCKET)(dpy), data, size, 0)

struct iovec {
    caddr_t iov_base;
    int iov_len;
};

typedef fd_set FdSet;

#define CLEARBITS(set) FD_ZERO(&set)
#define BITSET(set,s) FD_SET(s,&set)
#define GETBIT(set,s) FD_ISSET(s,&set)
#define _XANYSET(set) set.fd_count

#endif /* WIN32 */

#define ReadvFromServer(dpy, iov, iovcnt) _XReadV((dpy), (iov), (iovcnt))
#define WritevToServer(dpy, iov, iovcnt) _XWriteV((dpy), (iov), (iovcnt))
