/*
 * $XConsortium: XLockDis.c,v 1.1 93/06/21 15:35:26 gildea Exp $
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

#ifdef MULTI_THREADED

#include "Xlibint.h"
#include "locking.h"

/* prototypes - move to Xlib.h */

extern Status XLockDisplay(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);

extern Status XUnlockDisplay(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);

/* end of decls */

static void _XFancyLockDisplay();
static void _XDisplayLockWait();

#if NeedFunctionPrototypes
Status XLockDisplay(
    register Display* dpy)
#else
Status XLockDisplay(dpy)
    register Display* dpy;
#endif
{
    cthread_t self = cthread_self();

    LockDisplay(dpy);
    if (dpy->lock) {
	/* substitute fancier, slower lock function */
	dpy->lock_fns->lock_display = _XFancyLockDisplay;
	/* really only needs to be done once */
	dpy->lock_fns->lock_wait = _XDisplayLockWait;
	if (dpy->lock->locking_thread) {
	    /* XXX - we are in XLockDisplay, error.  Print message? */
	}
	dpy->lock->locking_thread = self;
    }
}

#if NeedFunctionPrototypes
Status XUnlockDisplay(
    register Display* dpy)
#else
Status XUnlockDisplay(dpy)
    register Display* dpy;
#endif
{
    if (dpy->lock) {
	if (!dpy->lock->locking_thread) {
	    /* XXX - we are not in XLockDisplay, error.  Print message? */
	}
	/* signal other threads that might be waiting in XLockDisplay */
	condition_broadcast(dpy->lock->cv);
	/* substitute function back */
	dpy->lock_fns->lock_display = _XLockDisplay;
	dpy->lock->locking_thread = 0;
    }
    UnlockDisplay(dpy);
}

/*
 * wait for thread with user-level display lock to release it.
 */

static void _XDisplayLockWait(dpy)
    Display *dpy;
{
    if (dpy->lock->locking_thread &&
	dpy->lock->locking_thread != cthread_self())
    {
	if (!dpy->lock->cv)
	    dpy->lock->cv = condition_alloc();

	ConditionWait(dpy, dpy->lock);
    }
}

/*
 * version of display locking function to use when
 * a user-level lock might be active.
 */
static void _XFancyLockDisplay(dpy, file, line)
    Display *dpy;
    char *file;			/* source file, from macro */
    int line;
{
    _XLockDisplay(dpy, file, line);
    _XDisplayLockWait(dpy);
}

#else /* MULTI_THREADED */
static int unused;		/* always create a symbol table */
#endif /* MULTI_THREADED */
