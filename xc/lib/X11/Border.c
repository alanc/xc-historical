#include "copyright.h"

/* $XConsortium: XBorder.c,v 11.6 88/08/10 16:07:58 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetWindowBorder(dpy, w, pixel)
    register Display *dpy;
    Window w;
    unsigned long pixel;
{
    register xChangeWindowAttributesReq *req;

    LockDisplay(dpy);
    GetReqExtra (ChangeWindowAttributes, 4, req);
    req->window = w;
    req->valueMask = CWBorderPixel;
    OneDataCard32 (dpy, NEXTPTR(req,xChangeWindowAttributesReq), pixel);
    UnlockDisplay(dpy);
    SyncHandle();
}

