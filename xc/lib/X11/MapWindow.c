#include "copyright.h"

/* $Header: XMapWindow.c,v 11.5 87/05/24 21:37:16 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"
XMapWindow (dpy, w)
	Window w;
	register Display *dpy;
{
	register xResourceReq *req;
	LockDisplay (dpy);
        GetResReq(MapWindow, w, req);
	UnlockDisplay (dpy);
	SyncHandle();
}

