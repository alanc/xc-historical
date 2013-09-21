/* $XConsortium: Xpoll.h,v 1.2 95/04/05 19:57:45 kaleb Exp $ */

/*

Copyright (c) 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#ifndef _XPOLL_H_
#define _XPOLL_H_

#ifndef WIN32

#ifndef USE_POLL

#include <X11/Xos.h>
#if (defined(SVR4) || defined(CRAY) || defined(AIXV3)) && !defined(FD_SETSIZE)
#include <sys/select.h>
#ifdef luna
#include <sysent.h>
#endif
#endif
#include <X11/Xmd.h>
#ifdef CSRG_BASED
#include <sys/param.h>
# if BSD < 199103
typedef long fd_mask;
# endif
#endif

#define XFD_SETSIZE	256
#ifndef FD_SETSIZE
#define FD_SETSIZE	XFD_SETSIZE
#endif

#ifndef NBBY
#define NBBY	8		/* number of bits in a byte */
#endif

#ifndef NFDBITS
#define NFDBITS (sizeof(fd_mask) * NBBY)	/* bits per mask */
#endif

#ifndef howmany
#define howmany(x,y)	(((x)+((y)-1))/(y))
#endif

#ifdef BSD
# if BSD < 198911	/* 198911 == OSF/1, 199103 == CSRG_BASED */
#  ifndef luna		/* and even though on LUNA BSD ==  43, it has it */
typedef struct fd_set {
	fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;
#  endif
# endif
#endif

#ifdef hpux /* and perhaps old BSD ??? */
#define Select(n,r,w,e,t) select(n,(int*)r,(int*)w,(int*)e,(struct timeval*)t)
#else
#define Select(n,r,w,e,t) select(n,(fd_set*)r,(fd_set*)w,(fd_set*)e,(struct timeval*)t)
#endif

#ifndef FD_SET
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#endif
#ifndef FD_CLR
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#endif
#ifndef FD_ISSET
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#endif
#ifndef FD_ZERO
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))
#endif

/*
 * The following macros are used by the servers only. There is an
 * explicit assumption that the bit array in the fd_set is at least
 * 256 bits long (8 32-bit words). This is true on most modern POSIX 
 * systems. Note that this is merely an optimization for the servers
 * based on assumptions about the way that file descripters are
 * allocated on POSIX systems. 
 *
 * When porting X to new systems it is important to adjust these
 * macros if the system has fewer than 256 bits in the fd_set bit
 * array.
 */
#define XFD_ANYSET(p) \
		((p)->fds_bits[0] || (p)->fds_bits[1] || \
		(p)->fds_bits[2] || (p)->fds_bits[3] || \
		(p)->fds_bits[4] || (p)->fds_bits[5] || \
		(p)->fds_bits[6] || (p)->fds_bits[7])

#define XFD_COPYSET(src,dst) \
		(dst)->fds_bits[0] = (src)->fds_bits[0]; \
		(dst)->fds_bits[1] = (src)->fds_bits[1]; \
		(dst)->fds_bits[2] = (src)->fds_bits[2]; \
		(dst)->fds_bits[3] = (src)->fds_bits[3]; \
		(dst)->fds_bits[4] = (src)->fds_bits[4]; \
		(dst)->fds_bits[5] = (src)->fds_bits[5]; \
		(dst)->fds_bits[6] = (src)->fds_bits[6]; \
		(dst)->fds_bits[7] = (src)->fds_bits[7];

#define XFD_ANDSET(dst,b1,b2) \
		(dst)->fds_bits[0] = ((b1)->fds_bits[0] & (b2)->fds_bits[0]); \
		(dst)->fds_bits[1] = ((b1)->fds_bits[1] & (b2)->fds_bits[1]); \
		(dst)->fds_bits[2] = ((b1)->fds_bits[2] & (b2)->fds_bits[2]); \
		(dst)->fds_bits[3] = ((b1)->fds_bits[3] & (b2)->fds_bits[3]); \
		(dst)->fds_bits[4] = ((b1)->fds_bits[4] & (b2)->fds_bits[4]); \
		(dst)->fds_bits[5] = ((b1)->fds_bits[5] & (b2)->fds_bits[5]); \
		(dst)->fds_bits[6] = ((b1)->fds_bits[6] & (b2)->fds_bits[6]); \
		(dst)->fds_bits[7] = ((b1)->fds_bits[7] & (b2)->fds_bits[7]);

#define XFD_ORSET(dst,b1,b2) \
		(dst)->fds_bits[0] = ((b1)->fds_bits[0] | (b2)->fds_bits[0]); \
		(dst)->fds_bits[1] = ((b1)->fds_bits[1] | (b2)->fds_bits[1]); \
		(dst)->fds_bits[2] = ((b1)->fds_bits[2] | (b2)->fds_bits[2]); \
		(dst)->fds_bits[3] = ((b1)->fds_bits[3] | (b2)->fds_bits[3]); \
		(dst)->fds_bits[4] = ((b1)->fds_bits[4] | (b2)->fds_bits[4]); \
		(dst)->fds_bits[5] = ((b1)->fds_bits[5] | (b2)->fds_bits[5]); \
		(dst)->fds_bits[6] = ((b1)->fds_bits[6] | (b2)->fds_bits[6]); \
		(dst)->fds_bits[7] = ((b1)->fds_bits[7] | (b2)->fds_bits[7]);

#define XFD_UNSET(dst,b1) \
		(dst)->fds_bits[0] &= ~((b1)->fds_bits[0]); \
		(dst)->fds_bits[1] &= ~((b1)->fds_bits[1]); \
		(dst)->fds_bits[2] &= ~((b1)->fds_bits[2]); \
		(dst)->fds_bits[3] &= ~((b1)->fds_bits[3]); \
		(dst)->fds_bits[4] &= ~((b1)->fds_bits[4]); \
		(dst)->fds_bits[5] &= ~((b1)->fds_bits[5]); \
		(dst)->fds_bits[6] &= ~((b1)->fds_bits[6]); \
		(dst)->fds_bits[7] &= ~((b1)->fds_bits[7]);

#else /* USE_POLL */
#include <sys/poll.h>
#endif /* USE_POLL */

#else /* WIN32 */

#ifndef FD_SETSIZE
#define FD_SETSIZE      256
#endif
#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#define Select(n,r,w,e,t) select(0,(fd_set*)r,(fd_set*)w,(fd_set*)e,(struct timeval*)t)

/* 
 * These are merely placeholders for the present, to allow servers to
 * be compiled on WinNT. Eventually they need to be filled in.
 */
#define XFD_ANYSET(p)	1
#define XFD_COPYSET(src,dst)
#define XFD_ANDSET(dst,b1,b2)
#define XFD_ORSET(dst,b1,b2)
#define XFD_UNSET(dst,b1)

#endif /* WIN32 */

#endif /* _XPOLL_H_ */
