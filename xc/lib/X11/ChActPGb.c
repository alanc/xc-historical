#include "copyright.h"

/* $Header: XChActPGb.c,v 11.6 87/05/24 21:32:44 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XChangeActivePointerGrab(dpy, event_mask, curs, time)
register Display *dpy;
unsigned int event_mask; /* CARD16 */
Cursor curs;
Time time;
{
    register xChangeActivePointerGrabReq *req;

    LockDisplay(dpy);
    GetReq(ChangeActivePointerGrab, req);
    req->eventMask = event_mask;
    req->cursor = curs;
    req->time = time;
    UnlockDisplay(dpy);
    SyncHandle();
}
