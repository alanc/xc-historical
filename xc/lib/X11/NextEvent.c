/* $XConsortium: NextEvent.c,v 11.17 93/01/28 13:16:38 gildea Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

#define NEED_EVENTS
#include "Xlibint.h"

/* 
 * Return next event in queue, or if none, flush output and wait for
 * events.
 */

XNextEvent (dpy, event)
	register Display *dpy;
	register XEvent *event;
{
	register _XQEvent *qelt;
	
	LockDisplay(dpy);
	
	if (dpy->head == NULL)
	    _XReadEvents(dpy);
	qelt = dpy->head;
	*event = qelt->event;
	_XDeq(dpy, NULL, qelt);
	UnlockDisplay(dpy);
	return 0;
}

