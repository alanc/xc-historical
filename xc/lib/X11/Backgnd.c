#include "copyright.h"

/* $Header: XBackgnd.c,v 11.5 87/05/24 21:32:32 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetWindowBackground(dpy, w, pixel)
    register Display *dpy;
    Window w;
    unsigned long pixel;
{
    register xChangeWindowAttributesReq *req;

    LockDisplay(dpy);
    GetReqExtra (ChangeWindowAttributes, 4, req);
    req->window = w;
    req->valueMask = CWBackPixel;
    * (unsigned long *) (req + 1) = pixel;
    UnlockDisplay(dpy);
    SyncHandle();
}

