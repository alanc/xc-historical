#include "copyright.h"

/* $Header: XCopyCmap.c,v 11.5 87/05/24 21:33:43 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

Colormap XCopyColormapAndFree(dpy, src_cmap)
register Display *dpy;
Colormap src_cmap;
{
    Colormap mid;
    register xCopyColormapAndFreeReq *req;

    LockDisplay(dpy);
    GetReq(CopyColormapAndFree, req);

    mid = req->mid = XAllocID(dpy);
    req->srcCmap = src_cmap;
    UnlockDisplay(dpy);
    SyncHandle();
    return(mid);
}
