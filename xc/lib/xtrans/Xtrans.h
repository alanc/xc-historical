/*      Copyright (c) 1993 NCR Corporation - Dayton, Ohio, USA */
/*        All Rights Reserved   */

#ifndef _XTRANS_H
#define _XTRANS_H
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
 * NCR DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NCR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <X11/Xfuncproto.h>
#ifdef WIN32
#define _WILLWINSOCK_
#endif
#include <X11/Xos.h>
/*
 * Set the functions names according to where this code is being compiled.
 */
#ifdef X11
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _X11Trans##func
#else
#define TRANS(func) _X11Trans/**/func
#endif
#endif /* X11 */

#ifdef FS
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FSTrans##func
#else
#define TRANS(func) _FSTrans/**/func
#endif
#endif /* FS */

#ifdef ICE
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _ICETrans##func
#else
#define TRANS(func) _ICETrans/**/func
#endif
#endif /* ICE */

#ifdef TEST
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _TESTTrans##func
#else
#define TRANS(func) _TESTTrans/**/func
#endif
#endif /* TEST */

#if !defined(TRANS)
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XTrans##func
#else
#define TRANS(func) _XTrans/**/func
#endif
#endif /* !TRANS */

/*
 * Some useful type definitions
 */

#ifdef LONG64
typedef int BytesReadable_t;
#else
typedef long BytesReadable_t;
#endif

/*
 * Some useful #defines
 */
#define X_TCP_PORT	6000

/*
 * Create a single address structure that can be used wherever
 * an address structure is needed. struct sockaddr is not big enough
 * to hold a sockadd_un, so we create this definition to have a single
 * structure that is big enough for all the structures we might need.
 */
/*
 * This structure needs to be independent of the socket/TLI interface used.
 */

#define XTRANS_MAX_ADDR_LEN	128	/* large enough to hold sun_path */

typedef	union {
	unsigned short	family;
	unsigned char	addr[XTRANS_MAX_ADDR_LEN];
	}	Xtransaddr;

/*
 * Transport Option definitions
 */
#define TRANS_NONBLOCKING	1
#define	TRANS_CLOSEONEXEC	2

/*
 * Some WIN32 stuff
 */
#ifdef WIN32
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef EINTR
#define EINTR WSAEINTR

#define NEEDIOVEC

#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#include <X11/Xw32defs.h>
#endif /* WIN32 */

#if defined(USG) && !defined(CRAY) && !defined(umips) && !defined(MOTOROLA) && !defined(uniosu) 
#define NEEDIOVEC
#endif /* USG && !CRAY && !umips && !MOTOROLA && !uniosu */

#ifdef NEEDIOVEC
/*
 *      TRANS(Readv) and TRANS(Writev) use struct iovec, normally found
 *      in Berkeley systems in <sys/uio.h>.  See the readv(2) and writev(2)
 *      manual pages for details.
 */

struct iovec {
    caddr_t iov_base;
    int iov_len;
};

#undef NEEDIOVEC

#else
#include <sys/uio.h>
#endif /* NEEDIOVEC */

#ifndef NULL
#define NULL 0
#endif

typedef struct _XtransConnInfo *XtransConnInfo;


/*
 * Return values of Connect (0 is success)
 */

#ifndef TRANS_CONNECT_FAILED
#define TRANS_CONNECT_FAILED -1
#endif

#ifndef TRANS_TRY_CONNECT_AGAIN
#define TRANS_TRY_CONNECT_AGAIN -2
#endif


/*
 * Function prototypes for the exposed interface
 */
XtransConnInfo TRANS(OpenCOTSClient)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);
XtransConnInfo TRANS(OpenCOTSServer)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);
XtransConnInfo TRANS(OpenCLTSClient)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);
XtransConnInfo TRANS(OpenCLTSServer)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);
int TRANS(SetOption)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    int,		/* option */
    int			/* arg */
#endif
);
int TRANS(CreateListener)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    char *		/* port */
#endif
);
XtransConnInfo TRANS(Accept)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);
int TRANS(Connect)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    char *		/* address */
#endif
);
int TRANS(BytesReadable)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    BytesReadable_t *	/* pend */
#endif
);
int TRANS(Read)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    char *,		/* buf */
    int			/* size */
#endif
);
int TRANS(Write)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    char *,		/* buf */
    int			/* size */
#endif
);
int TRANS(Readv)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* buf */
    int			/* size */
#endif
);
int TRANS(Writev)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* buf */
    int			/* size */
#endif
);
int TRANS(Disconnect)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);
int TRANS(Close)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);
int TRANS(NameToAddr)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);
int TRANS(AddrToName)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);
int TRANS(GetMyAddr)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
#endif
);
int TRANS(GetPeerAddr)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
#endif
);
int TRANS(GetConnectionNumber)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);
int TRANS(MakeAllCOTSServerListeners)(
#if NeedFunctionPrototypes
    char *,		/* port */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
#endif
);
int TRANS(MakeAllCLTSServerListeners)(
#if NeedFunctionPrototypes
    char *,		/* port */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
#endif
);

/*
 * Function Prototypes for Utility Functions.
 */

#ifdef X11

int TRANS(ConvertAddress)(
#if NeedFunctionPrototypes
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr *	/* addrp */
#endif
);

#endif /* X11 */

#ifdef ICE

char *
TRANS(GetMyNetworkId)(
#if NeedFunctionPrototypes
    int,		/* family */
    int,		/* addrlen */
    Xtransaddr *	/* addr */
#endif
);

char *
TRANS(GetPeerNetworkId)(
#if NeedFunctionPrototypes
    int,		/* family */
    int,		/* addrlen */
    Xtransaddr *	/* addr */
#endif
);

#endif /* ICE */

#endif /* _XTRANS_H */
