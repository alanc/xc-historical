/* $XConsortium: Threads.c,v 1.12 93/09/15 15:09:33 kaleb Exp $ */

/************************************************************
Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include "IntrinsicI.h"

#ifdef XTHREADS

#define xmalloc XtMalloc
#define xfree XtFree
#include <X11/Xthreads.h>

#define NDEBUG
#include <assert.h>

typedef struct _LockRec {
    xmutex_t mutex;
    int level;
#ifndef _XMUTEX_NESTS
    xthread_t holder;
    xcondition_t cond;
#endif
} LockRec;

typedef struct _ThreadStack {
    unsigned int size;
    int sp;
    struct _Tstack {
	xthread_t t;
	xcondition_t c;
    } *st;
} ThreadStack;

#define STACK_INCR 16

static LockPtr process_lock = NULL;

static void
InitProcessLock()
{
    if(!process_lock) {
    	process_lock = XtNew(LockRec);
    	process_lock->mutex = xmutex_malloc();
    	xmutex_init(process_lock->mutex);
    	process_lock->level = 0;
#ifndef _XMUTEX_NESTS
    	process_lock->cond = xcondition_malloc();
    	xcondition_init(process_lock->cond);
    	xthread_clear_id(process_lock->holder);
#endif
    }
}

static void 
ProcessLock()
{
#ifdef _XMUTEX_NESTS
    xmutex_lock(process_lock->mutex);
    process_lock->level++;
#else
    xthread_t this_thread = xthread_self();
    
    xmutex_lock(process_lock->mutex);
    
    if (!xthread_have_id(process_lock->holder)) {
	process_lock->holder = this_thread;
	xmutex_unlock(process_lock->mutex);
	return;
    }
    
    if (xthread_equal(process_lock->holder,this_thread)) {
	process_lock->level++;
	xmutex_unlock(process_lock->mutex);
	return;
    }
    
    while(xthread_have_id(process_lock->holder))
	xcondition_wait(process_lock->cond, process_lock->mutex);
    
    process_lock->holder = this_thread;
    assert(xthread_equal(process_lock->holder, this_thread));
    xmutex_unlock(process_lock->mutex);
#endif
}

static void 
ProcessUnlock()
{
#ifdef _XMUTEX_NESTS
    process_lock->level--;
    xmutex_unlock(process_lock->mutex);
#else
    xmutex_lock(process_lock->mutex);
    assert(xthread_equal(process_lock->holder, ThrSelf()));
    if (process_lock->level != 0) {
	process_lock->level--;
	xmutex_unlock(process_lock->mutex);
	return;
    }
    
    xthread_clear_id(process_lock->holder);
    xcondition_signal(process_lock->cond);
    
    xmutex_unlock(process_lock->mutex);
#endif
}


static void
#if NeedFunctionPrototypes
AppLock(
    XtAppContext app)
#else
AppLock(app)
    XtAppContext app;
#endif
{
    LockPtr app_lock = app->lock_info;
#ifdef _XMUTEX_NESTS
    xmutex_lock(app_lock->mutex);
    app_lock->level++;
#else
    xthread_t this_thread = xthread_self();
    
    xmutex_lock(app_lock->mutex);
    
    if (!xthread_have_id(app_lock->holder)) {
	app_lock->holder = this_thread;
    	assert(xthread_equal(app_lock->holder, this_thread));
	xmutex_unlock(app_lock->mutex);
	return;
    }
    
    if (xthread_equal(app_lock->holder, this_thread)) {
	app_lock->level++;
	xmutex_unlock(app_lock->mutex);
	return;
    }
    
    while(xthread_have_id(app_lock->holder))
	xcondition_wait(app_lock->cond, app_lock->mutex);
    
    app_lock->holder = this_thread;
    assert(xthread_equal(app_lock->holder, this_thread));
    
    xmutex_unlock(app_lock->mutex);
#endif
}

static void
#if NeedFunctionPrototypes
AppUnlock(
    XtAppContext app)
#else
AppUnlock(app)
    XtAppContext app;
#endif
{
    LockPtr app_lock = app->lock_info;

#ifdef _XMUTEX_NESTS
    app_lock->level--;
    xmutex_unlock(app_lock->mutex);
#else
    xmutex_lock(app_lock->mutex);
    assert(xthread_equal(app_lock->holder, xthread_self()));
    if (app_lock->level != 0) {
	app_lock->level--;
	xmutex_unlock(app_lock->mutex);
	return;
    }

    xthread_clear_id(app_lock->holder);
    xcondition_signal(app_lock->cond);

    xmutex_unlock(app_lock->mutex);
#endif
}

static int
#if NeedFunctionPrototypes
YieldAppLock(
    XtAppContext app)
#else
YieldAppLock(app)
    XtAppContext app;
#endif
{
    LockPtr app_lock = app->lock_info;
    int level;
#ifdef _XMUTEX_NESTS
    level = app_lock->level;
    while (app_lock->level) {
	app_lock->level--;
	xmutex_unlock(app_lock->mutex);
    }
#else
    xthread_t this_thread = xthread_self();
    
    xmutex_lock(app_lock->mutex);

    assert(xthread_equal(app_lock->holder, this_thread));

    level = app_lock->level;

    app_lock->level = 0;
    xthread_clear_id(app_lock->holder);

    xcondition_signal(app_lock->cond);
    xmutex_unlock(app_lock->mutex);
#endif
    return level;
}


static void
#if NeedFunctionPrototypes
RestoreAppLock(
    XtAppContext app, 
    int level)
#else
RestoreAppLock(app, level)
    XtAppContext app;
    int level;
#endif
{
    LockPtr app_lock = app->lock_info;
#ifdef _XMUTEX_NESTS
    while (--level >= 0) {
	xmutex_lock(app_lock->mutex);
	app_lock->level++;
    }
#else
    xthread_t this_thread = xthread_self();

    xmutex_lock(app_lock->mutex);
    while(xthread_have_id(app_lock->holder))
	xcondition_wait(app_lock->cond, app_lock->mutex);

    app_lock->holder = this_thread;
    app_lock->level = level;
    assert(xthread_equal(app_lock->holder, this_thread)); 

    xmutex_unlock(app_lock->mutex);
#endif
}

static void
#if NeedFunctionPrototypes
FreeAppLock(
    XtAppContext app)
#else
FreeAppLock(app)
    XtAppContext app;
#endif
{
    int ii;
    LockPtr app_lock = app->lock_info;

    if(app_lock) {
	xmutex_clear(app_lock->mutex);
	xmutex_free(app_lock->mutex);
#ifndef _XMUTEX_NESTS
	xcondition_clear(app_lock->cond);
	xcondition_free(app_lock->cond);
#endif
	XtFree((char *)app_lock);
	app->lock_info = NULL;
	if(app->stack->st != (struct _Tstack *)NULL) {
	    for (ii = 0; ii < app->stack->size; ii++) {
		xcondition_clear(app->stack->st[ii].c);
		xcondition_free(app->stack->st[ii].c);
	    }
	    XtFree((char *)app->stack->st);
	}
	if(app->stack != (ThreadStackPtr)NULL)
	    XtFree((char *)app->stack);
	app->stack = NULL;
    }
}

/*
 * When this function is called, the app lock must be held 
 */

