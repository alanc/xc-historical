/*
 * $XConsortium: locking.c,v 1.5 93/07/11 13:39:57 rws Exp $
 *
 * Copyright 1992 Massachusetts Institute of Technology
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
 * locking.c - multi-thread locking routines implemented in C Threads
 */

#ifdef XTHREADS

#include "Xlibint.h"
#include "locking.h"
#ifdef XTHREADS_WARN
#include <stdio.h>		/* for warn/debug stuff */
#endif

/* in XOpenDis.c */
extern int  (*_XInitDisplayLock_fn)();
extern void (*_XFreeDisplayLock_fn)();

#ifdef _DECTHREADS_
xthread_t _X_no_thread_id;
#endif

static mutex_t _Xglobal_lock;

static void _XLockMutex()
{
    mutex_lock(_Xglobal_lock);
}

static void _XUnlockMutex()
{
    mutex_unlock(_Xglobal_lock);
}

#ifdef XTHREADS_WARN
static char *locking_file;
static int locking_line;
static xthread_t locking_thread;
static Bool xlibint_unlock = False; /* XlibInt.c may Unlock and re-Lock */

/* history that is useful to examine in a debugger */
#define LOCK_HIST_SIZE 21

static struct {
    Bool lockp;			/* True for lock, False for unlock */
    xthread_t thread;
    char *file;
    int line;
} locking_history[LOCK_HIST_SIZE];

int lock_hist_loc = 0;		/* next slot to fill */
#endif /* XTHREADS_WARN */

#ifdef XTHREADS_WARN
void _XLockDisplay(dpy,file,line)
    Display *dpy;
    char *file;			/* source file, from macro */
    int line;
#else
void _XLockDisplay(dpy)
    Display *dpy;
#endif
{
#ifdef XTHREADS_WARN
    xthread_t self;
    xthread_t old_locker;

    self = xthread_self();
    old_locker = locking_thread;
    if (xthread_have_id(old_locker)) {
	if (xthread_equal(old_locker, self))
	    printf("Xlib ERROR: %s line %d thread %x: locking display already locked at %s line %d\n",
		   file, line, self, locking_file, locking_line);
#ifdef XTHREADS_DEBUG
	else
	    printf("%s line %d: thread %x waiting on lock held by %s line %d thread %x\n",
		   file, line, self,
		   locking_file, locking_line, old_locker);
#endif /* XTHREADS_DEBUG */
    }
#endif /* XTHREADS_WARN */

    mutex_lock(dpy->lock->mutex);

#ifdef XTHREADS_WARN
    if (strcmp(file, "XlibInt.c") == 0) {
	if (!xlibint_unlock)
	    printf("Xlib ERROR: XlibInt.c line %d thread %x locking display it did not unlock\n",
		   line, self);
	xlibint_unlock = False;
    }

#ifdef XTHREADS_DEBUG
    /* if (old_locker  &&  old_locker != self) */
    if (strcmp("XClearArea.c", file) && strcmp("XDrSegs.c", file)) /* ico */
	printf("%s line %d: thread %x got display lock\n", file, line, self);
#endif /* XTHREADS_DEBUG */

    locking_thread = self;
    if (strcmp(file, "XlibInt.c") != 0) {
	locking_file = file;
	locking_line = line;
    }
    locking_history[lock_hist_loc].file = file;
    locking_history[lock_hist_loc].line = line;
    locking_history[lock_hist_loc].thread = self;
    locking_history[lock_hist_loc].lockp = True;
    lock_hist_loc++;
    if (lock_hist_loc >= LOCK_HIST_SIZE)
	lock_hist_loc = 0;
#endif /* XTHREADS_WARN */
}

#ifdef XTHREADS_WARN
static void _XUnlockDisplay(dpy,file,line)
    Display *dpy;
    char *file;
    int line;
#else
static void _XUnlockDisplay(dpy)
    Display *dpy;
#endif
{
#ifdef XTHREADS_WARN
    xthread_t self = xthread_self();

#ifdef XTHREADS_DEBUG
    if (strcmp("XClearArea.c", file) && strcmp("XDrSegs.c", file)) /* ico */
	printf("%s line %d: thread %x unlocking display\n", file, line, self);
#endif /* XTHREADS_DEBUG */

    if (!xthread_have_id(locking_thread))
	printf("Xlib ERROR: %s line %d thread %x: unlocking display that is not locked\n",
	       file, line, self);
    else if (strcmp(file, "XlibInt.c") == 0)
	xlibint_unlock = True;
#ifdef XTHREADS_DEBUG
    else if (strcmp(file, locking_file) != 0)
	/* not always an error because locking_file is not per-thread */
	printf("%s line %d: unlocking display locked from %s line %d (probably okay)\n",
	       file, line, locking_file, locking_line);
#endif /* XTHREADS_DEBUG */
    xthread_clear_id(locking_thread);

    locking_history[lock_hist_loc].file = file;
    locking_history[lock_hist_loc].line = line;
    locking_history[lock_hist_loc].thread = self;
    locking_history[lock_hist_loc].lockp = False;
    lock_hist_loc++;
    if (lock_hist_loc >= LOCK_HIST_SIZE)
	lock_hist_loc = 0;
#endif /* XTHREADS_WARN */
    mutex_unlock(dpy->lock->mutex);
}


