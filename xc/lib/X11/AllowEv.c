#include "copyright.h"

/* $Header: XAllowEv.c,v 1.11 87/05/24 21:32:27 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XAllowEvents(dpy, mode, time)
    register Display *dpy;
    int mode;
    Time time;

{
    register xAllowEventsReq *req;

    LockDisplay(dpy);
    GetReq(AllowEvents,req);
    req->mode = mode;
    req->time = time;
    UnlockDisplay(dpy);
    SyncHandle();
}



