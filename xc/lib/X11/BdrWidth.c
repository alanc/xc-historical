#include "copyright.h"

/* $Header: XBdrWidth.c,v 11.5 87/05/31 12:36:32 weissman Exp $ */
/* Copyright    Massachusetts Institute of Technology    1987 */

#include "Xlibint.h"

XSetWindowBorderWidth(dpy, w, width)
Display *dpy;
Window w;
unsigned int width;
{
    register xConfigureWindowReq *req;
    LockDisplay(dpy);
    GetReqExtra(ConfigureWindow, 4, req);
    req->window = w;
    req->mask = CWBorderWidth;
    * (unsigned long *) (req + 1) = width;
    UnlockDisplay(dpy);
    SyncHandle();
}

