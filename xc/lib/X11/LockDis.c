/*
 * $XConsortium: LockDis.c,v 1.6 93/07/22 13:30:47 gildea Exp $
 *
 * Copyright 1993 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Stephen Gildea, MIT X Consortium
 *
 * XLockDis.c - multi-thread application-level locking routines
 */

#include "Xlibint.h"
#ifdef XTHREADS
#include "locking.h"
#endif

#if NeedFunctionPrototypes
void XLockDisplay(
    register Display* dpy)
#else
void XLockDisplay(dpy)
    register Display* dpy;
#endif
{
#ifdef XTHREADS
    LockDisplay(dpy);
    if (dpy->lock)
	(*dpy->lock_fns->user_lock_display)(dpy);
    /*
     * We want the threads in the reply queue to all get out before
     * XLockDisplay returns, in case they have any side effects the
     * caller of XLockDisplay was trying to protect against.
     * XLockDisplay puts itself at the head of the event waiters queue
     * to wait for all the replies to come in.
     */
    if (dpy->lock && dpy->lock->reply_awaiters) {
	struct _XCVList *cvl;

	cvl = (*_XCreateCVL_fn)();

	/* stuff ourselves on the head of the queue */
	cvl->next = dpy->lock->event_awaiters;
	dpy->lock->event_awaiters = cvl;

	ConditionWait(dpy, cvl);
	UnlockNextEventReader(dpy, cvl); /* pass the signal on */
    }
    UnlockDisplay(dpy);
#endif
}

#if NeedFunctionPrototypes
void XUnlockDisplay(
    register Display* dpy)
#else
void XUnlockDisplay(dpy)
    register Display* dpy;
#endif
{
#ifdef XTHREADS
    LockDisplay(dpy);
    if (dpy->lock)
	(*dpy->lock_fns->user_unlock_display)(dpy);
    UnlockDisplay(dpy);
#endif
}
