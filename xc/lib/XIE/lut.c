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


XieLut
XieCreateLUT (display)

Display	*display;

{
    xieCreateLUTReq	*req;
    char		*pBuf;
    XieLut		id;

    LockDisplay (display);

    id = XAllocID (display);

    GET_REQUEST (CreateLUT, pBuf);

    BEGIN_REQUEST_HEADER (CreateLUT, pBuf, req);

    STORE_REQUEST_HEADER (CreateLUT, req);
    req->lut = id;

    END_REQUEST_HEADER (CreateLUT, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (id);
}


void
XieDestroyLUT (display, lut)

Display	*display;
XieLut	lut;

{
    xieDestroyLUTReq	*req;
    char		*pBuf;

    LockDisplay (display);

    GET_REQUEST (DestroyLUT, pBuf);

    BEGIN_REQUEST_HEADER (DestroyLUT, pBuf, req);

    STORE_REQUEST_HEADER (DestroyLUT, req);
    req->lut = lut;

    END_REQUEST_HEADER (DestroyLUT, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}
