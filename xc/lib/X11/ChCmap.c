#include "copyright.h"

/* $XConsortium: XChCmap.c,v 11.6 88/08/10 16:08:01 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetWindowColormap(dpy, w, colormap)
    register Display *dpy;
    Window w;
    Colormap colormap;
{
    register xChangeWindowAttributesReq *req;

    LockDisplay(dpy);
    GetReqExtra (ChangeWindowAttributes, 4, req);
    req->window = w;
    req->valueMask = CWColormap;
    OneDataCard32 (dpy, NEXTPTR(req,xChangeWindowAttributesReq), colormap);
    UnlockDisplay(dpy);
    SyncHandle();
}

