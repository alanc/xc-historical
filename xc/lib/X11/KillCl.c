#include "copyright.h"

/* $Header: XKillCl.c,v 11.7 87/05/24 21:36:46 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"
XKillClient(dpy, resource)
	register Display *dpy;
	XID resource;
{
	register xResourceReq *req;
	LockDisplay(dpy);
        GetResReq(KillClient, resource, req);
	UnlockDisplay(dpy);
	SyncHandle();
}

