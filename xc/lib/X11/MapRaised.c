#include "copyright.h"

/* $Header: XMapRaised.c,v 1.6 87/09/11 08:05:02 toddb Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XMapRaised (dpy, w)
    Window w;
    register Display *dpy;
{
    register xConfigureWindowReq *req;
    register xResourceReq *req2;
    unsigned long val = Above;		/* needed for macro */

    LockDisplay(dpy);
    GetReqExtra(ConfigureWindow, 4, req);
    req->window = w;
    req->mask = CWStackMode;
    OneDataCard32 (dpy, NEXTPTR(req,xConfigureWindowReq), val);
    GetResReq (MapWindow, w, req2);
    UnlockDisplay(dpy);
    SyncHandle();
}

