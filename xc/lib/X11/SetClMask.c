#include "copyright.h"

/* $Header: XSetClMask.c,v 11.6 87/06/01 16:23:43 sue Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetClipMask (dpy, gc, mask)
register Display *dpy;
GC gc;
Pixmap mask;
{
    LockDisplay(dpy);
    if ((gc->values.clip_mask != mask) || (gc->rects == 1)) {
	gc->values.clip_mask = mask;
	gc->dirty |= GCClipMask;
	gc->rects = 0;
	_XFlushGCCache(dpy, gc);
    }
    UnlockDisplay(dpy);
    SyncHandle();
}
