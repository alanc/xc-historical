#include "copyright.h"

/* $Header: XChClMode.c,v 11.4 87/05/30 21:26:24 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"


XSetCloseDownMode(dpy, mode)
    register Display *dpy; 
    int mode;

{
    register xSetCloseDownModeReq *req;

    LockDisplay(dpy);
    GetReq(SetCloseDownMode, req);
    req->mode = mode;
    UnlockDisplay(dpy);
    SyncHandle();
}

