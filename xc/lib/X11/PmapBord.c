#include "copyright.h"

/* $Header: XPmapBord.c,v 11.5 87/05/24 21:37:44 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetWindowBorderPixmap(dpy, w, pixmap)
    register Display *dpy;
    Window w;
    Pixmap pixmap;
{
    register xChangeWindowAttributesReq *req;
    LockDisplay(dpy);
    GetReqExtra (ChangeWindowAttributes, 4, req);
    req->window = w;
    req->valueMask = CWBorderPixmap;
    * (unsigned long *) (req + 1) = pixmap;
    UnlockDisplay(dpy);
    SyncHandle();
}

