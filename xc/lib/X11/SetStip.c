#include "copyright.h"

/* $XConsortium: XSetStip.c,v 11.12 88/06/20 11:57:21 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetStipple (dpy, gc, stipple)
register Display *dpy;
GC gc;
Pixmap stipple;
{
    LockDisplay(dpy);
    /* always update, since client may have changed pixmap contents */
    gc->values.stipple = stipple;
    gc->dirty |= GCStipple;
    _XFlushGCCache(dpy, gc);
    UnlockDisplay(dpy);
    SyncHandle();
}
