/*
* $XConsortium: fd.h,v 1.18 93/09/11 16:50:18 rws Exp $
* $oHeader: fd.h,v 1.4 88/08/26 14:49:54 asente Exp $
*/

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _Xt_fd_set
#define _Xt_fd_set

#ifdef WIN32

#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif
#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#define Fd_set fd_set

#else /* !WIN32 */

#if (defined(SVR4) || defined(CRAY) || defined(AIXV3)) && !defined(FD_SETSIZE)
#include <sys/select.h>		/* defines FD stuff except howmany() */
#endif

#ifndef NBBY
#define	NBBY	8		/* number of bits in a byte */
#endif
/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

#ifdef LONG64
typedef int Fd_mask;
#else
typedef long Fd_mask;
#endif
#ifndef NFDBITS
#define NFDBITS	(sizeof(Fd_mask) * NBBY)	/* bits per mask */
#endif
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef	struct Fd_set {
	Fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} Fd_set;

#ifndef FD_SET
#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#endif
#ifndef FD_CLR
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#endif
#ifndef FD_ISSET
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#endif
#ifndef FD_ZERO
#define FD_ZERO(p)	memset((char *)(p), 0, sizeof(*(p)))
#endif

#endif /* WIN32 */

#endif /*_Xt_fd_set*/
