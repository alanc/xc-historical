#include "copyright.h"

/* $Header: XDrLines.c,v 11.10 88/08/10 16:09:12 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDrawLines (dpy, d, gc, points, npoints, mode)
    register Display *dpy;
    Drawable d;
    GC gc;
    XPoint *points;
    int npoints;
    int mode;
{
    register xPolyLineReq *req;
    register long length;
    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq (PolyLine, req);
    req->drawable = d;
    req->gc = gc->gid;
    req->coordMode = mode;
    req->length += npoints;
       /* each point is 2 16-bit integers */
    length = npoints << 2;		/* watch out for macros... */
    Data16 (dpy, (short *) points, length);
    UnlockDisplay(dpy);
    SyncHandle();
}

