#include "copyright.h"

/* $Header: XMoveWin.c,v 11.6 87/09/11 08:05:15 toddb Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XMoveWindow (dpy, w, x, y)
    register Display *dpy;
    Window w;
    int x, y;
{
    register xConfigureWindowReq *req;

    LockDisplay(dpy);
    GetReqExtra(ConfigureWindow, 8, req);

    req->window = w;
    req->mask = CWX | CWY;

#ifdef MUSTCOPY
    {
	long lx = (long) x, ly = (long) y;
	dpy->bufptr -= 8;
	Data32 (dpy, (char *) &lx, 4);	/* order dictated by CWX and CWY */
	Data32 (dpy, (char *) &ly, 4);
    }
#else
    {
	unsigned long *valuePtr =
	  (unsigned long *) NEXTPTR(req,xConfigureWindowReq);
	*valuePtr++ = x;
	*valuePtr = y;
    }
#endif /* MUSTCOPY */
    UnlockDisplay(dpy);
    SyncHandle();
}

