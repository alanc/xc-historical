#include "copyright.h"

/* $Header: XDrRects.c,v 11.10 88/08/10 16:09:15 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDrawRectangles(dpy, d, gc, rects, n_rects)
register Display *dpy;
Drawable d;
GC gc;
XRectangle *rects;
int n_rects;
{
    register xPolyRectangleReq *req;

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq(PolyRectangle, req);
    req->drawable = d;
    req->gc = gc->gid;

    /* SIZEOF(xRectangle) will be a multiple of 4 */
    req->length += n_rects * (SIZEOF(xRectangle) / 4);

    n_rects *= SIZEOF(xRectangle);
    Data16 (dpy, (short *) rects, n_rects);
    UnlockDisplay(dpy);
    SyncHandle();
}
