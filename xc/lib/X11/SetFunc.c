#include "copyright.h"

/* $Header: XSetFunc.c,v 11.7 87/05/26 16:56:11 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetFunction (dpy, gc, function)
register Display *dpy;
GC gc;
int function;
{
    LockDisplay(dpy);
    if (gc->values.function != function) {
	gc->values.function = function;
	gc->dirty |= GCFunction;
    }
    UnlockDisplay(dpy);
    SyncHandle();
}
