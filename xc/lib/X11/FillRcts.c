#include "copyright.h"

/* $Header: XFillRects.c,v 11.10 88/08/10 16:08:25 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XFillRectangles(dpy, d, gc, rectangles, n_rects)
register Display *dpy;
Drawable d;
GC gc;
XRectangle *rectangles;
int n_rects;
{
    register xPolyFillRectangleReq *req;
    register long nbytes;

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq(PolyFillRectangle, req);
    req->drawable = d;
    req->gc = gc->gid;

    /* SIZEOF(xRectangle) will be a multiple of 4 */
    req->length += n_rects * (SIZEOF(xRectangle) / 4);

    nbytes = n_rects * SIZEOF(xRectangle);

    Data16 (dpy, (short *) rectangles, nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}
    
