/*
 * $XConsortium:  $
 *
 * 
Copyright (c) 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 * *
 */

#ifndef _XPOLL_H_
#define _XPOLL_H_

#ifndef USE_POLL

#ifndef WIN32

#include <X11/Xos.h>
#if (defined(SVR4) || defined(CRAY) || defined(AIXV3)) && !defined(FD_SETSIZE)
#include <sys/select.h>
#endif
#include <X11/Xmd.h>
#ifdef CSRG_BASED
#include <sys/param.h>
# if BSD < 199103
typedef long fd_mask;
# endif
#endif

#ifndef NBBY
#define NBBY	8		/* number of bits in a byte */
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE	256
#endif

#ifndef NFDBITS
#define NFDBITS (sizeof(fd_mask) * NBBY)	/* bits per mask */
#endif

#ifndef howmany
#define howmany(x,y)	(((x)+((y)-1))/(y))
#endif

#ifdef BSD
# if BSD < 199103
typedef struct fd_set {
	fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;
# endif
#endif
#define Fd_set fd_set

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
#define Fd_set fd_set

#endif /* WIN32 */

#else /* USE_POLL */

#include <sys/poll.h>

#endif /* USE_POLL */

#endif /* _XPOLL_H_ */
