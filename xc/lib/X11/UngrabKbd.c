#include "copyright.h"

/* $Header: XUngrabKbd.c,v 11.8 87/05/24 21:40:26 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"
XUngrabKeyboard (dpy, time)
        register Display *dpy;
	Time time;
{
        register xResourceReq *req;

	LockDisplay(dpy);
        GetResReq(UngrabKeyboard, time, req);
	UnlockDisplay(dpy);
	SyncHandle();
}

