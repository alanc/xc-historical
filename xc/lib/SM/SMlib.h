/* $XConsortium: SMlib.h,v 1.11 93/12/06 20:04:08 mor Exp $ */
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

#ifndef SMLIB_H
#define SMLIB_H

#include <X11/SM/SM.h>
#include <X11/ICE/ICElib.h>


/*
 * Generic SM pointer
 */

typedef IcePointer SmPointer;


/*
 * Connection objects.  Defined later on.
 */

typedef struct _SmcConn *SmcConn;
typedef struct _SmsConn *SmsConn;


/*
 * Session Management property
 */

typedef struct {
    int		length;		/* length (in bytes) of the value */
    SmPointer   value;		/* the value */
} SmPropValue;

typedef struct {
    char	*name;		/* name of property */
    char	*type;		/* type of property */
    int		num_vals;	/* number of values in property */
    SmPropValue *vals;		/* the values */
} SmProp;



/*
 * Client callbacks
 */

typedef void (*SmcSaveYourselfProc) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */,
    int  		/* saveType */,
    Bool		/* shutdown */,
    int			/* interactStyle */,
    Bool		/* fast */
#endif
);

typedef void (*SmcInteractProc) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */
#endif
);

typedef void (*SmcDieProc) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */
#endif
);

typedef void (*SmcShutdownCancelledProc) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */
#endif
);

typedef void (*SmcPropReplyProc) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */,
    int			/* numProps */,
    SmProp **		/* props */
#endif
);


/*
 * Callbacks set up at SmcOpenConnection time
 */

typedef struct {

    struct {
	SmcSaveYourselfProc	 callback;
	SmPointer		 client_data;
    } save_yourself;

    struct {
	SmcDieProc		 callback;
	SmPointer		 client_data;
    } die;

    struct {
	SmcShutdownCancelledProc callback;
	SmPointer		 client_data;
    } shutdown_cancelled;

} SmcCallbacks;

#define SmcSaveYourselfProcMask		1
#define SmcDieProcMask			2
#define SmcShutdownCancelledProcMask	4
#define SmcAllProcMask			(1+2+4)


/*
 * Waiting for Interact
 */

typedef struct _SmcInteractWait {
    SmcInteractProc		interact_proc;
    SmPointer			client_data;
    struct _SmcInteractWait 	*next;
} _SmcInteractWait;


/*
 * Waiting for Properties Reply
 */

typedef struct _SmcPropReplyWait {
    SmcPropReplyProc		prop_reply_proc;
    SmPointer			client_data;
    struct _SmcPropReplyWait 	*next;
} _SmcPropReplyWait;



/*
 * Client connection object
 */

struct _SmcConn {

    /*
     * We use ICE to esablish a connection with the SM.
     */

    IceConn		iceConn;


    /*
     * Major and minor versions of the XSMP.
     */

    int			proto_major_version;
    int			proto_minor_version;


    /*
     * The session manager vendor and release number.
     */

    char		*vendor;
    char		*release;


    /*
     * The Client Id uniquely identifies this client to the session manager.
     */

    char		*client_id;


    /*
     * Callbacks to be invoked when messages arrive from the session manager.
     * These callbacks are specified at SmcOpenConnection time.
     */

    SmcCallbacks	callbacks;


    /*
     * We keep track of all Interact Requests sent by the client.  When the
     * Interact message arrives, we remove it from the list (a FIFO list
     * is maintained).
     */

    _SmcInteractWait	*interact_waits;


    /*
     * We keep track of all Get Properties sent by the client.  When the
     * Properties Reply arrives, we remove it from the list (a FIFO list
     * is maintained).
     */

    _SmcPropReplyWait	*prop_reply_waits;


    /*
     * Did we receive a SaveYourself with Shutdown = True?
     */

    Bool		shutdown_in_progress;
};



/*
 * Session manager callbacks
 */

typedef Status (*SmsRegisterClientProc) (
#if NeedFunctionPrototypes
    SmsConn 		/* smsConn */,
    SmPointer		/* managerData */,
    char *		/* previousId */				     
#endif
);

typedef void (*SmsInteractRequestProc) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    int			/* dialogType */
#endif
);

typedef void (*SmsInteractDoneProc) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    Bool		/* cancelShutdown */
#endif
);

typedef void (*SmsSaveYourselfDoneProc) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    Bool		/* success */
#endif
);

typedef void (*SmsCloseConnectionProc) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    char *		/* locale */,
    int			/* count */,
    char **		/* reasonMsgs */
#endif
);

typedef void (*SmsSetPropertiesProc) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    unsigned long 	/* sequenceRef */,
    int			/* numProps */,
    SmProp **		/* props */
#endif
);

typedef void (*SmsGetPropertiesProc) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */
#endif
);


/*
 * Callbacks set up by a session manager when a new client connects.
 */

