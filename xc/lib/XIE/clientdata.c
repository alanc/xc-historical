/* $XConsortium: clientdata.c,v 1.1 93/07/19 11:39:13 mor Exp $ */

/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
******************************************************************************/

#include "XIElibint.h"

#include <stdio.h>


void
XiePutClientData (display, name_space, flo_id, element, final,
    band_number, data, nbytes)

Display      	*display;
unsigned long  	name_space;
unsigned long  	flo_id;
XiePhototag	element;
Bool         	final;
unsigned     	band_number;
unsigned char  	*data;
unsigned     	nbytes;

{
    xiePutClientDataReq	*req;
    char		*pBuf;

    LockDisplay (display);

    GET_REQUEST (PutClientData, pBuf);

    BEGIN_REQUEST_HEADER (PutClientData, pBuf, req);

    STORE_REQUEST_EXTRA_HEADER (PutClientData, nbytes, req);
    req->nameSpace = name_space;
    req->floID = flo_id;
    req->element = element;
    req->final = final;
    req->bandNumber = band_number;
    req->byteCount = nbytes;

    END_REQUEST_HEADER (PutClientData, pBuf, req);

    Data (display, (char *) data, nbytes);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


Status
XieGetClientData (display, name_space, flo_id, element, max_bytes,
    terminate, band_number, new_state_ret, data_ret, nbytes_ret)

Display      	*display;
unsigned long  	name_space;
unsigned long  	flo_id;
XiePhototag	element;
unsigned  	max_bytes;
Bool		terminate;
unsigned     	band_number;
XieExportState 	*new_state_ret;
unsigned char   **data_ret;
unsigned     	*nbytes_ret;

{
    xieGetClientDataReq		*req;
    xieGetClientDataReply	rep;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (GetClientData, pBuf);

    BEGIN_REQUEST_HEADER (GetClientData, pBuf, req);

    STORE_REQUEST_HEADER (GetClientData, req);
    req->nameSpace = name_space;
    req->floID = flo_id;
    req->maxBytes = max_bytes;
    req->element = element;
    req->terminate = terminate;
    req->bandNumber = band_number;

    END_REQUEST_HEADER (GetClientData, pBuf, req);

    if (_XReply (display, &rep, 0, xFalse) == 0)
	/* V4.13 moved the data length word into the reply */
    {
        UnlockDisplay (display);
	SYNC_HANDLE (display);

   	*nbytes_ret = 0;
	*data_ret = NULL;

	return (0);
    }


    *new_state_ret = rep.newState;
    *nbytes_ret = rep.byteCount;

    *data_ret = (unsigned char *) Xmalloc (PADDED_BYTES (rep.byteCount));

    _XReadPad (display, (char *) *data_ret, rep.byteCount);


    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (1);
}
