#include "copyright.h"

/* $Header: XSetPMask.c,v 11.7 87/05/26 16:55:39 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetPlaneMask (dpy, gc, planemask)
register Display *dpy;
GC gc;
unsigned long planemask; /* CARD32 */
{
    LockDisplay(dpy);
    if (gc->values.plane_mask != planemask) {
	gc->values.plane_mask = planemask;
	gc->dirty |= GCPlaneMask;
    }
    UnlockDisplay(dpy);	
    SyncHandle();
}
