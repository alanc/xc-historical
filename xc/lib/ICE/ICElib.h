/* $XConsortium: ICElib.h,v 1.1 93/08/18 00:41:35 mor Exp $ */
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
******************************************************************************/

#ifndef ICELIB_H
#define ICELIB_H

#include <X11/ICE/ICE.h>
#include <X11/Xfuncproto.h>

#define Bool int
#define Status int
#define True 1
#define False 0
#define NULL 0

#if NeedFunctionPrototypes
typedef void *IcePointer;
#else
typedef char *IcePointer;
#endif

typedef enum {
    IceOCLauthHaveReply,
    IceOCLauthRejected,
    IceOCLauthFailed,
    IceOCLauthDoneCleanup
} IceOCLauthStatus;

typedef enum {
    IceACLauthContinue,
    IceACLauthAccepted,
    IceACLauthRejected,
    IceACLauthFailed
} IceACLauthStatus;

typedef enum {
    IceConnectPending,
    IceConnectAccepted,
    IceConnectRejected
} IceConnectStatus;

typedef struct {
    int			opcode_of_request;
    unsigned long	sequence_of_request;
    IcePointer		reply;
} IceReplyWaitInfo;

typedef struct _IceConn *IceConn;

typedef Bool (*IceOCLprocessMsgCB) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* opcode */,
    unsigned long	/* length */,
    Bool		/* swap */,
    IceReplyWaitInfo *  /* replyWait */
#endif
);

typedef void (*IceACLprocessMsgCB) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* opcode */,
    unsigned long	/* length */,
    Bool		/* swap */
#endif
);

typedef struct {
    int			major_version;
    int			minor_version;
    IceOCLprocessMsgCB	process_msg_cb;
} IceOCLversionRec;

typedef struct {
    int			major_version;
    int			minor_version;
    IceACLprocessMsgCB	process_msg_cb;
} IceACLversionRec;

typedef IceOCLauthStatus (*IceOCLauthProc) (
#if NeedFunctionPrototypes
    IcePointer *	/* authStatePtr */,
    Bool		/* cleanUp */,
    int			/* authDataLen */,
    IcePointer		/* authData */,
    int *		/* replyDataLenRet */,
    IcePointer *	/* replyDataRet */,
    char **		/* errorStringRet */
#endif
);

typedef IceACLauthStatus (*IceACLauthProc) (
#if NeedFunctionPrototypes
    IcePointer *	/* authStatePtr */,
    int			/* replyDataLen */,
    IcePointer		/* replyData */,
    int *		/* authDataLenRet */,
    IcePointer *	/* authDataRet */,
    char **		/* errorStringRet */
#endif
);

typedef struct {
    char		*auth_name;
    IceOCLauthProc	auth_proc;
} IceOCLauthRec;

typedef struct {
    char		*auth_name;
    IceACLauthProc	auth_proc;
} IceACLauthRec;

typedef void (*IceProtocolSetupNotifyCB) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* majorVersion */,
    int			/* minorVersion */,
    char *		/* vendor */,
    char *		/* release */
#endif
);

typedef void (*IcePingReplyCB) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */
#endif
);

typedef void (*IceErrorHandler) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* offendingMinorOpcode */,
    unsigned long 	/* offendingSequence */,
    int 		/* errorClass */,
    int			/* severity */,
    IcePointer		/* data */
#endif
);

typedef void (*IceIOErrorHandler) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */
#endif
);



/*
 * Internal stuff.  Put in ICElibint.h???
 */

typedef struct {
    char		*vendor;
    char		*release;
    int			version_count;
    IceOCLversionRec	*version_recs;
    int			auth_count;
    IceOCLauthRec	*auth_recs;
} _IceOCLprotocol;

typedef struct {
    char			*vendor;
    char			*release;
    int				version_count;
    IceACLversionRec		*version_recs;
    IceProtocolSetupNotifyCB	protocol_setup_notify_cb;
    int				auth_count;
    IceACLauthRec		*auth_recs;
} _IceACLprotocol;

typedef struct {
    char		*protocol_name;
    _IceOCLprotocol	*orig_client;
    _IceACLprotocol   	*accept_client;
} _IceProtocol;

typedef struct {
    Bool			in_use;
    _IceProtocol		*protocol;
    Bool			accept_flag;
    union {
	IceACLprocessMsgCB	accept_client;
	IceOCLprocessMsgCB	orig_client;
    } process_msg_cb;
} _IceProcessMsgInfo;

typedef struct {
    int		his_version_index;
    int		my_version_index;
    char	*his_vendor;
    char	*his_release;
    char	my_auth_index;
    IcePointer 	my_auth_state;
} _IceConnectToMeInfo;

typedef struct {
    int		his_opcode;
    int		my_opcode;
    int		his_version_index;
    int		my_version_index;
    char	*his_vendor;
    char	*his_release;
    char	my_auth_index;
    IcePointer 	my_auth_state;
} _IceProtoSetupToMeInfo;

typedef struct {
    Bool 	auth_active;
    char	my_auth_index;
    IcePointer 	my_auth_state;
} _IceConnectToYouInfo;

