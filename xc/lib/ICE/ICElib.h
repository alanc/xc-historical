/* $XConsortium: ICElib.h,v 1.23 93/12/28 11:44:14 mor Exp $ */
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
#include <stdio.h>

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
    char *		/* address */,
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
    char *		/* address */,
    Bool		/* swap */,
    int			/* replyDataLen */,
    IcePointer		/* replyData */,
    int *		/* authDataLenRet */,
    IcePointer *	/* authDataRet */,
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
 * An entry in the ICE authority file.
 */

typedef struct {
    char    	    *protocol_name;
    unsigned short  protocol_data_length;
    char   	    *protocol_data;
    char    	    *address;
    char    	    *auth_name;
    unsigned short  auth_data_length;
    char   	    *auth_data;
} IceAuthFileEntry;


/*
 * Authentication data maintained in memory.
 */

typedef struct {
    char    	    *protocol_name;
    char	    *address;
    char    	    *auth_name;
    unsigned short  auth_data_length;
    char   	    *auth_data;
} IceAuthDataEntry;


/*
 * Return values from IceLockAuthFile
 */

#define IceAuthLockSuccess	0   /* lock succeeded */
#define IceAuthLockError	1   /* lock unexpectely failed, check errno */
#define IceAuthLockTimeout	2   /* lock failed, timeouts expired */


/*
 * Maxium number of ICE authentication methods allowed, and maxiumum
 * number of authentication data entries allowed in an .ICEauthority
 * file or in ICElib implementation.  This makes it easier to declare
 * local arrays.
 */

#define MAX_ICE_AUTH_NAMES 32
#define ICE_MAX_AUTH_DATA_ENTRIES 100


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
    char		**auth_names;
    IcePoAuthProc	*auth_procs;
    IceIOErrorProc	io_error_proc;
} _IcePoProtocol;

typedef struct {
    char			*vendor;
    char			*release;
    int				version_count;
    IcePaVersionRec		*version_recs;
    IceProtocolSetupNotifyProc	protocol_setup_notify_proc;
    int				auth_count;
    char			**auth_names;
    IcePaAuthProc		*auth_procs;
    IceHostBasedAuthProc	host_based_auth_proc;
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
    Bool	must_authenticate;
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
    Bool	must_authenticate;
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
 * ICE listen object
 */

struct _IceListenObj {
    int				fd;
    Bool			local_conn;
    char			*network_id;
    IceHostBasedAuthProc 	host_based_auth_proc;
};

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
     * If this ICE connection was created with IceAcceptConnection,
     * the listen_obj field is set to the listen object.  Otherwise,
     * the listen_obj field is NULL.
     */

    IceListenObj		listen_obj;


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

extern int IceGetListenDescrip (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */
#endif
);

extern char *IceGetListenNetworkId (
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

extern void IceFreeListenObj (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */
#endif
);

extern void IceSetHostBasedAuthProc (
#if NeedFunctionPrototypes
    IceListenObj		/* listenObj */,
    IceHostBasedAuthProc   	/* hostBasedAuthProc */
#endif
);

extern void IceSetPaAuthData (
#if NeedFunctionPrototypes
    int			/* numEntries */,
    IceAuthDataEntry *	/* entries */
#endif
);

extern void IceGetPoAuthData (
#if NeedFunctionPrototypes
    char *		/* protocol_name */,
    char *		/* address */,
    char *		/* auth_name */,
    unsigned short *	/* auth_data_length_ret */,
    char **		/* auth_data_ret */
#endif
);

extern void IceGetPaAuthData (
#if NeedFunctionPrototypes
    char *		/* protocol_name */,
    char *		/* address */,
    char *		/* auth_name */,
    unsigned short *	/* auth_data_length_ret */,
    char **		/* auth_data_ret */
#endif
);

extern IceConn IceAcceptConnection (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */
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

extern char *IceAllocScratch (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */,
   unsigned long	/* size */
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


/*
 * Multithread Routines
 */

extern Status IceInitThreads (
#if NeedFunctionPrototypes
    void
#endif
);

extern void IceLockConn (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

extern void IceUnlockConn (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);


/*
 * ICE Authority File Utils
 */

extern char *IceAuthFileName (
#if NeedFunctionPrototypes
    void
#endif
);

extern int IceLockAuthFile (
#if NeedFunctionPrototypes
    char *		/* file_name */,
    int			/* retries */,
    int			/* timeout */,
    long		/* dead */
#endif
);

extern void IceUnlockAuthFile (
#if NeedFunctionPrototypes
    char *		/* file_name */
#endif
);

extern IceAuthFileEntry *IceReadAuthFileEntry (
#if NeedFunctionPrototypes
    FILE *		/* auth_file */
#endif
);

extern void IceFreeAuthFileEntry (
#if NeedFunctionPrototypes
    IceAuthFileEntry *	/* auth */
#endif
);

extern Status IceWriteAuthFileEntry (
#if NeedFunctionPrototypes
    FILE *		/* auth_file */,
    IceAuthFileEntry *	/* auth */
#endif
);

extern IceAuthFileEntry *IceGetAuthFileEntry (
#if NeedFunctionPrototypes
    char *		/* protocol_name */,
    char *		/* address */,
    char *		/* auth_name */
#endif
);

#endif /* ICELIB_H */
