#include "copyright.h"

/* $Header: XLowerWin.c,v 11.6 88/08/10 16:08:28 jim Exp $ */
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
    OneDataCard32 (dpy, NEXTPTR(req,xConfigureWindowReq), val);
    UnlockDisplay(dpy);
    SyncHandle();
}

