#include "copyright.h"

/* $Header: XLowerWin.c,v 11.5 87/09/11 08:04:56 toddb Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XLowerWindow(dpy, w)
Display *dpy;
Window w;
{
    register xConfigureWindowReq *req;
    unsigned long val = Below;		/* needed for macro */

    LockDisplay(dpy);
    GetReqExtra(ConfigureWindow, 4, req);
    req->window = w;
    req->mask = CWStackMode;
    OneDataCard32 (dpy, NEXTPTR(req,XConfigureWindowReq), val);
    UnlockDisplay(dpy);
    SyncHandle();
}

