#include "copyright.h"

/* $XConsortium: XGetMoEv.c,v 11.15 89/03/27 11:35:35 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

XTimeCoord *XGetMotionEvents(dpy, w, start, stop, nEvents)
    register Display *dpy;
    Time start, stop;
    Window w;
    int *nEvents;  /* RETURN */
{       
    xGetMotionEventsReply rep;
    register xGetMotionEventsReq *req;
    XTimeCoord *tc;
    long nbytes;
    LockDisplay(dpy);
    GetReq(GetMotionEvents, req);
    req->window = w;
/* XXX is this right for all machines? */
    req->start = start;
    req->stop  = stop;
    if (!_XReply (dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
        SyncHandle();
	*nEvents = 0;
	return (NULL);
	}
    
    *nEvents = rep.nEvents;
    tc = (XTimeCoord *) Xmalloc (
		(unsigned)(nbytes = (long)rep.nEvents * sizeof (XTimeCoord)));
    if (!tc) {
	_XEatData (dpy, nbytes);		/* throw it away */
	*nEvents = 0;
	UnlockDisplay(dpy);
        SyncHandle();
	return (NULL);
	}

    nbytes = SIZEOF (xTimecoord);
    {
	register XTimeCoord *tcptr;
	register int i;
	xTimecoord xtc;

	for (i = rep.nEvents, tcptr = tc; i > 0; i--, tcptr++) {
	    _XRead (dpy, (char *) &xtc, nbytes);
	    tcptr->time = xtc.time;
	    tcptr->x    = cvtINT16ToInt (xtc.x);
	    tcptr->y    = cvtINT16ToInt (xtc.y);
	}
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return (tc);
}

