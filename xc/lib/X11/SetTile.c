#include "copyright.h"

/* $XConsortium: XSetTile.c,v 11.12 88/06/20 11:57:00 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetTile (dpy, gc, tile)
register Display *dpy;
GC gc;
Pixmap tile;
{
    LockDisplay(dpy);
    /* always update, since client may have changed pixmap contents */
    gc->values.tile = tile;
    gc->dirty |= GCTile;
    _XFlushGCCache(dpy, gc);
    UnlockDisplay(dpy);
    SyncHandle();
}
