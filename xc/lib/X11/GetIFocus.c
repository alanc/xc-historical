#include "copyright.h"

/* $Header: XGetIFocus.c,v 11.8 87/05/24 21:35:47 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

XGetInputFocus(dpy, focus, revert_to)
     register Display *dpy;
     Window *focus;
     int *revert_to;
{       
    xGetInputFocusReply rep;
    register xReq *req;
    LockDisplay(dpy);
    GetEmptyReq(GetInputFocus, req);
    (void) _XReply (dpy, (xReply *)&rep, 0, xTrue);
    *focus = rep.focus;
    *revert_to = rep.revertTo;
    UnlockDisplay(dpy);
    SyncHandle();
}

