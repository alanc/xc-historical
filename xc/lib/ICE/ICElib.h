/* $XConsortium: ICElib.h,v 1.13 93/09/27 11:44:45 mor Exp $ */
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

typedef struct {
    unsigned long	sequence_of_request;
    int			major_opcode_of_request;
    int			minor_opcode_of_request;
    IcePointer		reply;
} IceReplyWaitInfo;

typedef struct _IceConn *IceConn;

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
    int			/* opcode */,
    unsigned long	/* length */,
    Bool		/* swap */,
    IceReplyWaitInfo *  /* replyWait */
#endif
);

typedef void (*IcePaProcessMsgProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
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
    IcePointer *	/* authStatePtr */,
    Bool		/* swap */,
    int			/* replyDataLen */,
    IcePointer		/* replyData */,
    int *		/* authDataLenRet */,
    IcePointer *	/* authDataRet */,
    char **		/* errorStringRet */
#endif
);

typedef struct {
    char		*auth_name;
    IcePoAuthProc	auth_proc;
} IcePoAuthRec;

typedef struct {
    char		*auth_name;
    IcePaAuthProc	auth_proc;
} IcePaAuthRec;

typedef void (*IceProtocolSetupNotifyProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* majorVersion */,
    int			/* minorVersion */,
    char *		/* vendor */,
    char *		/* release */
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
 * Internal stuff.  Put in ICElibint.h???
 */

typedef struct _IceSavedReplyWait {
    IceReplyWaitInfo		*reply_wait;
    Bool			reply_ready;
    struct _IceSavedReplyWait	*next;
} _IceSavedReplyWait;

typedef struct _IcePingWait {
    IcePingReplyProc		ping_reply_proc;
    IcePointer			client_data;
    struct _IcePingWait 	*next;
} _IcePingWait;

typedef struct {
    char		*vendor;
    char		*release;
    int			version_count;
    IcePoVersionRec	*version_recs;
    int			auth_count;
    IcePoAuthRec	*auth_recs;
    IceIOErrorProc	io_error_proc;
} _IcePoProtocol;

typedef struct {
    char			*vendor;
    char			*release;
    int				version_count;
    IcePaVersionRec		*version_recs;
    IceProtocolSetupNotifyProc	protocol_setup_notify_proc;
    int				auth_count;
    IcePaAuthRec		*auth_recs;
    IceIOErrorProc		io_error_proc;
} _IcePaProtocol;

typedef struct {
    char		*protocol_name;
    _IcePoProtocol	*orig_client;
    _IcePaProtocol   	*accept_client;
} _IceProtocol;

typedef struct {
    Bool			in_use;
    int				my_opcode;
    _IceProtocol		*protocol;
    Bool			accept_flag;
    union {
	IcePaProcessMsgProc	accept_client;
	IcePoProcessMsgProc	orig_client;
    } process_msg_proc;
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

    int fd;				/* Socket descriptor */
    unsigned long sequence;     	/* Sequence number of last message */

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
     * Number of times this iceConn was returned in IceOpenConnection
     * or IceAcceptConnection.
     */

    unsigned char		open_ref_count;


    /*
     * Number of active protocols.
     */

    unsigned char		proto_ref_count;


    /*
     * iceConn_type = 1 if this iceConn was created using IceOpenConnection.
     * iceConn_type = 2 if this iceConn was created using IceAcceptConnection.
     */

    char			iceConn_type;


    /*
     * If we know the connection was terminated by the other side, we
     * want to skip sending a WantToClose in IceCloseConnection.
     */

    char			skip_want_to_close;


    /*
     * Did we send a WantToClose message?  This will get cancelled if we
     * receive a NoClose or a ProtocolSetup.  If this is the case, the
     * other side will be responsible for sending a WantToClose.
     */

    char			want_to_close;


    /*
     * We need to keep track of all the replies we're waiting for.
     * Check the comments in process.c for how this works.
     */

    _IceSavedReplyWait		*saved_reply_waits;


    /*
     * We keep track of all Pings sent from the client.  When the Ping reply
     * arrives, we remove it from the list.
     */

    _IcePingWait		*ping_waits;


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

extern int IceRegisterForProtocolSetup (
#if NeedFunctionPrototypes
    char *			/* protocolName */,
    char *			/* vendor */,
    char *			/* release */,
    int				/* versionCount */,
    IcePoVersionRec *		/* versionRecs */,
    int				/* authCount */,
    IcePoAuthRec *		/* authRecs */,
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
    IceProtocolSetupNotifyProc	/* protocolSetupNotifyProc */,
    int				/* authCount */,
    IcePaAuthRec *		/* authRecs */,
    IceIOErrorProc		/* IOErrorProc */
#endif
);

extern IceConn IceOpenConnection (
#if NeedFunctionPrototypes
    char *		/* networkIdsList */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern Status IceListenForConnections (
#if NeedFunctionPrototypes
    int *		/* countRet */,
    int **		/* descripsRet */,
    char **		/* networkIdsRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern IceConn IceAcceptConnection (
#if NeedFunctionPrototypes
    int			/* fd */
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

extern Status IceProtocolShutdown (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */
#endif
);

extern Bool IceProcessMessage (
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

extern unsigned long IceLastSequenceNumber (
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

extern char *IceAllocScratch (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */,
   unsigned long	/* size */
#endif
);

#endif /* ICELIB_H */

