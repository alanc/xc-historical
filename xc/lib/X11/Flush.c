#include "copyright.h"

/* $Header: XFlush.c,v 11.5 87/05/24 21:35:07 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

/* Flush all buffered output requests. */
/* NOTE: NOT necessary when calling any of the Xlib routines. */

XFlush (dpy)
    register Display *dpy;
    {
    _XFlush (dpy);
    }
