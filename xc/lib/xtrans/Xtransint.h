#ifndef _XTRANSINT_H_
#define _XTRANSINT_H_

/*      Copyright (c) 1993 NCR Corporation - Dayton, Ohio, USA */
/*        All Rights Reserved   */

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

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

#include <errno.h>
#ifdef X_NOT_STDC_ENV
extern int  errno;		/* Internal system error number. */
#endif

#ifndef WIN32
#include <sys/socket.h>

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

#define ESET(val) errno = val
#define EGET() errno

#else /* WIN32 */

#define ESET(val) WSASetLastError(val)
#define EGET() WSAGetLastError()

#endif /* WIN32 */

#ifndef NULL
#define NULL 0
#endif

#ifdef X11
#define X_TCP_PORT	6000
#endif

struct _XtransConnInfo {
    struct _Xtransport     *transptr;
    char	*priv;
    int		flags;
    int		fd;
    int		family;
    char	*addr;
    int		addrlen;
    char	*peeraddr;
    int		peeraddrlen;
};

#define XTRANS_OPEN_COTS_CLIENT       1
#define XTRANS_OPEN_COTS_SERVER       2
#define XTRANS_OPEN_CLTS_CLIENT       3
#define XTRANS_OPEN_CLTS_SERVER       4


typedef struct _Xtransport {
    char	*TransName;
    int		flags;

    XtransConnInfo (*OpenCOTSClient)(
#if NeedNestedPrototypes
	struct _Xtransport *,
	char *,
	char *,
	char *
#endif
    );

    XtransConnInfo (*OpenCOTSServer)(
#if NeedNestedPrototypes
	struct _Xtransport *,
	char *,
	char *,
	char *
#endif
    );

    XtransConnInfo (*OpenCLTSClient)(
#if NeedNestedPrototypes
	struct _Xtransport *,
	char *,
	char *,
	char *
#endif
    );

    XtransConnInfo (*OpenCLTSServer)(
#if NeedNestedPrototypes
	struct _Xtransport *,
	char *,
	char *,
	char *
#endif
    );

    int	(*SetOption)(
#if NeedNestedPrototypes
	XtransConnInfo,
	int,
	int
#endif
    );

    int	(*CreateListener)(
#if NeedNestedPrototypes
	XtransConnInfo,
	char *
#endif
    );

    XtransConnInfo (*Accept)(
#if NeedNestedPrototypes
	XtransConnInfo
#endif
    );

    int	(*Connect)(
#if NeedNestedPrototypes
	XtransConnInfo,
	char *,
	char *
#endif
    );

    int	(*BytesReadable)(
#if NeedNestedPrototypes
	XtransConnInfo,
	BytesReadable_t *
#endif
    );

    int	(*Read)(
#if NeedNestedPrototypes
	XtransConnInfo,
	char *,
	int
#endif
    );

    int	(*Write)(
#if NeedNestedPrototypes
	XtransConnInfo,
	char *,
	int
#endif
    );

    int	(*Readv)(
#if NeedNestedPrototypes
	XtransConnInfo,
	struct iovec *,
	int
#endif
    );

    int	(*Writev)(
#if NeedNestedPrototypes
	XtransConnInfo,
	struct iovec *,
	int
#endif
    );

    int	(*Disconnect)(
#if NeedNestedPrototypes
	XtransConnInfo
#endif
    );

    int	(*Close)(
#if NeedNestedPrototypes
	XtransConnInfo
#endif
    );

    int	(*NameToAddr)(
#if NeedNestedPrototypes
	XtransConnInfo
#endif
    );

    int	(*AddrToName)(
#if NeedNestedPrototypes
	XtransConnInfo
#endif
    );

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

#define READV(ciptr, iov, iovcnt)	TRANS(ReadV)(ciptr, iov, iovcnt)

static	int TRANS(ReadV)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* iov */
    int			/* iovcnt */
#endif
);

#define WRITEV(ciptr, iov, iovcnt)	TRANS(WriteV)(ciptr, iov, iovcnt)

static int TRANS(WriteV)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* iov */
    int 		/* iovcnt */
#endif
);

#else

#define READV(ciptr, iov, iovcnt)	readv(ciptr->fd, iov, iovcnt)
#define WRITEV(ciptr, iov, iovcnt)	writev(ciptr->fd, iov, iovcnt)

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
