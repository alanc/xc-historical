/*      Copyright (c) 1993 NCR Corporation - Dayton, Ohio, USA */
/*        All Rights Reserved   */

#ifndef _XTRANSINT_H_
#define _XTRANSINT_H_

/*
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name NCR not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCR make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NCR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DEBUG will enable the PRMSG() macros used in the X Transport Interface code.
 * Each use of the PRMSG macro has a level associated with it. DEBUG is defined
 * to be a level. If the invocation level is =< the value of DEBUG, then the
 * message will be printed out to stderr. Recommended levels are:
 *
 *	DEBUG=1	Error messages
 *	DEBUG=2 API Function Tracing
 *	DEBUG=3 All Function Tracing
 *	DEBUG=4 printing of intermediate values
 *	DEBUG=5 really detailed stuff
#define DEBUG 2
 */

#include "Xtrans.h"
/*
#include <debug/malloc.h>
*/
#ifdef DEBUG
#include <stdio.h>
#include <errno.h>
#endif /* DEBUG */

typedef struct  _XtransConnInfo {
	struct _Xtransport     *transptr;
	char	*priv;
	int	flags;
	int	fd;
	int	family;
	char	*addr;
	int	addrlen;
	char	*peeraddr;
	int	peeraddrlen;
} XtransConnInfo;

typedef struct _Xtransport {
	char	*TransName;
	int	flags;
	XtransConnInfo	*(*OpenCOTSClient)(struct _Xtransport *, char *,
							char *, char *);
#define XTRANS_OPEN_COTS_CLIENT       1
	XtransConnInfo *(*OpenCOTSServer)(struct _Xtransport *, char *,
							char *, char *);
#define XTRANS_OPEN_COTS_SERVER       2
	XtransConnInfo	*(*OpenCLTSClient)(struct _Xtransport *, char *,
							char *, char *);
#define XTRANS_OPEN_CLTS_CLIENT       3
	XtransConnInfo	*(*OpenCLTSServer)(struct _Xtransport *, char *,
							char *, char *);
#define XTRANS_OPEN_CLTS_SERVER       4
	int	(*SetOption)(struct _XtransConnInfo *, int, int, int);
	int	(*CreateListener)(struct _XtransConnInfo *, int, char *);
	XtransConnInfo	*(*Accept)(struct _XtransConnInfo *, int);
	int	(*Connect)(struct _XtransConnInfo *, int, char *, char *);
	int	(*BytesReadable)(struct _XtransConnInfo *, int, BytesReadable_t *);
	int	(*Read)(struct _XtransConnInfo *, int, char *, int);
	int	(*Write)(struct _XtransConnInfo *, int, char *, int);
	int	(*Readv)(struct _XtransConnInfo *, int, struct iovec *, int);
	int	(*Writev)(struct _XtransConnInfo *, int, struct iovec *, int);
	int	(*Disconnect)(struct _XtransConnInfo *, int);
	int	(*Close)(struct _XtransConnInfo *, int);
	int	(*NameToAddr)(struct _XtransConnInfo *, int);
	int	(*AddrToName)(struct _XtransConnInfo *, int);
	} Xtransport;

/*
 * Flags for the flags member of Xtransport.
 */
#define TRANS_ALIAS	0x01	/* record is an alias, don't create server */

/*
 * readv() and writev() don't exist or don't work correctly on some
 * systems, so they may be emulated.
 */
#if defined(CRAY) || (defined(SYSV) && defined(SYSV386)) || defined(WIN32)

#define READV(fd, iov, iovcnt)	TRANS(ReadV)(fd, iov, iovcnt)

static	int TRANS(ReadV)(
#if NeedFunctionPrototypes
    int,		/* fd */
    struct iovec *,	/* iov */
    int			/* iovcnt */
#endif
);

#define WRITEV(fd, iov, iovcnt)	TRANS(WriteV)(fd, iov, iovcnt)

static int TRANS(WriteV)(
#if NeedFunctionPrototypes
    int,		/* fd */
    struct iovec *,	/* iov */
    int 		/* iovcnt */
#endif
);

#else

#define READV(fd, iov, iovcnt)	readv(fd, iov, iovcnt)
#define WRITEV(fd, iov, iovcnt)	writev(fd, iov, iovcnt)

#endif /* CRAY || (SYSV && SYSV386) || WIN32 */

/*
 * Some DEBUG stuff
 */
#if defined(DEBUG)
#define PRMSG(lvl,x,a,b,c)	if (lvl <= DEBUG){ \
			int saveerrno=errno; \
			fprintf(stderr, x,a,b,c); fflush(stderr); \
			errno=saveerrno; \
			}
#else
#define PRMSG(lvl,x,a,b,c)
#endif /* DEBUG */

#endif /* _XTRANSINT_H_ */
