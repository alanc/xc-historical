/* $XConsortium: ICElib.h,v 1.31 94/03/17 15:36:28 mor Exp $ */
/******************************************************************************

Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.

Author: Ralph Mor, X Consortium
******************************************************************************/

#ifndef ICELIB_H
#define ICELIB_H

#include <X11/ICE/ICE.h>
#include <X11/Xfuncproto.h>

#define Bool int
#define Status int
#define True 1
#define False 0

#if NeedFunctionPrototypes
typedef void *IcePointer;
#else
typedef char *IcePointer;
#endif

typedef enum {
    IcePoAuthHaveReply,
    IcePoAuthRejected,
    IcePoAuthFailed,
    IcePoAuthDoneCleanup
} IcePoAuthStatus;

typedef enum {
    IcePaAuthContinue,
    IcePaAuthAccepted,
    IcePaAuthRejected,
    IcePaAuthFailed
} IcePaAuthStatus;

typedef enum {
    IceConnectPending,
    IceConnectAccepted,
    IceConnectRejected
} IceConnectStatus;

typedef enum {
    IceProtocolSetupSuccess,
    IceProtocolSetupFailure,
    IceProtocolAlreadyActive
} IceProtocolSetupStatus;

typedef enum {
    IceAcceptSuccess,
    IceAcceptFailure,
    IceAcceptBadMalloc
} IceAcceptStatus;

typedef struct {
    unsigned long	sequence_of_request;
    int			major_opcode_of_request;
    int			minor_opcode_of_request;
    IcePointer		reply;
} IceReplyWaitInfo;

typedef struct _IceConn *IceConn;
typedef struct _IceListenObj *IceListenObj;

typedef void (*IceWatchProc) (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IcePointer		/* clientData */,
    Bool		/* opening */,
    IcePointer *	/* watchData */
#endif
);

typedef Bool (*IcePoProcessMsgProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    IcePointer		/* clientData */,
    int			/* opcode */,
    unsigned long	/* length */,
    Bool		/* swap */,
    IceReplyWaitInfo *  /* replyWait */
#endif
);

typedef void (*IcePaProcessMsgProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    IcePointer		/* clientData */,
    int			/* opcode */,
    unsigned long	/* length */,
    Bool		/* swap */
#endif
);

typedef struct {
    int			 major_version;
    int			 minor_version;
    IcePoProcessMsgProc  process_msg_proc;
} IcePoVersionRec;

typedef struct {
    int			 major_version;
    int			 minor_version;
    IcePaProcessMsgProc  process_msg_proc;
} IcePaVersionRec;

typedef IcePoAuthStatus (*IcePoAuthProc) (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IcePointer *	/* authStatePtr */,
    Bool		/* cleanUp */,
    Bool		/* swap */,
    int			/* authDataLen */,
    IcePointer		/* authData */,
    int *		/* replyDataLenRet */,
    IcePointer *	/* replyDataRet */,
    char **		/* errorStringRet */
#endif
);

typedef IcePaAuthStatus (*IcePaAuthProc) (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IcePointer *	/* authStatePtr */,
    Bool		/* swap */,
    int			/* authDataLen */,
    IcePointer		/* authData */,
    int *		/* replyDataLenRet */,
    IcePointer *	/* replyDataRet */,
    char **		/* errorStringRet */
#endif
);

typedef Bool (*IceHostBasedAuthProc) (
#if NeedFunctionPrototypes
    char *		/* hostName */
#endif
);

typedef void (*IceProtocolSetupNotifyProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* majorVersion */,
    int			/* minorVersion */,
    char *		/* vendor */,
    char *		/* release */,
    IcePointer *	/* clientDataRet */
#endif
);

typedef void (*IceIOErrorProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */
#endif
);

typedef void (*IcePingReplyProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    IcePointer		/* clientData */
#endif
);

typedef void (*IceErrorHandler) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    Bool		/* swap */,
    int			/* offendingMinorOpcode */,
    unsigned long 	/* offendingSequence */,
    int 		/* errorClass */,
    int			/* severity */,
    IcePointer		/* values */
#endif
);

typedef void (*IceIOErrorHandler) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */
#endif
);


/*
 * Function prototypes
 */

