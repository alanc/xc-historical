#include "copyright.h"

/* $Header: XDelProp.c,v 11.4 87/05/24 21:34:16 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDeleteProperty(dpy, window, property)
register Display *dpy;
Window window;
Atom property;
{
    register xDeletePropertyReq *req;

    LockDisplay(dpy);
    GetReq(DeleteProperty, req);
    req->window = window;
    req->property = property;
    UnlockDisplay(dpy);
    SyncHandle();
}
