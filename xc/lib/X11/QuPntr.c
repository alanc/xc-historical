#include "copyright.h"

/* $XConsortium: XQuPntr.c,v 11.13 88/08/11 14:32:20 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

Bool XQueryPointer(dpy, w, root, child, root_x, root_y, win_x, win_y, mask)
     register Display *dpy;
     Window w, *root, *child;
     int *root_x, *root_y, *win_x, *win_y;
     unsigned int *mask;

{       
    xQueryPointerReply rep;
    xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(QueryPointer, w, req);
    if (_XReply (dpy, (xReply *)&rep, 0, xTrue) == 0) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return(False);
	}

    *root = rep.root;
    *child = rep.child;
    *root_x = cvtINT16toInt (rep.rootX);
    *root_y = cvtINT16toInt (rep.rootY);
    *win_x = cvtINT16toInt (rep.winX);
    *win_y = cvtINT16toInt (rep.winY);
    *mask = rep.mask;
    UnlockDisplay(dpy);
    SyncHandle();
    return (rep.sameScreen);
}

