/* Copyright 	Massachusetts Institute of Technology  1986 */
/* $XConsortium: PutBEvent.c,v 11.12 93/01/28 13:16:56 gildea Exp $ */

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

/* XPutBackEvent puts an event back at the head of the queue. */
#define NEED_EVENTS
#include "Xlibint.h"

XPutBackEvent (dpy, event)
	register Display *dpy;
	register XEvent *event;
	{
	register _XQEvent *qelt;

	LockDisplay(dpy);
	if (!dpy->qfree) {
    	    if ((dpy->qfree = (_XQEvent *) Xmalloc (sizeof (_XQEvent))) == NULL) {
		UnlockDisplay(dpy);
		return 0;
	    }
	    dpy->qfree->next = NULL;
	}
	qelt = dpy->qfree;
	dpy->qfree = qelt->next;
	qelt->qserial_num = dpy->next_event_serial_num++;
	qelt->next = dpy->head;
	qelt->event = *event;
	dpy->head = qelt;
	if (dpy->tail == NULL)
	    dpy->tail = qelt;
	dpy->qlen++;
	UnlockDisplay(dpy);
	return 0;
	}
