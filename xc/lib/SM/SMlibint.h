/* $XConsortium: SMlibint.h,v 1.12 94/01/31 11:05:35 mor Exp $ */
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

#ifndef SMLIBINT_H
#define SMLIBINT_H

#include <X11/ICE/ICElibint.h>
#include <X11/SM/SMproto.h>


/*
 * Vendor & Release
 */

#define SmVendorString	"MIT"
#define SmReleaseString	"1.0"


/*
 * Compute the number of bytes for an ARRAY8 representation
 */

#define ARRAY8_BYTES(_len) (4 + _len + PAD64 (4 + _len))


/*
 * Compute the number of bytes for a LISTofPROPERTY representation
 */

#define LISTOF_PROP_BYTES(_numProps, _props, _bytes) \
{ \
    int _i, _j; \
    _bytes = 8; \
    for (_i = 0; _i < _numProps; _i++) \
    { \
	_bytes += (8 + ARRAY8_BYTES (strlen (_props[_i]->name)) + \
	    ARRAY8_BYTES (strlen (_props[_i]->type))); \
\
	for (_j = 0; _j < _props[_i]->num_vals; _j++) \
	    _bytes += ARRAY8_BYTES (_props[_i]->vals[_j].length); \
    } \
}


/*
 * STORE FOO
 */

#define STORE_ARRAY8(_pBuf, _len, _array8) \
{ \
    STORE_CARD32 (_pBuf, _len); \
    memcpy (_pBuf, _array8, _len); \
    _pBuf += _len; \
    if (PAD64 (4 + _len)) \
        _pBuf += PAD64 (4 + _len); \
}

#define STORE_LISTOF_PROPERTY(_pBuf, _count, _props) \
{ \
    int _i, _j; \
    STORE_CARD32 (_pBuf, _count); \
    _pBuf += 4; \
    for (_i = 0; _i < _count; _i++) \
    { \
        STORE_ARRAY8 (_pBuf, strlen (_props[_i]->name), _props[_i]->name); \
        STORE_ARRAY8 (_pBuf, strlen (_props[_i]->type), _props[_i]->type); \
        STORE_CARD32 (_pBuf, _props[_i]->num_vals); \
        _pBuf += 4; \
        for (_j = 0; _j < _props[_i]->num_vals; _j++) \
	{ \
            STORE_ARRAY8 (_pBuf, _props[_i]->vals[_j].length, \
		(char *) _props[_i]->vals[_j].value); \
	} \
    } \
}


/*
 * EXTRACT FOO
 */

#define EXTRACT_ARRAY8(_pBuf, _swap, _len, _array8) \
{ \
    EXTRACT_CARD32 (_pBuf, _swap, _len); \
    _array8 = (char *) malloc (_len + 1); \
    memcpy (_array8, _pBuf, _len); \
    _array8[_len] = '\0'; \
    _pBuf += _len; \
    if (PAD64 (4 + _len)) \
        _pBuf += PAD64 (4 + _len); \
}

#define EXTRACT_ARRAY8_AS_STRING(_pBuf, _swap, _string) \
{ \
    CARD32 _len; \
    EXTRACT_CARD32 (_pBuf, _swap, _len); \
    _string = (char *) malloc (_len + 1); \
    memcpy (_string, _pBuf, _len); \
    _string[_len] = '\0'; \
    _pBuf += _len; \
    if (PAD64 (4 + _len)) \
        _pBuf += PAD64 (4 + _len); \
}

#define EXTRACT_LISTOF_PROPERTY(_pBuf, _swap, _count, _props) \
{ \
    int _i, _j; \
    EXTRACT_CARD32 (_pBuf, _swap, _count); \
    _pBuf += 4; \
    _props = (SmProp **) malloc (_count * sizeof (SmProp *)); \
    for (_i = 0; _i < _count; _i++) \
    { \
        _props[_i] = (SmProp *) malloc (sizeof (SmProp)); \
        EXTRACT_ARRAY8_AS_STRING (_pBuf, _swap, _props[_i]->name); \
        EXTRACT_ARRAY8_AS_STRING (_pBuf, _swap, _props[_i]->type); \
        EXTRACT_CARD32 (_pBuf, _swap, _props[_i]->num_vals); \
        _pBuf += 4; \
        _props[_i]->vals = (SmPropValue *) malloc ( \
	    _props[_i]->num_vals * sizeof (SmPropValue)); \
        for (_j = 0; _j < _props[_i]->num_vals; _j++) \
	{ \
	    char *_temp; \
            EXTRACT_ARRAY8 (_pBuf, _swap, _props[_i]->vals[_j].length, _temp);\
	    _props[_i]->vals[_j].value = (SmPointer) _temp; \
	} \
    } \
}


/*
 * Client replies not processed by callbacks (we block for them).
 */

typedef struct {
    Status  	status;		/* if 1, client successfully registered */
    char	*client_id;
} _SmcRegisterClientReply;


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
 * Extern declarations
 */

extern int     _SmcOpcode;
extern int     _SmsOpcode;

extern int		_SmVersionCount;
extern IcePoVersionRec	_SmcVersions[];
extern IcePaVersionRec	_SmsVersions[];

extern int	        _SmAuthCount;
extern char		*_SmAuthNames[];
extern IcePoAuthProc	_SmcAuthProcs[];
extern IcePaAuthProc	_SmsAuthProcs[];

extern SmsNewClientProc	_SmsNewClientProc;
extern SmPointer	_SmsNewClientData;

extern SmcErrorHandler _SmcErrorHandler;
extern SmsErrorHandler _SmsErrorHandler;

#endif /* SMLIBINT */
