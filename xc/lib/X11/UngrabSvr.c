#include "copyright.h"

/* $Header: XUngrabSvr.c,v 11.5 87/05/24 21:40:34 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"
XUngrabServer (dpy)
register Display *dpy;
{
        register xReq *req;

        LockDisplay(dpy);
        GetEmptyReq(UngrabServer, req);
        UnlockDisplay(dpy);
	SyncHandle();
}

