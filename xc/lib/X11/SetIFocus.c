#include "copyright.h"

/* $Header: XSetIFocus.c,v 11.7 87/05/24 21:39:19 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetInputFocus(dpy, focus, revert_to, time)
    register Display *dpy;
    Window focus;
    int revert_to;
    Time time;
{       
    register xSetInputFocusReq *req;

    LockDisplay(dpy);
    GetReq(SetInputFocus, req);
    req->focus = focus;
    req->revertTo = revert_to;
    req->time = time;
    UnlockDisplay(dpy);
    SyncHandle();
}

