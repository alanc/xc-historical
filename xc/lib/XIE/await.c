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


void
XieAwait (display, name_space, flo_id)

Display		*display;
unsigned long	name_space;
unsigned long	flo_id;

{
    xieAwaitReq	*req;
    char	*pBuf;

    LockDisplay (display);

    GET_REQUEST (Await, pBuf);

    BEGIN_REQUEST_HEADER (Await, pBuf, req);

    STORE_REQUEST_HEADER (Await, req);
    req->nameSpace = name_space;
    req->floID = flo_id;

    END_REQUEST_HEADER (Await, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}
