#include "copyright.h"

/* $Header: XCirWinUp.c,v 11.6 87/05/29 17:35:49 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XCirculateSubwindowsUp(dpy, w)
    register Display *dpy;
    Window w;
{
    register xCirculateWindowReq *req;

    LockDisplay(dpy);
    GetReq(CirculateWindow, req);
    req->window = w;
    req->direction = RaiseLowest;
    UnlockDisplay(dpy);
    SyncHandle();
}

