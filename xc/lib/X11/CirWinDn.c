#include "copyright.h"

/* $Header: XCirWinDn.c,v 11.6 87/05/29 17:36:01 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XCirculateSubwindowsDown(dpy,w)
    register Display *dpy;
    Window w;
{
    register xCirculateWindowReq *req;

    LockDisplay(dpy);
    GetReq(CirculateWindow, req);
    req->window = w;
    req->direction = LowerHighest;
    UnlockDisplay(dpy);
    SyncHandle();
}

