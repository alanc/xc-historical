#include "copyright.h"

/* $Header: XChkWinEv.c,v 11.9 87/09/08 14:30:29 newman Exp $ */
/* Copyright    Massachusetts Institute of Technology    1985, 1987	*/
#define NEED_EVENTS
#include "Xlibint.h"

extern _XQEvent *_qfree;
extern long _event_to_mask[];
/* 
 * Check existing events in queue to find if any match.  If so, return.
 * If not, flush buffer and see if any more events are readable. If one
 * matches, return.  If all else fails, tell the user no events found.
 */

Bool XCheckWindowEvent (dpy, w, mask, event)
        register Display *dpy;
	Window w;		/* Selected window. */
	int mask;		/* Selected event mask. */
	register XEvent *event;	/* XEvent to be filled in. */
{
 	register _XQEvent *prev, *qelt;
	int n;              /* time through count */

        LockDisplay(dpy);
	prev = NULL;
	for (n = 2; --n >= 0;) {
	    for (qelt = prev ? prev->next : dpy->head;
		 qelt;
		 prev = qelt, qelt = qelt->next) {
		if ((qelt->event.xany.window == w) &&
		    (_event_to_mask[qelt->event.type] & mask)) {
		    *event = qelt->event;
		    if (prev) {
			if ((prev->next = qelt->next) == NULL)
			    dpy->tail = prev;
		    } else {
			if ((dpy->head = qelt->next) == NULL)
			dpy->tail = NULL;
		    }
		    qelt->next = _qfree;
		    _qfree = qelt;
		    dpy->qlen--;
		    UnlockDisplay(dpy);
		    return True;
		}
	    }
	    if (n) _XEventsQueued(dpy, QueuedAfterFlush);
	}
	UnlockDisplay(dpy);
	return False;
}
