#include "copyright.h"

/* $Header: XSetBack.c,v 11.7 87/05/26 16:56:05 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetBackground (dpy, gc, background)
register Display *dpy;
GC gc;
unsigned long background; /* CARD32 */
{
    LockDisplay(dpy);
    if (gc->values.background != background) {
	gc->values.background = background;
	gc->dirty |= GCBackground;
    }
    UnlockDisplay(dpy);
    SyncHandle();
}
