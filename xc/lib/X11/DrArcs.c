#include "copyright.h"

/* $XConsortium: XDrArcs.c,v 11.13 89/05/26 18:28:30 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

#define arc_scale (SIZEOF(xArc) / 4)

XDrawArcs(dpy, d, gc, arcs, n_arcs)
register Display *dpy;
Drawable d;
GC gc;
XArc *arcs;
int n_arcs;
{
    register xPolyArcReq *req;
    register long len;

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq(PolyArc,req);
    req->drawable = d;
    req->gc = gc->gid;
    len = ((long)n_arcs) * arc_scale;
    if ((req->length + len) > 65535) { /* force BadLength, if possible */
	n_arcs = (65535 - req->length) / arc_scale;
	len = ((long)n_arcs) * arc_scale;
    }
    req->length += len;
    len <<= 2; /* watch out for macros... */
    Data16 (dpy, (short *) arcs, len);
    UnlockDisplay(dpy);
    SyncHandle();
}
