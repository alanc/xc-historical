#include "copyright.h"

/* $Header: XPeekEvent.c,v 11.11 87/09/11 08:05:29 toddb Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_EVENTS
#include "Xlibint.h"

extern _XQEvent *_qfree;

/* 
 * Return the next event in the queue,
 * BUT do not remove it from the queue.
 * If none found, flush and wait until there is an event to peek.
 */

XPeekEvent (dpy, event)
	register Display *dpy;
	register XEvent *event;
{
	LockDisplay(dpy);
	if (dpy->head == NULL)
	    _XReadEvents(dpy);
	*event = (dpy->head)->event;
	UnlockDisplay(dpy);
}

