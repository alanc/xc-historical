#include "copyright.h"

/* $Header: XUninsCmap.c,v 11.5 87/05/24 21:40:36 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XUninstallColormap(dpy, cmap)
register Display *dpy;
Colormap cmap;
{
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(UninstallColormap, cmap, req);
    UnlockDisplay(dpy);
    SyncHandle();
}
