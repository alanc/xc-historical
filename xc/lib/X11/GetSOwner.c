#include "copyright.h"

/* $Header: XGetSOwner.c,v 11.12 87/06/10 23:06:03 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

Window XGetSelectionOwner(dpy, selection)
register Display *dpy;
Atom selection;
{
    xGetSelectionOwnerReply rep;
    register xResourceReq *req;
    LockDisplay(dpy);
    GetResReq(GetSelectionOwner, selection, req);

    if (_XReply(dpy, (xReply *)&rep, 0, xTrue) == 0) rep.owner = None;
    UnlockDisplay(dpy);
    SyncHandle();
    return(rep.owner);
}
