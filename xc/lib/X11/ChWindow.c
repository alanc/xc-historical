#include "copyright.h"

/* $Header: XChWindow.c,v 11.6 87/05/31 14:07:45 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XResizeWindow(dpy, w, width, height)
register Display *dpy;
Window w;
unsigned int width, height;
{
    register unsigned long *valuePtr;
    register xConfigureWindowReq *req;

    LockDisplay(dpy);
    GetReqExtra(ConfigureWindow, 8, req); /* 2 4-byte quantities */

    /* XXX assuming that sizeof(unsigned long) is 32 bits */

    req->window = w;
    req->mask = CWWidth | CWHeight;
    valuePtr = (unsigned long *) (req + 1);
    *valuePtr++ = width;
    *valuePtr++ = height;
    UnlockDisplay(dpy);
    SyncHandle();
}