static void 
#if NeedFunctionPrototypes
PushThread(
    XtAppContext app)
#else
PushThread(app)
    XtAppContext app;
#endif
{
    unsigned int ii;
    ThreadStackPtr s = app->stack;

    assert(xthread_equal((app->lock_info->holder), (xthread_self()) ) );
    if(s->sp == s->size - 1) {
	s->st = (struct _Tstack *) 
	    XtRealloc ((char *)s->st, 
		(s->size + STACK_INCR) * sizeof (struct _Tstack));
	ii = s->size;
	s->size += STACK_INCR;
	for ( ; ii < s->size; ii++) {
	    s->st[ii].c = xcondition_malloc();
	    xcondition_init(s->st[ii].c);
	}
    }
    s->st[++(s->sp)].t = xthread_self();
}

/*
 * When this function is called, the app lock must be held 
 */

static void
#if NeedFunctionPrototypes
PopThread(
    XtAppContext app)
#else
PopThread(app)
    XtAppContext app;
#endif
{
    assert(xthread_equal((app->lock_info->holder), (xthread_self()) ) );
    assert(app->stack->sp >= 0);

    (app->stack->sp)--;
    if (app->stack->sp >= 0)
	xcondition_signal (app->stack->st[app->stack->sp].c);
}

/*
 * When this function is called, the app lock must be held 
 */

