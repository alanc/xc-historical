#include "copyright.h"

/* $Header: XRepWindow.c,v 11.5 87/05/24 21:38:41 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XReparentWindow(dpy, w, p, x, y)
    register Display *dpy;
    Window w, p;
    int x, y;
{
    register xReparentWindowReq *req;

    LockDisplay(dpy);
    GetReq(ReparentWindow, req);
    req->window = w;
    req->parent = p;
    req->x = x;
    req->y = y;
    UnlockDisplay(dpy);
    SyncHandle();
}