typedef struct {

    struct {
	SmsRegisterClientProc	callback;
	SmPointer		manager_data;
    } register_client;

    struct {
	SmsInteractRequestProc	callback;
	SmPointer		manager_data;
    } interact_request;

    struct {
	SmsInteractDoneProc	callback;
	SmPointer		manager_data;
    } interact_done;

    struct {
	SmsSaveYourselfDoneProc	callback;
	SmPointer		manager_data;
    } save_yourself_done;

    struct {
	SmsCloseConnectionProc	callback;
	SmPointer		manager_data;
    } close_connection;

    struct {
	SmsSetPropertiesProc	callback;
	SmPointer		manager_data;
    } set_properties;

    struct {
	SmsGetPropertiesProc	callback;
	SmPointer		manager_data;
    } get_properties;

} SmsCallbacks;


typedef void (*SmsNewClientProc) (
#if NeedFunctionPrototypes
    SmsConn 		/* smsConn */,
    SmPointer		/* managerData */,
    SmsCallbacks *	/* callbacksRet */
#endif
);


/*
 * Session manager connection object
 */

struct _SmsConn {

    /*
     * We use ICE to esablish a connection with the client.
     */

    IceConn		iceConn;


    /*
     * Major and minor versions of the XSMP.
     */

    int			proto_major_version;
    int			proto_minor_version;


    /*
     * The vendor and release number of the SMlib used by the client.
     */

    char		*vendor;
    char		*release;


    /*
     * The Client Id uniquely identifies this client to the session manager.
     */

    char		*client_id;


    /*
     * Callbacks to be invoked when messages arrive from the client.
     */

    SmsCallbacks	callbacks;


    /*
     * Some state.
     */

    char		save_yourself_in_progress;
    char		interaction_allowed;
    char		can_cancel_shutdown;
    char		interact_in_progress;
};



/*
 * Error handlers
 */

typedef void (*SmcErrorHandler) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    Bool		/* swap */,
    int			/* offendingMinorOpcode */,
    unsigned long 	/* offendingSequence */,
    int 		/* errorClass */,
    int			/* severity */,
    SmPointer		/* values */
#endif
);

typedef void (*SmsErrorHandler) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    Bool		/* swap */,
    int			/* offendingMinorOpcode */,
    unsigned long 	/* offendingSequence */,
    int 		/* errorClass */,
    int			/* severity */,
    SmPointer		/* values */
#endif
);



/*
 * Function Prototypes
 */

extern SmcConn SmcOpenConnection (
#if NeedFunctionPrototypes
    char *		/* networkIdsList */,
    SmcCallbacks *	/* callbacks */,
    char *		/* previousId */,
    char **		/* clientIdRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern void SmcCloseConnection (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    int			/* count */,
    char **		/* reasonMsgs */
#endif
);

extern void SmcModifyCallbacks (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    unsigned long	/* mask */,
    SmcCallbacks *	/* callbacks */
#endif
);

extern void SmcSetProperties (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    unsigned long 	/* sequenceRef */,
    int      	        /* numProps */,
    SmProp **		/* props */
#endif
);

extern void SmcGetProperties (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmcPropReplyProc	/* propReplyProc */,
    SmPointer		/* clientData */
#endif
);

extern void SmcInteractRequest (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    int			/* dialogType */,
    SmcInteractProc	/* interactProc */,
    SmPointer		/* clientData */
#endif
);

extern void SmcInteractDone (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    Bool 		/* cancelShutdown */
#endif
);

extern void SmcSaveYourselfDone (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    Bool		/* success */
#endif
);

extern int SmcProtocolVersion (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */
#endif
);

extern int SmcProtocolRevision (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */
#endif
);

extern char *SmcVendor (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */
#endif
);

extern char *SmcRelease (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */
#endif
);

extern char *SmcClientID (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */
#endif
);

extern IceConn SmcGetIceConnection (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */
#endif
);

extern Status SmsInitialize (
#if NeedFunctionPrototypes
    char *		/* vendor */,
    char *		/* release */,
    SmsNewClientProc	/* newClientProc */,
    SmPointer		/* managerData */,
    IceHostBasedAuthProc /* hostBasedAuthProc */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern char *SmsClientHostName (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern void SmsRegisterClientReply (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    char *		/* clientId */
#endif
);

extern void SmsSaveYourself (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    int			/* saveType */,
    Bool 		/* shutdown */,
    int			/* interactStyle */,
    Bool		/* fast */
#endif
);

extern void SmsInteract (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern void SmsDie (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern void SmsShutdownCancelled (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern void SmsReturnProperties (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    int			/* numProps */,
    SmProp **		/* props */
#endif
);

extern void SmsCleanUp (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern int SmsProtocolVersion (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern int SmsProtocolRevision (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern char *SmsVendor (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern char *SmsRelease (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern char *SmsClientID (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern IceConn SmsGetIceConnection (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */
#endif
);

extern SmcErrorHandler SmcSetErrorHandler (
#if NeedFunctionPrototypes
    SmcErrorHandler 	/* handler */
#endif
);

extern SmsErrorHandler SmsSetErrorHandler (
#if NeedFunctionPrototypes
    SmsErrorHandler 	/* handler */
#endif
);

extern void SmFreeProperty (
#if NeedFunctionPrototypes
    SmProp *		/* prop */
#endif
);

extern void SmFreeReasons (
#if NeedFunctionPrototypes
    int			/* count */,
    char **		/* reasonMsgs */
#endif
);

#endif /* SMLIB_H */
