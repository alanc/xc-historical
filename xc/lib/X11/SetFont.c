#include "copyright.h"

/* $Header: XSetFont.c,v 11.7 87/05/26 16:56:00 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetFont (dpy, gc, font)
register Display *dpy;
GC gc;
Font font;
{
    LockDisplay(dpy);
    if (gc->values.font != font) {
        gc->values.font = font;
	gc->dirty |= GCFont;
	_XFlushGCCache(dpy, gc);
    }
    UnlockDisplay(dpy);
    SyncHandle();
}
