#include "copyright.h"

/* $Header: XBdrWidth.c,v 11.5 87/09/11 08:09:10 toddb Exp $ */
/* Copyright    Massachusetts Institute of Technology    1987 */

#include "Xlibint.h"

XSetWindowBorderWidth(dpy, w, width)
Display *dpy;
Window w;
unsigned int width;
{
    unsigned long lwidth = width;	/* must be CARD32 */

    register xConfigureWindowReq *req;
    LockDisplay(dpy);
    GetReqExtra(ConfigureWindow, 4, req);
    req->window = w;
    req->mask = CWBorderWidth;
    OneDataCard32 (dpy, NEXTPTR(req,xConfigureWindowReq), lwidth);
    UnlockDisplay(dpy);
    SyncHandle();
}

