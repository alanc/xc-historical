#include "copyright.h"

/* $Header: XPmapBgnd.c,v 11.5 87/05/24 21:37:42 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetWindowBackgroundPixmap(dpy, w, pixmap)
    register Display *dpy;
    Window w;
    Pixmap pixmap;
{
    register xChangeWindowAttributesReq *req;
    LockDisplay (dpy);
    GetReqExtra (ChangeWindowAttributes, 4, req);
    req->window = w;
    req->valueMask = CWBackPixmap;
    * (unsigned long *) (req + 1) = pixmap;
    UnlockDisplay (dpy);
    SyncHandle();
}

