#include "copyright.h"

/* $Header: XSetCRects.c,v 11.12 87/06/03 12:21:08 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetClipRectangles (dpy, gc, clip_x_origin, clip_y_origin, rectangles, n,
                    ordering)
    register Display *dpy;
    GC gc;
    int clip_x_origin, clip_y_origin;
    XRectangle *rectangles;
    int n;
    int ordering;
{
    LockDisplay(dpy);
    _XSetClipRectangles (dpy, gc, clip_x_origin, clip_y_origin, rectangles, n,
                    ordering)
    UnlockDisplay(dpy);
    SyncHandle();
}
    
/* can only call when display is locked. */
_XSetClipRectangles (dpy, gc, clip_x_origin, clip_y_origin, rectangles, n,
                    ordering)
    register Display *dpy;
    GC gc;
    int clip_x_origin, clip_y_origin;
    XRectangle *rectangles;
    int n;
    int ordering;
{
    register xSetClipRectanglesReq *req;
    register long nbytes;

    GetReq (SetClipRectangles, req);
    req->gc = gc->gid;
    req->xOrigin = clip_x_origin;
    req->yOrigin = clip_y_origin;
    req->ordering = ordering;
    req->length += n * (sizeof (xRectangle)/4);
    gc->rects = 1;
    nbytes = (long)n * sizeof(xRectangle);
    PackData (dpy, (char *) rectangles, nbytes);
    gc->dirty &= ~GCClipMask;
}
