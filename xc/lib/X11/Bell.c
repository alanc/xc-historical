#include "copyright.h"

/* $Header: XBell.c,v 11.6 87/05/24 21:32:36 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XBell(dpy, percent)
    register Display *dpy;
    int percent;
{
    register xBellReq *req;

    LockDisplay(dpy);
    GetReq(Bell,req);
    req->percent = percent;
    UnlockDisplay(dpy);
    SyncHandle();
}

