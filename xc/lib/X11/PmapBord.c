#include "copyright.h"

/* $XConsortium: XPmapBord.c,v 11.6 88/08/10 16:08:33 jim Exp $ */
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
    OneDataCard32 (dpy, NEXTPTR(req,xChangeWindowAttributesReq), pixmap);
    UnlockDisplay(dpy);
    SyncHandle();
}

