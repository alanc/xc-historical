/* $XConsortium$ */

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



XiePhotospace
XieCreatePhotospace (display)

Display	*display;

{
    xieCreatePhotospaceReq	*req;
    char			*pBuf;
    XiePhotospace		id;

    LockDisplay (display);

    id = XAllocID (display);

    GET_REQUEST (CreatePhotospace, pBuf);

    BEGIN_REQUEST_HEADER (CreatePhotospace, pBuf, req);

    STORE_REQUEST_HEADER (CreatePhotospace, req);
    req->nameSpace = id;

    END_REQUEST_HEADER (CreatePhotospace, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (id);
}


void
XieDestroyPhotospace (display, photospace)

Display		*display;
XiePhotospace	photospace;

{
    xieDestroyPhotospaceReq	*req;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (DestroyPhotospace, pBuf);

    BEGIN_REQUEST_HEADER (DestroyPhotospace, pBuf, req);

    STORE_REQUEST_HEADER (DestroyPhotospace, req);
    req->nameSpace = photospace;

    END_REQUEST_HEADER (DestroyPhotospace, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


void
XieExecuteImmediate (display, photospace, flo_id,
    notify, elem_list, elem_count)

Display		*display;
XiePhotospace	photospace;
unsigned long	flo_id;
Bool		notify;
XiePhotoElement	*elem_list;
int		elem_count;

{
    xieExecuteImmediateReq	*req;
    char			*pBuf, *pStart;
    unsigned			size;
    int				i;

    LockDisplay (display);

    size = _XiePhotofloSize (elem_list, elem_count);

    GET_REQUEST (ExecuteImmediate, pBuf);

    BEGIN_REQUEST_HEADER (ExecuteImmediate, pBuf, req);

    STORE_REQUEST_EXTRA_HEADER (ExecuteImmediate, size, req);
    req->nameSpace = photospace;
    req->floID = flo_id;
    req->numElements = elem_count;
    req->notify = notify;

    END_REQUEST_HEADER (ExecuteImmediate, pBuf, req);

    pBuf = pStart = _XAllocScratch (display, size);

    for (i = 0; i < elem_count; i++) 
	(*_XieElemFuncs[elem_list[i].elemType - 1]) (&pBuf, &elem_list[i]);

    Data (display, pStart, size);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


