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


XieRoi
XieCreateROI (display)

Display	*display;

{
    xieCreateROIReq	*req;
    char		*pBuf;
    XieRoi		id;

    LockDisplay (display);

    id = XAllocID (display);

    GET_REQUEST (CreateROI, pBuf);

    BEGIN_REQUEST_HEADER (CreateROI, pBuf, req);

    STORE_REQUEST_HEADER (CreateROI, req);
    req->roi = id;

    END_REQUEST_HEADER (CreateROI, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (id);
}


void
XieDestroyROI (display, roi)

Display	*display;
XieRoi	roi;

{
    xieDestroyROIReq	*req;
    char		*pBuf;

    LockDisplay (display);

    GET_REQUEST (DestroyROI, pBuf);

    BEGIN_REQUEST_HEADER (DestroyROI, pBuf, req);

    STORE_REQUEST_HEADER (DestroyROI, req);
    req->roi = roi;

    END_REQUEST_HEADER (DestroyROI, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}
