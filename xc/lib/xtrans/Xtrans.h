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

#ifdef LONG64
typedef unsigned int FdMask;
#else
typedef unsigned long FdMask;
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
#define	TRANS_COALESCENCE	3

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


/*
 * Function prototypes for the exposed interface
 */
int TRANS(OpenCOTSClient)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);
int TRANS(OpenCOTSServer)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);
int TRANS(OpenCLTSClient)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);
int TRANS(OpenCLTSServer)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);
int TRANS(CreateListener)(
#if NeedFunctionPrototypes
    int,		/* fd */
    char *		/* port */
#endif
);
int TRANS(Listen)(
#if NeedFunctionPrototypes
    int			/* fd */
#endif
);
int TRANS(Accept)(
#if NeedFunctionPrototypes
    int			/* fd */
#endif
);
int TRANS(Connect)(
#if NeedFunctionPrototypes
    int,		/* fd */
    char *		/* address */
#endif
);
int TRANS(BytesReadable)(
#if NeedFunctionPrototypes
    int,		/* fd */
    BytesReadable_t *	/* pend */
#endif
);
void TRANS(SetOption)(
#if NeedFunctionPrototypes
    int,		/* fd */
    int,		/* option */
    int			/* arg */
#endif
);
int TRANS(Read)(
#if NeedFunctionPrototypes
    int,		/* fd */
    char *,		/* buf */
    int			/* size */
#endif
);
int TRANS(Write)(
#if NeedFunctionPrototypes
    int,		/* fd */
    char *,		/* buf */
    int			/* size */
#endif
);
int TRANS(Readv)(
#if NeedFunctionPrototypes
    int,		/* fd */
    struct iovec *,	/* buf */
    int			/* size */
#endif
);
int TRANS(Writev)(
#if NeedFunctionPrototypes
    int,		/* fd */
    struct iovec *,	/* buf */
    int			/* size */
#endif
);
int TRANS(Disconnect)(
#if NeedFunctionPrototypes
    int			/* fd */
#endif
);
int TRANS(Close)(
#if NeedFunctionPrototypes
    int			/* fd */
#endif
);
int TRANS(NameToAddr)(
#if NeedFunctionPrototypes
    int			/* fd */
#endif
);
int TRANS(AddrToName)(
#if NeedFunctionPrototypes
    int			/* fd */
#endif
);
void TRANS(GetMyAddr)(
#if NeedFunctionPrototypes
    int,		/* fd */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
#endif
);
void TRANS(GetPeerAddr)(
#if NeedFunctionPrototypes
    int,		/* fd */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
#endif
);
void TRANS(MakeAllCOTSServerListeners)(
#if NeedFunctionPrototypes
    char *,		/* port */
    FdMask *		/* fds */
#endif
);
void TRANS(MakeAllCLTSServerListeners)(
#if NeedFunctionPrototypes
    char *,		/* port */
    FdMask *		/* fds */
#endif
);

/*
 * Function Prototypes for Utility Functions.
 */

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

#endif

int TRANS(ConvertAddress)(
#if NeedFunctionPrototypes
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr *	/* addrp */
#endif
);

#ifdef X11
void _X11TransCreateWellKnowListeners(
#if NeedFunctionPrototypes
    char *,		/* port */
    FdMask *		/* fds */
#endif
);
#endif /* X11 */

#endif /* _XTRANS_H */
