#include "copyright.h"

/* $Header: XFreeCurs.c,v 11.5 87/05/24 21:35:22 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XFreeCursor(dpy, cursor)
    register Display *dpy;
    Cursor cursor;
{
    register xResourceReq *req;
    LockDisplay(dpy);
    GetResReq(FreeCursor, cursor, req);
    UnlockDisplay(dpy);
    SyncHandle();
}

