#include "copyright.h"

/* $XConsortium: XMapRaised.c,v 1.7 88/08/10 16:08:29 jim Exp $ */
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

