/* $XConsortium: Xtrans.h,v 1.13 94/02/06 16:03:21 mor Exp $ */

/* Copyright (c) 1993, 1994 NCR Corporation - Dayton, Ohio, USA
 * Copyright 1993, 1994 by the Massachusetts Institute of Technology
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name NCR or M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCR and M.I.T. make no representations about the
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

#ifndef _XTRANS_H
#define _XTRANS_H

#include <X11/Xfuncproto.h>
#include <X11/Xos.h>


/*
 * Set the functions names according to where this code is being compiled.
 */

#ifdef X11_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _X11Trans##func
#else
#define TRANS(func) _X11Trans/**/func
#endif
#endif /* X11_t */

#ifdef XSERV_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XSERVTrans##func
#else
#define TRANS(func) _XSERVTrans/**/func
#endif
#define X11_t
#endif /* X11_t */

#ifdef XIM_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XIMTrans##func
#else
#define TRANS(func) _XIMTrans/**/func
#endif
#endif /* XIM_t */

#ifdef FS_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FSTrans##func
#else
#define TRANS(func) _FSTrans/**/func
#endif
#endif /* FS_t */

#ifdef FONT_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FONTTrans##func
#else
#define TRANS(func) _FONTTrans/**/func
#endif
#endif /* FONT_t */

#ifdef ICE_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _ICETrans##func
#else
#define TRANS(func) _ICETrans/**/func
#endif
#endif /* ICE_t */

#ifdef TEST_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _TESTTrans##func
#else
#define TRANS(func) _TESTTrans/**/func
#endif
#endif /* TEST_t */

#if !defined(TRANS)
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XTrans##func
#else
#define TRANS(func) _XTrans/**/func
#endif
#endif /* !TRANS */


/*
 * Create a single address structure that can be used wherever
 * an address structure is needed. struct sockaddr is not big enough
 * to hold a sockadd_un, so we create this definition to have a single
 * structure that is big enough for all the structures we might need.
 *
 * This structure needs to be independent of the socket/TLI interface used.
 */

#define XTRANS_MAX_ADDR_LEN	128	/* large enough to hold sun_path */

typedef	struct {
    unsigned char	addr[XTRANS_MAX_ADDR_LEN];
} Xtransaddr;


#ifdef LONG64
typedef int BytesReadable_t;
#else
typedef long BytesReadable_t;
#endif


#if defined(WIN32) || (defined(USG) && !defined(CRAY) && !defined(umips) && !defined(MOTOROLA) && !defined(uniosu) && !defined(__sxg__))

/*
 *      TRANS(Readv) and TRANS(Writev) use struct iovec, normally found
 *      in Berkeley systems in <sys/uio.h>.  See the readv(2) and writev(2)
 *      manual pages for details.
 */

struct iovec {
    caddr_t iov_base;
    int iov_len;
};

#else
#include <sys/uio.h>
#endif

typedef struct _XtransConnInfo *XtransConnInfo;


/*
 * Transport Option definitions
 */

#define TRANS_NONBLOCKING	1
#define	TRANS_CLOSEONEXEC	2


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
 * ResetListener return values
 */

#define TRANS_RESET_NOOP	1
#define TRANS_RESET_NEW_FD	2
#define TRANS_RESET_FAILURE	3


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

int TRANS(ResetListener)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
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

int TRANS(IsLocal)(
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
    int *,		/* partial */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
#endif
);

int TRANS(MakeAllCLTSServerListeners)(
#if NeedFunctionPrototypes
    char *,		/* port */
    int *,		/* partial */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
#endif
);


/*
 * Function Prototypes for Utility Functions.
 */

#ifdef X11_t

int TRANS(ConvertAddress)(
#if NeedFunctionPrototypes
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr *	/* addrp */
#endif
);

#endif /* X11_t */

#ifdef ICE_t

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

#endif /* ICE_t */

#endif /* _XTRANS_H */
