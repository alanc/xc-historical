/* $XConsortium: PeekIfEv.c,v 11.14 93/01/28 13:17:01 gildea Exp $ */
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
 * return the next event in the queue that satisfies the predicate.
 * BUT do not remove it from the queue.
 * If none found, flush, and then wait until one satisfies the predicate.
 */

XPeekIfEvent (dpy, event, predicate, arg)
	register Display *dpy;
	register XEvent *event;
	Bool (*predicate)(
#if NeedNestedPrototypes
			  Display*			/* display */,
			  XEvent*			/* event */,
			  char*				/* arg */
#endif
			  );
	char *arg;
{
	register _XQEvent *prev, *qelt;
	unsigned long qe_serial = 0;

	LockDisplay(dpy);
	prev = NULL;
	while (1) {
	    for (qelt = prev ? prev->next : dpy->head;
		 qelt;
		 prev = qelt, qelt = qelt->next) {
		if(qelt->qserial_num > qe_serial
		   && (*predicate)(dpy, &qelt->event, arg)) {
		    *event = qelt->event;
		    UnlockDisplay(dpy);
		    return 0;
		}
	    }
	    if (prev)
		qe_serial = prev->qserial_num;
	    _XReadEvents(dpy);
	    if (prev && prev->qserial_num != qe_serial)
		/* another thread has snatched this event */
		prev = NULL;
	}
}