/* Put ourselves on the queue to read the connection.
   Allocates and returns a queue element. */

static struct _XCVList *
_XPushReader(tail)
    struct _XCVList ***tail;
{
    struct _XCVList *cvl;

    cvl = (struct _XCVList *)Xmalloc(sizeof(struct _XCVList));
    cvl->cv = condition_malloc();
    condition_init(cvl->cv);
    cvl->next = NULL;
#ifdef XTHREADS_DEBUG
    printf("_XPushReader called in thread %x, pushing %x\n",
	   xthread_self(), cvl);
#endif
    **tail = cvl;
    *tail = &cvl->next;
    return cvl;
}

/* signal the next thread waiting to read the connection */

static void _XPopReader(dpy, list, tail)
    Display *dpy;
    struct _XCVList **list;
    struct _XCVList ***tail;
{
    register struct _XCVList *front = *list;

#ifdef XTHREADS_DEBUG
    printf("_XPopReader called in thread %x, popping %x\n",
	   xthread_self(), front);
#endif

    *list = front->next;
    if (*tail == &front->next)	/* did we free the last elt? */
	*tail = list;
    condition_clear(front->cv);
    Xfree((char *)front->cv);
    Xfree((char *)front);

    /* signal new front after it is in place */
    if (dpy->lock->reply_awaiters) {
	ConditionSignal(dpy, dpy->lock->reply_awaiters);
    } else if (dpy->lock->event_awaiters) {
	ConditionSignal(dpy, dpy->lock->event_awaiters);
    }
}

#ifdef XTHREADS_WARN
static void _XConditionWait(cv, mutex,file,line)
    condition_t cv;
    mutex_t mutex;
    char *file;
    int line;
#else
static void _XConditionWait(cv, mutex)
    condition_t cv;
    mutex_t mutex;
#endif
{
#ifdef XTHREADS_WARN
    xthread_t self = xthread_self();
    char *old_file = locking_file;
    int old_line = locking_line;
    
#ifdef XTHREADS_DEBUG
    printf("line %d thread %x in condition wait\n", line, self);
#endif
    xthread_clear_id(locking_thread);

    locking_history[lock_hist_loc].file = file;
    locking_history[lock_hist_loc].line = line;
    locking_history[lock_hist_loc].thread = self;
    locking_history[lock_hist_loc].lockp = False;
    lock_hist_loc++;
    if (lock_hist_loc >= LOCK_HIST_SIZE)
	lock_hist_loc = 0;
#endif /* XTHREADS_WARN */

    condition_wait(cv, mutex);

#ifdef XTHREADS_WARN
    locking_thread = self;
    locking_file = old_file;
    locking_line = old_line;

    locking_history[lock_hist_loc].file = file;
    locking_history[lock_hist_loc].line = line;
    locking_history[lock_hist_loc].thread = self;
    locking_history[lock_hist_loc].lockp = True;
    lock_hist_loc++;
    if (lock_hist_loc >= LOCK_HIST_SIZE)
	lock_hist_loc = 0;
#ifdef XTHREADS_DEBUG
    printf("line %d thread %x was signaled\n", line, self);
#endif /* XTHREADS_DEBUG */
#endif /* XTHREADS_WARN */
}

#ifdef XTHREADS_WARN
static void _XConditionSignal(cv,file,line)
    condition_t cv;
    char *file;
    int line;
#else
static void _XConditionSignal(cv)
    condition_t cv;
#endif
{
#ifdef XTHREADS_DEBUG
    printf("line %d thread %x is signalling\n", line, xthread_self());
#endif
    condition_signal(cv);
}
    

/* returns 0 if initialized ok, -1 if unable to allocate
   a mutex or other memory */

