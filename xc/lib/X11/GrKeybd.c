#include "copyright.h"

/* $Header: XGrKeybd.c,v 11.13 87/06/10 23:06:07 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"
int XGrabKeyboard (dpy, window, ownerEvents, pointerMode, keyboardMode, time)
    register Display *dpy;
    Window window;
    int ownerEvents;  /* boolean */
    int pointerMode, keyboardMode;
    Time time;
{
        xGrabKeyboardReply rep;
	register xGrabKeyboardReq *req;
	register int status;
	LockDisplay(dpy);
        GetReq(GrabKeyboard, req);
	req->grabWindow = window;
	req->ownerEvents = ownerEvents;
	req->pointerMode = pointerMode;
	req->keyboardMode = keyboardMode;
	req->time = time;

       /* if we ever return, suppress the error */
	if (_XReply (dpy, (xReply *) &rep, 0, xTrue) == 0) 
		rep.status = GrabSuccess;
	status = rep.status;
	UnlockDisplay(dpy);
	SyncHandle();
	return (status);
}

