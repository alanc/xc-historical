#include "copyright.h"

/* $XConsortium: XUndefCurs.c,v 11.6 88/08/10 16:08:53 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XUndefineCursor (dpy,w)
    register Display *dpy;
    Window w;
{
    register xChangeWindowAttributesReq *req;
    unsigned long defcurs = None;

    LockDisplay(dpy);
    GetReqExtra (ChangeWindowAttributes, 4, req);
    req->window = w;
    req->valueMask = CWCursor;
    OneDataCard32 (dpy, NEXTPTR(req,xChangeWindowAttributesReq), defcurs);
    UnlockDisplay(dpy);
    SyncHandle();
}