typedef struct {
    int		my_opcode;
    int		my_auth_count;
    int		*my_auth_indices;
    Bool 	auth_active;
    char	my_auth_index;
    IcePointer	my_auth_state;
} _IceProtoSetupToYouInfo;



/*
 * ICE connection object
 */

struct _IceConn {

    IceConnectStatus connection_status; /* pending, accepted, rejected */

    char waiting_for_byteorder; 	/* waiting for a ByteOrder message? */
    char swap;  		        /* do we need to swap on reads? */
    unsigned char my_ice_version_index; /* which version are we using? */
    unsigned char ref_count;            /* might be used by several libs */

    int fd;				/* Socket descriptor */
    unsigned long sequence;     	/* Sequence number of last message */

    IcePingReplyCB ping_reply_cb;	/* Are we waiting for a Ping Reply? */

    char *connection_string;		/* network connection string */
    char *vendor;			/* other client's vendor */
    char *release;			/* other client's release */

    char *inbuf;			/* Input buffer starting address */
    char *inbufptr;			/* Input buffer index pointer */
    char *inbufmax;			/* Input buffer maximum+1 address */

    char *outbuf;			/* Output buffer starting address */
    char *outbufptr;			/* Output buffer index pointer */
    char *outbufmax;			/* Output buffer maximum+1 address */

    char *scratch;			/* scratch buffer */
    unsigned long scratch_size;		/* scratch size */


    /*
     * Before we read a message, the major opcode of the message must be
     * mapped to our corresponding major opcode (the two clients can use
     * different opcodes for the same protocol).  In order to save space,
     * we keep track of the mininum and maximum major opcodes used by the
     * other client.  To get the information on how to process this message,
     * we do the following...
     *
     * processMsgInfo = iceConn->process_msg_info[
     *     message->majorOpcode - iceConn->his_min_opcode]
     *
     * Note that the number of elements in the iceConn->process_msg_info
     * array is always (iceConn->his_max_opcode - iceConn->his_min_opcode + 1).
     * We check process_msg_info->in_use to see if the opcode is being used.
     */

    _IceProcessMsgInfo		*process_msg_info;
    char 			his_min_opcode;   /* [1..255] */
    char			his_max_opcode;	  /* [1..255] */


    /*
     * Some state for a client doing a Connection/Protocol Setup
     */

    _IceConnectToYouInfo	*connect_to_you;
    _IceProtoSetupToYouInfo	*protosetup_to_you;


    /*
     * Some state for a client receiving a Connection/Protocol Setup
     */

    _IceConnectToMeInfo		*connect_to_me;
    _IceProtoSetupToMeInfo	*protosetup_to_me;

};


/*
 * Function prototypes
 */

extern int
IceRegisterForProtocolSetup (
#if NeedFunctionPrototypes
    char *			/* protocolName */,
    char *			/* vendor */,
    char *			/* release */,
    int				/* versionCount */,
    IceOCLversionRec *		/* versionRecs */,
    int				/* authCount */,
    IceOCLauthRec *		/* authRecs */
#endif
);

extern int
IceRegisterForProtocolReply (
#if NeedFunctionPrototypes
    char *			/* protocolName */,
    char *			/* vendor */,
    char *			/* release */,
    int				/* versionCount */,
    IceACLversionRec *		/* versionRecs */,
    IceProtocolSetupNotifyCB	/* protocolSetupNotifyCB */,
    int				/* authCount */,
    IceACLauthRec *		/* authRecs */
#endif
);

extern IceConn
IceOpenConnection (
#if NeedFunctionPrototypes
    char *		/* networkIdsList */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern void
IceCloseConnection (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern Status
IceWatchConnections (
#if NeedFunctionPrototypes
    int *		/* countRet */,
    int **		/* descripsRet */,
    char **		/* networkIdsRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern IceConn
IceAcceptConnection (
#if NeedFunctionPrototypes
    int			/* fd */
#endif
);

extern void
IceDestroyConnection (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern Status
IceProtocolSetup (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int 		/* myOpcode */,
    int			/* authCount */,
    int	*		/* authIndices */,
    int	*		/* majorVersionRet */,
    int	*		/* minorVersionRet */,
    char **		/* vendorRet */,
    char **		/* releaseRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern Bool
IceProcessMessage (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IceReplyWaitInfo *	/* replyWait */
#endif
);

extern void
IcePing (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */,
   IcePingReplyCB	/* pingReplyCB */
#endif
);

extern IceConnectStatus
IceConnectionStatus (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern char *
IceVendor (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern char *
IceRelease (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern int
IceProtocolVersion (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern int
IceProtocolRevision (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern int
IceConnectionNumber (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern char *
IceConnectionString (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern unsigned long
IceLastSequenceNumber (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern IceErrorHandler
IceSetErrorHandler (
#if NeedFunctionPrototypes
    IceErrorHandler 	/* handler */
#endif
);

extern IceIOErrorHandler
IceSetIOErrorHandler (
#if NeedFunctionPrototypes
    IceIOErrorHandler 	/* handler */
#endif
);

extern char *
IceAllocScratch (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */,
   unsigned long	/* size */
#endif
);

#endif /* ICELIB_H */
