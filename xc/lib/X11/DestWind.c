#include "copyright.h"

/* $Header: XDestWind.c,v 11.5 87/05/24 21:34:23 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"
XDestroyWindow (dpy, w)
	register Display *dpy;
	Window w;
{
        register xResourceReq *req;

	LockDisplay(dpy);
        GetResReq(DestroyWindow, w, req);
	UnlockDisplay(dpy);
	SyncHandle();
}

