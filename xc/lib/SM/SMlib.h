/* $XConsortium: SMlib.h,v 1.2 93/09/03 17:08:36 mor Exp $ */
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

typedef void (*SmcSaveYourselfCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */,
    int  		/* saveType */,
    Bool		/* shutdown */,
    int			/* interactStyle */,
    Bool		/* fast */
#endif
);

typedef void (*SmcInteractCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */
#endif
);

typedef void (*SmcDieCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */
#endif
);

typedef void (*SmcShutdownCancelledCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */
#endif
);

typedef void (*SmcPropReplyCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* clientData */,
    int			/* numProps */,
    SmProp *		/* props */
#endif
);


/*
 * Callbacks set up at SmcInitialize time
 */

typedef struct {
    SmcSaveYourselfCB		save_yourself;
    SmcDieCB			die;
    SmcShutdownCancelledCB	shutdown_cancelled;
} SmcCallbacks;


/*
 * Waiting for Properties Reply
 */

typedef struct _SmcPropReplyWait {
    SmcPropReplyCB		prop_reply_cb;
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
     * Client data specified in SmcOpenConnection.  This pointer is passed
     * to each Smc callback.
     */

    SmPointer		client_data;


    /*
     * Callback to be invoked when Interact message arrives.
     */

    SmcInteractCB	interact_cb;


    /*
     * We keep track of all Get Properties sent by the client.  When the
     * Properties Reply arrives, we remove it from the list (a FIFO list
     * is maintained).
     */

    _SmcPropReplyWait	*prop_reply_waits;
};



/*
 * Session manager callbacks
 */

typedef void (*SmsNewClientCB) (
#if NeedFunctionPrototypes
    SmsConn 		/* smsConn */,
    SmPointer *		/* managerDataRet */
#endif
);

typedef void (*SmsRegisterClientCB) (
#if NeedFunctionPrototypes
    SmsConn 		/* smsConn */,
    SmPointer		/* managerData */,
    char *		/* previousId */				     
#endif
);

typedef void (*SmsInteractRequestCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    int			/* dialogType */
#endif
);

typedef void (*SmsInteractDoneCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    Bool		/* cancelShutdown */
#endif
);

typedef void (*SmsSaveYourselfDoneCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    Bool		/* success */
#endif
);

typedef void (*SmsCloseConnectionCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    char *		/* locale */,
    int			/* count */,
    char **		/* reasonMsgs */
#endif
);

typedef void (*SmsSetPropertiesCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    unsigned long 	/* sequenceRef */,
    int			/* numProps */,
    SmProp *		/* props */
#endif
);

typedef void (*SmsGetPropertiesCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */
#endif
);


/*
 * Callbacks set up at SmsInitialize time
 */

typedef struct {
    SmsNewClientCB		new_client;
    SmsRegisterClientCB		register_client;
    SmsInteractRequestCB	interact_request;
    SmsInteractDoneCB		interact_done;
    SmsSaveYourselfDoneCB	save_yourself_done;
    SmsCloseConnectionCB	close_connection;
    SmsSetPropertiesCB		set_properties;
    SmsGetPropertiesCB		get_properties;
} SmsCallbacks;


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
     * Manager data specified when the SmsNewClientCB callback is invoked.
     * This pointer is passed to all of the other Sms callbacks.
     */

    SmPointer		manager_data;


    /*
     * Some state.
     */

    Bool		save_yourself_in_progress;
    Bool		waiting_to_interact;
    Bool		interact_in_progress;
};



/*
 * Error handlers
 */

typedef void (*SmcErrorHandler) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    int			/* offendingMinorOpcode */,
    unsigned long 	/* offendingSequence */,
    int 		/* errorClass */,
    int			/* severity */,
    SmPointer		/* data */
#endif
);

typedef void (*SmsErrorHandler) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    int			/* offendingMinorOpcode */,
    unsigned long 	/* offendingSequence */,
    int 		/* errorClass */,
    int			/* severity */,
    SmPointer		/* data */
#endif
);



/*
 * Function Prototypes
 */

extern Status SmcInitialize (
#if NeedFunctionPrototypes
    SmcCallbacks *	/* callbacks */
#endif
);

extern SmcConn SmcOpenConnection (
#if NeedFunctionPrototypes
    char *		/* networkIdsList */,
    SmPointer		/* clientData */,
    char *		/* previousId */,
    char **		/* clientIdRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

extern void SmcCloseConnection (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    char *		/* locale */,
    int			/* count */,
    char **		/* reasonMsgs */
#endif
);

extern void SmcSetProperties (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    unsigned long 	/* sequenceRef */,
    int      	        /* numProps */,
    SmProp *		/* props */
#endif
);

extern void SmcGetProperties (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmcPropReplyCB	/* propReplyCB */
#endif
);

extern void SmcInteractRequest (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    int			/* dialogType */,
    SmcInteractCB	/* interactCB */
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
    SmsCallbacks * 	/* callbacks */
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
    SmProp *		/* props */
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

#endif /* SMLIB_H */