extern int IceRegisterForProtocolSetup (
#if NeedFunctionPrototypes
    char *			/* protocolName */,
    char *			/* vendor */,
    char *			/* release */,
    int				/* versionCount */,
    IcePoVersionRec *		/* versionRecs */,
    int				/* authCount */,
    char **			/* authNames */,
    IcePoAuthProc *		/* authProcs */,
    IceIOErrorProc		/* IOErrorProc */
#endif
);

extern int IceRegisterForProtocolReply (
#if NeedFunctionPrototypes
    char *			/* protocolName */,
    char *			/* vendor */,
    char *			/* release */,
    int				/* versionCount */,
    IcePaVersionRec *		/* versionRecs */,
    int				/* authCount */,
    char **			/* authNames */,
    IcePaAuthProc *		/* authProcs */,
    IceHostBasedAuthProc	/* hostBasedAuthProc */,
    IceProtocolSetupNotifyProc	/* protocolSetupNotifyProc */,
    IceIOErrorProc		/* IOErrorProc */
#endif
);

extern IceConn IceOpenConnection (
#if NeedFunctionPrototypes
    char *		/* networkIdsList */,
    Bool		/* mustAuthenticate */,
    int			/* majorOpcodeCheck */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern Status IceListenForConnections (
#if NeedFunctionPrototypes
    int *		/* countRet */,
    IceListenObj **	/* listenObjsRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern int IceGetListenConnectionNumber (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */
#endif
);

extern char *IceGetListenConnectionString (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */
#endif
);

extern char *IceComposeNetworkIdList (
#if NeedFunctionPrototypes
    int			/* count */,
    IceListenObj *	/* listenObjs */
#endif
);

extern void IceFreeListenObjs (
#if NeedFunctionPrototypes
    int			/* count */,
    IceListenObj *	/* listenObjs */
#endif
);

extern void IceSetHostBasedAuthProc (
#if NeedFunctionPrototypes
    IceListenObj		/* listenObj */,
    IceHostBasedAuthProc   	/* hostBasedAuthProc */
#endif
);

extern IceConn IceAcceptConnection (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */,
    IceAcceptStatus *	/* statusRet */
#endif
);

extern void IceSetShutdownNegotiation (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    Bool		/* negotiate */
#endif
);

extern Bool IceCheckShutdownNegotiation (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern Status IceCloseConnection (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern Status IceAddConnectionWatch (
#if NeedFunctionPrototypes
    IceWatchProc		/* watchProc */,
    IcePointer			/* clientData */
#endif
);

extern void IceRemoveConnectionWatch (
#if NeedFunctionPrototypes
    IceWatchProc		/* watchProc */,
    IcePointer			/* clientData */
#endif
);

extern IceProtocolSetupStatus IceProtocolSetup (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int 		/* myOpcode */,
    IcePointer		/* clientData */,
    Bool		/* mustAuthenticate */,
    int	*		/* majorVersionRet */,
    int	*		/* minorVersionRet */,
    char **		/* vendorRet */,
    char **		/* releaseRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern Status IceProtocolShutdown (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */
#endif
);

extern Bool IceProcessMessages (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IceReplyWaitInfo *	/* replyWait */
#endif
);

extern void IcePing (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */,
   IcePingReplyProc	/* pingReplyProc */,
   IcePointer		/* clientData */
#endif
);

extern char *IceAllocScratch (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */,
   unsigned long	/* size */
#endif
);

extern IceFlush (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */
#endif
);

extern int IceGetOutBufSize (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */
#endif
);

extern int IceGetInBufSize (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */
#endif
);

extern IceConnectStatus IceConnectionStatus (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern char *IceVendor (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern char *IceRelease (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern int IceProtocolVersion (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern int IceProtocolRevision (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern int IceConnectionNumber (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern char *IceConnectionString (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern unsigned long IceLastSentSequenceNumber (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern unsigned long IceLastReceivedSequenceNumber (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern Bool IceSwapping (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern IceErrorHandler IceSetErrorHandler (
#if NeedFunctionPrototypes
    IceErrorHandler 	/* handler */
#endif
);

extern IceIOErrorHandler IceSetIOErrorHandler (
#if NeedFunctionPrototypes
    IceIOErrorHandler 	/* handler */
#endif
);


/*
 * Multithread Routines
 */

extern Status IceInitThreads (
#if NeedFunctionPrototypes
    void
#endif
);

extern void IceAppLockConn (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern void IceAppUnlockConn (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#endif /* ICELIB_H */