static Boolean
#if NeedFunctionPrototypes
IsTopThread(
    XtAppContext app)
#else
IsTopThread(app)
    XtAppContext app;
#endif
{
    LockPtr app_lock = app->lock_info;
    Boolean retval;

    xmutex_lock(app_lock->mutex);
    assert(xthread_equal((app->lock_info->holder), (xthread_self()) ) );
    assert(app->stack->sp >= 0);
    retval = (xthread_equal( (app->stack->st[app->stack->sp].t), 
			(xthread_self()) )) ? TRUE : FALSE;
    xmutex_unlock(app_lock->mutex);
    return retval;
}

static void
#if NeedFunctionPrototypes
WaitThread(
    XtAppContext app)
#else
WaitThread(app)
    XtAppContext app;
#endif
{
    int ii;
    LockPtr app_lock = app->lock_info;
    xthread_t self = xthread_self();

    xmutex_lock(app_lock->mutex);
    for (ii = 0; ii < app->stack->sp; ii++)
	if (xthread_equal (app->stack->st[ii].t, self)) {
	    xcondition_wait (app->stack->st[ii].c, app_lock->mutex);
	    break;
	}
    xmutex_unlock(app_lock->mutex);
}

static void
#if NeedFunctionPrototypes
InitAppLock(
    XtAppContext app)
#else
InitAppLock(app)
    XtAppContext app;
#endif
{
    int ii;

    app->lock = AppLock;
    app->unlock = AppUnlock;
    app->yield_lock = YieldAppLock;
    app->restore_lock = RestoreAppLock;
    app->free_lock = FreeAppLock;

    app->push_thread = PushThread;
    app->pop_thread = PopThread;
    app->is_top_thread = IsTopThread;
    app->wait_thread = WaitThread;

    app->lock_info = XtNew(LockRec);
    app->lock_info->mutex = xmutex_malloc();
    xmutex_init(app->lock_info->mutex);
    app->lock_info->level = 0;
#ifndef _XMUTEX_NESTS
    app->lock_info->cond = xcondition_malloc();
    xcondition_init(app->lock_info->cond);
    xthread_clear_id(app->lock_info->holder);
#endif   
    app->stack = XtNew(ThreadStack);
    app->stack->size = STACK_INCR;
    app->stack->sp = -1;
    app->stack->st = 
	(struct _Tstack *)XtMalloc(sizeof(struct _Tstack)*STACK_INCR);
    for (ii = 0; ii < STACK_INCR; ii++) {
	app->stack->st[ii].c = xcondition_malloc();
	xcondition_init(app->stack->st[ii].c);
    }
}

#endif /* defined(XTHREADS) */

#if NeedFunctionPrototypes
void XtAppLock(
    XtAppContext app)
#else
void XtAppLock(app)
    XtAppContext app;
#endif
{
#ifdef XTHREADS
    if(app->lock)
	(*app->lock)(app);
#endif
}

#if NeedFunctionPrototypes
void XtAppUnlock(
    XtAppContext app)
#else
void XtAppUnlock(app)
    XtAppContext app;
#endif
{
#ifdef XTHREADS
    if(app->unlock)
	(*app->unlock)(app);
#endif
}

void XtProcessLock()
{
#ifdef XTHREADS
    if(_XtProcessLock)
	(*_XtProcessLock)();
#endif
}

void XtProcessUnlock()
{
#ifdef XTHREADS
    if(_XtProcessUnlock)
	(*_XtProcessUnlock)();
#endif
}

Boolean XtToolkitThreadInitialize()
{
#ifdef XTHREADS
    if (_XtProcessLock == NULL) {
#ifdef xthread_init
	xthread_init();
#endif
	InitProcessLock();
	_XtProcessLock = ProcessLock;
	_XtProcessUnlock = ProcessUnlock;
	_XtInitAppLock = InitAppLock;
    }
    return True;
#else
    return False;
#endif
}
