#include "copyright.h"

/* $Header: XSetSOwner.c,v 11.7 87/05/24 21:39:39 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSetSelectionOwner(dpy, selection, owner, time)
register Display *dpy;
Atom selection;
Window owner;
Time time;
{
    register xSetSelectionOwnerReq *req;

    LockDisplay(dpy);
    GetReq(SetSelectionOwner,req);
    req->selection = selection;
    req->window = owner;
    req->time = time;
    UnlockDisplay(dpy);
    SyncHandle();
}
