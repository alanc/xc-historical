#include "copyright.h"

/* $Header: XDestSubs.c,v 11.4 87/05/24 21:34:22 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDestroySubwindows(dpy, win)
register Display *dpy;
Window win;
{
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq (DestroySubwindows,win, req);
    UnlockDisplay(dpy);
    SyncHandle();
}

