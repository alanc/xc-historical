#include "copyright.h"

/* $Header: XSetTile.c,v 11.11 87/05/26 16:55:49 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetTile (dpy, gc, tile)
register Display *dpy;
GC gc;
Pixmap tile;
{
    LockDisplay(dpy);
    if (gc->values.tile != tile) {
	gc->values.tile = tile;
	gc->dirty |= GCTile;
	_XFlushGCCache(dpy, gc);
    }
    UnlockDisplay(dpy);
    SyncHandle();
}
