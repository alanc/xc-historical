#include "copyright.h"

/* $Header: XSetDashes.c,v 11.7 87/06/01 18:06:29 drewry Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetDashes (dpy, gc, dash_offset, list, n)
    register Display *dpy;
    GC gc;
    int dash_offset;
    char *list;
    int n;
    {
    register xSetDashesReq *req;

    LockDisplay(dpy);
    GetReq (SetDashes,req);
    req->gc = gc->gid;
    req->dashOffset = dash_offset;
    req->nDashes = n;
    req->length += (n+3)>>2;
    gc->dashes = 1;
    gc->dirty &= ~GCDashList;
    Data (dpy, list, (long)n);
    UnlockDisplay(dpy);
    SyncHandle();
    }
    