static int _XInitDisplayLock(dpy)
    Display *dpy;
{
    dpy->lock = (struct _XLockInfo *)Xmalloc(sizeof(struct _XLockInfo));
    if (dpy->lock == NULL)
	return -1;
    dpy->lock_fns = (struct _XLockPtrs*)Xmalloc(sizeof(struct _XLockPtrs));
    if (dpy->lock_fns == NULL)
	return -1;

    dpy->lock->reply_bytes_left = 0;
    dpy->lock->reply_was_read = False;
    dpy->lock->reply_awaiters = NULL;
    dpy->lock->reply_awaiters_tail = &dpy->lock->reply_awaiters;
    dpy->lock->event_awaiters = NULL;
    dpy->lock->event_awaiters_tail = &dpy->lock->event_awaiters;
    xthread_clear_id(dpy->lock->locking_thread);
    dpy->lock->cv = NULL;

    dpy->lock->mutex = mutex_malloc();
    if (dpy->lock->mutex==NULL)
	return -1;
    mutex_init(dpy->lock->mutex);

    dpy->lock_fns->lock_display = _XLockDisplay;
    dpy->lock_fns->unlock_display = _XUnlockDisplay;
    dpy->lock_fns->pop_reader = _XPopReader;
    dpy->lock_fns->push_reader = _XPushReader;
    dpy->lock_fns->condition_wait = _XConditionWait;
    dpy->lock_fns->condition_signal = _XConditionSignal;
    dpy->lock_fns->lock_wait = NULL; /* filled in by XLockDisplay() */

    return 0;
}

static void _XFreeDisplayLock(dpy)
    Display *dpy;
{
    if (dpy->lock != NULL) {
	if (dpy->lock->mutex != NULL) {
	    mutex_clear(dpy->lock->mutex);
	    mutex_free(dpy->lock->mutex);
	}
	if (dpy->lock->cv != NULL) {
	    condition_clear(dpy->lock->cv);
	    condition_free(dpy->lock->cv);
	}
	Xfree((char *)dpy->lock);
	dpy->lock = NULL;
    }
    if (dpy->lock_fns != NULL) {
	Xfree((char *)dpy->lock_fns);
	dpy->lock_fns = NULL;
    }
}

/*
 * wait for thread with user-level display lock to release it.
 */

static void _XDisplayLockWait(dpy)
    Display *dpy;
{
    if (xthread_have_id(dpy->lock->locking_thread) &&
	!xthread_equal(dpy->lock->locking_thread, xthread_self()))
    {
	if (!dpy->lock->cv) {
	    dpy->lock->cv = condition_malloc();
	    condition_init(dpy->lock->cv);
	}

	ConditionWait(dpy, dpy->lock);
    }
}

/*
 * version of display locking function to use when
 * a user-level lock might be active.
 */
#ifdef XTHREADS_WARN
static void _XFancyLockDisplay(dpy, file, line)
    Display *dpy;
    char *file;			/* source file, from macro */
    int line;
#else
static void _XFancyLockDisplay(dpy)
    Display *dpy;
#endif
{
#ifdef XTHREADS_WARN
    _XLockDisplay(dpy, file, line);
#else
    _XLockDisplay(dpy);
#endif
    _XDisplayLockWait(dpy);
}

#if NeedFunctionPrototypes
static void _XUserLockDisplay(
    register Display* dpy)
#else
static void _XUserLockDisplay(dpy)
    register Display* dpy;
#endif
{
    /* substitute fancier, slower lock function */
    dpy->lock_fns->lock_display = _XFancyLockDisplay;
    /* really only needs to be done once */
    dpy->lock_fns->lock_wait = _XDisplayLockWait;
    if (xthread_have_id(dpy->lock->locking_thread)) {
	/* XXX - we are in XLockDisplay, error.  Print message? */
    }
    dpy->lock->locking_thread = xthread_self();
}

#if NeedFunctionPrototypes
void _XUserUnlockDisplay(
    register Display* dpy)
#else
void _XUserUnlockDisplay(dpy)
    register Display* dpy;
#endif
{
    if (!xthread_have_id(dpy->lock->locking_thread)) {
	/* XXX - we are not in XLockDisplay, error.  Print message? */
    }
    /* signal other threads that might be waiting in XLockDisplay */
    condition_broadcast(dpy->lock->cv);
    /* substitute function back */
    dpy->lock_fns->lock_display = _XLockDisplay;
    xthread_clear_id(dpy->lock->locking_thread);
}

Status XInitThreads()
{
#ifdef xthread_init
    xthread_init();		/* return value? */
#endif
    _Xglobal_lock = mutex_malloc();
    if (_Xglobal_lock == NULL)
	return 1;
    mutex_init(_Xglobal_lock);
    _XLockMutex_fn = _XLockMutex;
    _XUnlockMutex_fn = _XUnlockMutex;
    _XInitDisplayLock_fn = _XInitDisplayLock;
    _XFreeDisplayLock_fn = _XFreeDisplayLock;

#ifdef XTHREADS_DEBUG
    setlinebuf(stdout);		/* for debugging messages */
#endif

    return 0;
}

#else /* XTHREADS */
Status XInitThreads()
{
}
#endif /* XTHREADS */
