#include "copyright.h"

/* $Header: XDefCursor.c,v 11.5 87/09/11 08:02:40 toddb Exp $ */
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
    OneDataCard32 (dpy, NEXTPTR(req,xChangeWindowAttributesReq), cursor);
    UnlockDisplay(dpy);
    SyncHandle();
}

