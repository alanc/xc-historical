/* $XConsortium: SMlibint.h,v 1.9 93/11/08 16:35:06 mor Exp $ */
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
 * Extern declarations
 */

extern SmcConn _SmcConnectionObjs[256];
extern int     _SmcConnectionCount;
extern int     _SmcOpcode;

extern SmsConn _SmsConnectionObjs[256];
extern int     _SmsConnectionCount;
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
