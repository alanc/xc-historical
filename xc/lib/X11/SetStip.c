#include "copyright.h"

/* $Header: XSetStip.c,v 11.11 87/05/26 16:55:24 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetStipple (dpy, gc, stipple)
register Display *dpy;
GC gc;
Pixmap stipple;
{
    LockDisplay(dpy);

    if (gc->values.stipple != stipple) {
	gc->values.stipple = stipple;
	gc->dirty |= GCStipple;
	_XFlushGCCache(dpy, gc);
    }
    UnlockDisplay(dpy);
    SyncHandle();
}
