#include "copyright.h"

/* $Header: XSendEvent.c,v 11.8 88/05/16 11:30:37 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_EVENTS
#include "Xlibint.h"

extern Status _XEventToWire();
/*
 * In order to avoid all images requiring _XEventToWire, we install the
 * event converter here if it has never been installed.
 */
Status
XSendEvent(dpy, w, propagate, event_mask, event)
    register Display *dpy;
    Window w;
    Bool propagate;
    long event_mask;
    XEvent *event;
{
    register xSendEventReq *req;
    xEvent ev;
    register Status (**fp)();
    Status status;

    LockDisplay (dpy);

    /* call through display to find proper conversion routine */

    fp = &dpy->wire_vec[event->type & 0177];
    if (*fp == NULL) *fp = _XEventToWire;
    status = (**fp)(dpy, event, &ev);

    if (status) {
	GetReq(SendEvent, req);
	req->destination = w;
	req->propagate = propagate;
	req->eventMask = event_mask;
#ifdef WORD64
	/* avoid quad-alignment problems */
	bcopy ((char *) &ev, (char *) req->eventdata, SIZEOF(xEvent));
#else
	req->event = ev;
#endif /* WORD64 */
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return(status);
}
