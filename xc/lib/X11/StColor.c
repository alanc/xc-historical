#include "copyright.h"

/* $Header: XStColor.c,v 11.6 88/08/09 15:57:26 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XStoreColor(dpy, cmap, def)
register Display *dpy;
Colormap cmap;
XColor *def;
{
    xColorItem *citem;
    register xStoreColorsReq *req;

    LockDisplay(dpy);
    GetReqExtra(StoreColors, SIZEOF(xColorItem), req); /* assume size is 4*n */

    req->cmap = cmap;

    citem = (xColorItem *) NEXTPTR(req,xStoreColorsReq);

    citem->pixel = def->pixel;
    citem->red = def->red;
    citem->green = def->green;
    citem->blue = def->blue;
    citem->flags = def->flags; /* do_red, do_green, do_blue */
    UnlockDisplay(dpy);
    SyncHandle();
}
