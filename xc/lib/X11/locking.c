/*
 * $XConsortium: locking.c,v 1.1 93/06/21 17:27:07 gildea Exp $
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
#include <stdio.h>		/* for debugging stuff */
#include "locking.h"

 
/* in XOpenDis.c */
extern int  (*_XInitDisplayLock_fn)();
extern void (*_XFreeDisplayLock_fn)();



static mutex_t _Xglobal_lock;

Status XInitThreads()
{
    cthread_init();		/* return value? */
    _Xglobal_lock = mutex_alloc();
    if (_Xglobal_lock == NULL)
	return 1;
    _XLockMutex_fn = _XLockMutex;
    _XUnlockMutex_fn = _XUnlockMutex;
    _XInitDisplayLock_fn = _XInitDisplayLock;
    _XFreeDisplayLock_fn = _XFreeDisplayLock;

#ifdef TDEBUG
    setlinebuf(stdout);		/* for debugging messages */
#endif

    return 0;
}


void _XLockMutex()
{
    mutex_lock(_Xglobal_lock);
}

void _XUnlockMutex()
{
    mutex_unlock(_Xglobal_lock);
}

static char *locking_file;
static int locking_line;
static cthread_t locking_thread = NULL;
static Bool xlibint_unlock = False; /* XlibInt.c may Unlock and re-Lock */

void _XLockDisplay(dpy,file,line)
    Display *dpy;
    char *file;			/* source file, from macro */
    int line;
{
    cthread_t self = cthread_self();
    cthread_t old_locker = locking_thread;

    if (old_locker) {
	if (old_locker == self)
	    printf("Xlib ERROR: %s line %d thread %x: locking display already locked at %s line %d\n",
		   file, line, cthread_self(), locking_file, locking_line);
#ifdef TDEBUG
	else
	    printf("%s line %d: thread %x waiting on lock held by %s line %d thread %x\n",
		   file, line, self,
		   locking_file, locking_line, old_locker);
#endif
    }

    mutex_lock(dpy->lock->mutex);

    if (strcmp(file, "XlibInt.c") == 0) {
	if (!xlibint_unlock)
	    printf("Xlib ERROR: XlibInt.c line %d thread %x locking display it did not unlock\n",
		   line, self);
	xlibint_unlock = False;
    }

#ifdef TDEBUG
    /* if (old_locker  &&  old_locker != self) */
    if (strcmp("XClearArea.c", file) && strcmp("XDrSegs.c", file)) /* ico */
	printf("%s line %d: thread %x got display lock\n", file, line, self);
#endif

    locking_thread = self;
    if (strcmp(file, "XlibInt.c") != 0) {
	locking_file = file;
	locking_line = line;
    }
}

static void _XUnlockDisplay(dpy,file,line)
    Display *dpy;
    char *file;
    int line;
{
    cthread_t self = cthread_self();

#ifdef TDEBUG
    if (strcmp("XClearArea.c", file) && strcmp("XDrSegs.c", file)) /* ico */
	printf("%s line %d: thread %x unlocking display\n", file, line, self);
#endif

    if (!locking_thread)
	printf("Xlib ERROR: %s line %d thread %x: unlocking display that is not locked\n",
	       file, line, self);
    else if (strcmp(file, "XlibInt.c") == 0)
	xlibint_unlock = True;
    else if (strcmp(file, locking_file) != 0)
	/* not always an error because locking_file is not per-thread */
	printf("%s line %d: unlocking display locked from a different file: %s line %d\n",
	       file, line, locking_file, locking_line);
    locking_thread = NULL;
    mutex_unlock(dpy->lock->mutex);
}


/* Put ourselves on the queue to read the connection.
   Allocates and returns a queue element. */

struct _XCVList *
_XPushReader(tail)
    struct _XCVList ***tail;
{
    struct _XCVList *cvl;

    cvl = (struct _XCVList *)Xmalloc(sizeof(struct _XCVList));
    cvl->cv = condition_alloc();
    cvl->next = NULL;
#ifdef TDEBUG
    printf("_XPushReader called in thread %x, pushing %x\n",
	   cthread_self(), cvl);
#endif
    **tail = cvl;
    *tail = &cvl->next;
    return cvl;
}

/* signal the next thread waiting to read the connection */

void _XPopReader(dpy, list, tail)
    Display *dpy;
    struct _XCVList **list;
    struct _XCVList ***tail;
{
    register struct _XCVList *front = *list;

#ifdef TDEBUG
    printf("_XPopReader called in thread %x, popping %x\n",
	   cthread_self(), front);
#endif

    *list = front->next;
    if (*tail == &front->next)	/* did we free the last elt? */
	*tail = list;
    condition_free(front->cv);
    Xfree(front);

    /* signal new front after it is in place */
    if (dpy->lock->reply_awaiters) {
	ConditionSignal(dpy, dpy->lock->reply_awaiters);
    } else if (dpy->lock->event_awaiters) {
	ConditionSignal(dpy, dpy->lock->event_awaiters);
    }
}


void _XConditionWait(cv, mutex,file,line)
    condition_t cv;
    mutex_t mutex;
    char *file;
    int line;
{
    cthread_t self = cthread_self();
    char *old_file = locking_file;
    int old_line = locking_line;
    
#ifdef TDEBUG
    printf("line %d thread %x in condition wait\n", line, self);
#endif
    locking_thread = NULL;
    condition_wait(cv, mutex);
    locking_thread = self;
    locking_file = old_file;
    locking_line = old_line;
#ifdef TDEBUG
    printf("line %d thread %x was signaled\n", line, self);
#endif
}

void _XConditionSignal(cv,file,line)
    condition_t cv;
    char *file;
    int line;
{
#ifdef TDEBUG
    cthread_t self = cthread_self();
    printf("line %d thread %x is signalling\n", line, self);
#endif
    condition_signal(cv);
}
    

/* returns 0 if initialized ok, -1 if unable to allocate
   a mutex or other memory */

int _XInitDisplayLock(dpy)
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
    dpy->lock->locking_thread = 0;
    dpy->lock->cv = NULL;

    dpy->lock->mutex = mutex_alloc();

    if (dpy->lock->mutex==NULL)
	return -1;

    dpy->lock_fns->lock_display = _XLockDisplay;
    dpy->lock_fns->unlock_display = _XUnlockDisplay;
    dpy->lock_fns->pop_reader = _XPopReader;
    dpy->lock_fns->push_reader = _XPushReader;
    dpy->lock_fns->condition_wait = _XConditionWait;
    dpy->lock_fns->condition_signal = _XConditionSignal;
    dpy->lock_fns->lock_wait = NULL; /* filled in by XLockDisplay() */

    return 0;
}

void _XFreeDisplayLock(dpy)
    Display *dpy;
{
    if (dpy->lock != NULL) {
	if (dpy->lock->mutex != NULL)
	    mutex_free(dpy->lock->mutex);
	if (dpy->lock->cv != NULL)
	    condition_free(dpy->lock->cv);
	Xfree(dpy->lock);
	dpy->lock = NULL;
    }
    if (dpy->lock_fns != NULL) {
	Xfree(dpy->lock_fns);
	dpy->lock_fns = NULL;
    }
}

#else /* XTHREADS */
static int unused;		/* always create a symbol table */
#endif /* XTHREADS */
