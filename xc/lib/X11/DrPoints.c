#include "copyright.h"

/* $XConsortium: XDrPoints.c,v 1.11 88/09/06 16:06:53 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDrawPoints(dpy, d, gc, points, n_points, mode)
    register Display *dpy;
    Drawable d;
    GC gc;
    XPoint *points;
    int n_points;
    int mode; /* CoordMode */
{
    register xPolyPointReq *req;
    register long nbytes;
    int n;
    LockDisplay(dpy);
    FlushGC(dpy, gc);
    while (n_points) {
	GetReq(PolyPoint, req);
	req->drawable = d;
	req->gc = gc->gid;
	req->coordMode = mode;
	n = n_points;
	if (n > (dpy->max_request_size - req->length))
	    n = dpy->max_request_size - req->length;
	req->length += n;
	nbytes = n << 2; /* watch out for macros... */
	Data16 (dpy, (short *) points, nbytes);
	n_points -= n;
	if (n_points && (mode == CoordModePrevious)) {
	    register XPoint *pptr = points;
	    points += n;
	    while (pptr != points) {
		points->x += pptr->x;
		points->y += pptr->y;
		pptr++;
	    }
	} else
	    points += n;
    }
    UnlockDisplay(dpy);
    SyncHandle();
}
