#include "copyright.h"

/* $Header: XDefCursor.c,v 11.5 87/05/24 21:34:11 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDefineCursor (dpy, w, cursor)
    register Display *dpy;
    Window w;
    Cursor cursor;
{
    register xChangeWindowAttributesReq *req;

    LockDisplay(dpy);
    GetReqExtra (ChangeWindowAttributes, 4, req);
    req->window = w;
    req->valueMask = CWCursor;
    * (unsigned long *) (req + 1) = cursor;
    UnlockDisplay(dpy);
    SyncHandle();
}

