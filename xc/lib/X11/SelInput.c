#include "copyright.h"

/* $XConsortium: XSelInput.c,v 11.7 88/08/10 16:08:41 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XSelectInput (dpy, w, mask)
    register Display *dpy;
    Window w;
    long mask;
{
    register xChangeWindowAttributesReq *req;

    LockDisplay(dpy);
    GetReqExtra (ChangeWindowAttributes, 4, req);
    req->window = w;
    req->valueMask = CWEventMask;
    OneDataCard32 (dpy, NEXTPTR(req,xChangeWindowAttributesReq), mask);
    UnlockDisplay(dpy);
    SyncHandle();
}

