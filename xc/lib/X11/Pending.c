#include "copyright.h"

/* $Header: XPending.c,v 11.10 87/09/11 08:05:32 toddb Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

/* Read in any pending events and return the number of queued events. */

int XEventsQueued (dpy, mode)
    register Display *dpy;
    int mode;
{
    int ret_val;
    LockDisplay(dpy);
    ret_val = _XEventsQueued (dpy, mode);
    UnlockDisplay(dpy);
    return ret_val;
}

int XPending (dpy)
    register Display *dpy;
{
    int ret_val;
    LockDisplay(dpy);
    ret_val = _XEventsQueued (dpy, QueuedAfterFlush);
    UnlockDisplay(dpy);
    return ret_val;
}
