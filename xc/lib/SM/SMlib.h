/* $XConsortium: SMlib.h,v 1.1 93/09/03 13:24:47 mor Exp $ */
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
    int		length;
    SmPointer   value;
} SmPropValue;

typedef struct {
    char	*name;
    char	*type;
    int		num_vals;
    SmPropValue *vals;
} SmProp;


/*
 * Client callbacks
 */

typedef void (*SmcSaveYourselfCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* callData */,
    int  		/* saveType */,
    Bool		/* shutdown */,
    int			/* interactStyle */,
    Bool		/* fast */
#endif
);

typedef void (*SmcInteractCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* callData */
#endif
);

typedef void (*SmcDieCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* callData */
#endif
);

typedef void (*SmcShutdownCancelledCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* callData */
#endif
);

typedef void (*SmcPropReplyCB) (
#if NeedFunctionPrototypes
    SmcConn		/* smcConn */,
    SmPointer		/* callData */,
    int			/* numProps */,
    SmProp *		/* props */
#endif
);


/*
 * Callbacks setup at SmcInitialize time
 */

typedef struct {
    SmcSaveYourselfCB		save_yourself;
    SmcDieCB			die;
    SmcShutdownCancelledCB	shutdown_cancelled;
} SmcCallbacks;


/*
 * Client replies not processed by callbacks (we block for them).
 */

#define SMC_ERROR_REPLY			1
#define SMC_REGISTER_CLIENT_REPLY	2

typedef struct {
    int		  	type;
    int			client_id_len;
    char		*client_id;
} SmcRegisterClientReply;

typedef union {
    int				type;
    SmcRegisterClientReply	register_client_reply;
} SmcReply;


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
    IceConn		iceConn;
    int			proto_major_version;
    int			proto_minor_version;
    char		*vendor;
    char		*release;
    int			client_id_len;
    char		*client_id;
    SmPointer		call_data;

    SmcInteractCB	interact_cb;

    /*
     * We keep track of all Get Properties sent by the client.  When the
     * Properties Reply arrives, we remove it from the list.
     */

    _SmcPropReplyWait	*prop_reply_waits;
};


/*
 * Session manager callbacks
 */

typedef void (*SmsNewClientCB) (
#if NeedFunctionPrototypes
    SmsConn 		/* smsConn */,
    SmPointer *		/* callDataRet */
#endif
);

typedef void (*SmsRegisterClientCB) (
#if NeedFunctionPrototypes
    SmsConn 		/* smsConn */,
    SmPointer		/* callData */,
    int			/* previousIdLen */,
    char *		/* previousId */				     
#endif
);

typedef void (*SmsInteractRequestCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* callData */,
    int			/* dialogType */
#endif
);

typedef void (*SmsInteractDoneCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* callData */,
    Bool		/* cancelShutdown */
#endif
);

typedef void (*SmsSaveYourselfDoneCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* callData */,
    Bool		/* success */
#endif
);

typedef void (*SmsCloseConnectionCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* callData */,
    char *		/* locale */,
    int			/* count */,
    char **		/* reasonMsgs */
#endif
);

typedef void (*SmsSetPropertiesCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* callData */,
    unsigned long 	/* sequenceRef */,
    int			/* numProps */,
    SmProp *		/* props */
#endif
);

typedef void (*SmsGetPropertiesCB) (
#if NeedFunctionPrototypes
    SmsConn		/* smsConn */,
    SmPointer		/* callData */
#endif
);


/*
 * Callbacks setup at SmsInitialize time
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
    IceConn		iceConn;
    int			proto_major_version;
    int			proto_minor_version;
    char		*vendor;
    char		*release;
    int			client_id_len;
    char		*client_id;
    SmPointer		call_data;
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

extern SmcConn
SmcOpenConnection (
#if NeedFunctionPrototypes
    char *		/* networkIdsList */,
    SmPointer		/* callData */,
    int			/* previousIdLen */,
    char *		/* previousId */,
    int *		/* clientIdLenRet */,
    char **		/* clientIdRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

#endif /* SMLIB_H */
