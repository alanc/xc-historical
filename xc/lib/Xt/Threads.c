/* $XConsortium: Threads.c,v 1.0 93/08/19 08:41:03 kaleb Exp $ */

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

#if defined(XTHREADS)
#define NDEBUG
#include <assert.h>

#include "ThreadsI.h"

static LockPtr process_lock = NULL;

ProcessLockProc _XtProcessLock = NULL;
ProcessUnlockProc _XtProcessUnlock = NULL;
InitAppLockProc _XtInitAppLock = NULL;

static void
InitProcessLock()
{
    if(!process_lock) {
    	process_lock = XtNew(LockRec);
    	process_lock->mutex = NewMutex();
    	process_lock->cond = NewCond();

    	MutexInit(process_lock->mutex);
    	CondInit(process_lock->cond);

    	process_lock->recursion = 0;
    	process_lock->holder = _XT_NO_THREAD_ID;
    }
}

static void 
ProcessLock()
{
    ThreadId this_thread = ThrSelf();
    
    MutexLock(process_lock->mutex);
    
    if (SameThrId(process_lock->holder, _XT_NO_THREAD_ID)) {
	process_lock->holder = this_thread;
	MutexUnlock(process_lock->mutex);
	return;
    }
    
    if (SameThrId(process_lock->holder,this_thread)) {
	process_lock->recursion++;
	MutexUnlock(process_lock->mutex);
	return;
    }
    
    while(!SameThrId(process_lock->holder, _XT_NO_THREAD_ID))  
	CondWait(process_lock->cond, process_lock->mutex);
    
    process_lock->holder = this_thread;
    assert(SameThrId(process_lock->holder, this_thread));
    MutexUnlock(process_lock->mutex);
}

static void 
ProcessUnlock()
{
    MutexLock(process_lock->mutex);
    assert(SameThrId(process_lock->holder, ThrSelf()));
    if (process_lock->recursion != 0) {
	process_lock->recursion--;
	MutexUnlock(process_lock->mutex);
	return;
    }
    
    process_lock->holder = _XT_NO_THREAD_ID;
    CondSignal(process_lock->cond);
    
    MutexUnlock(process_lock->mutex);
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
    ThreadId this_thread = ThrSelf();
    
    MutexLock(app_lock->mutex);
    
    if (SameThrId(app_lock->holder, _XT_NO_THREAD_ID)) {
	app_lock->holder = this_thread;
    	assert(SameThrId(app_lock->holder, this_thread));
	MutexUnlock(app_lock->mutex);
	return;
    }
    
    if (SameThrId(app_lock->holder, this_thread)) {
	app_lock->recursion++;
	MutexUnlock(app_lock->mutex);
	return;
    }
    
    while(!SameThrId(app_lock->holder, _XT_NO_THREAD_ID))  
	CondWait(app_lock->cond, app_lock->mutex);
    
    app_lock->holder = this_thread;
    assert(SameThrId(app_lock->holder, this_thread));
    
    MutexUnlock(app_lock->mutex);
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

    MutexLock(app_lock->mutex);
    assert(SameThrId(app_lock->holder, ThrSelf()));
    if (app_lock->recursion != 0) {
	app_lock->recursion--;
	MutexUnlock(app_lock->mutex);
	return;
    }
    
    app_lock->holder = _XT_NO_THREAD_ID;
    CondSignal(app_lock->cond);
    
    MutexUnlock(app_lock->mutex);
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
    int i =0;
    int r = -1;
    ThreadId this_thread = ThrSelf();
    
    MutexLock(app_lock->mutex);

    assert(SameThrId(app_lock->holder, this_thread));

    r = app_lock->recursion;

    app_lock->recursion = 0;
    app_lock->holder = _XT_NO_THREAD_ID;

    CondSignal(app_lock->cond);
    MutexUnlock(app_lock->mutex);
    return r;
}


static void
#if NeedFunctionPrototypes
RestoreAppLock(
    XtAppContext app, 
    int r)
#else
RestoreAppLock(app, r)
    XtAppContext app;
    int r;
#endif
{
    ThreadId this_thread = ThrSelf();
    LockPtr app_lock = app->lock_info;

    MutexLock(app_lock->mutex);
    while(!SameThrId(app_lock->holder, _XT_NO_THREAD_ID))  
	CondWait(app_lock->cond, app_lock->mutex);

    app_lock->holder = this_thread;
    app_lock->recursion = r;
    assert(SameThrId(app_lock->holder, this_thread)); 

    MutexUnlock(app_lock->mutex);
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
    LockPtr app_lock = app->lock_info;

    if(app_lock) {
	DestroyMutex(app_lock->mutex);
	DeleteMutex(app_lock->mutex);

	DestroyCond(app_lock->cond);
	DeleteCond(app_lock->cond);

	XtFree((char *)app_lock);
	app->lock_info = NULL;
	if(app->stack->p != (ThreadId *)NULL)
	    XtFree((char *)app->stack->p);
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
    ThreadStackPtr s = app->stack;

    assert( SameThrId((app->lock_info->holder), (ThrSelf()) ) );
    if(s->sp == s->size - 1) 
	s->p = (ThreadId *) XtRealloc((char *)s->p, (s->size += STACK_INCR));
    s->p[++(s->sp)] = ThrSelf();
}

/*
 * When this function is called, the app lock must be held 
 */

static void
#if NeeddFunctionPrototypes
PopThread(
    XtAppContext app)
#else
PopThread(app)
    XtAppContext app;
#endif
{
    assert( SameThrId((app->lock_info->holder), (ThrSelf()) ) );
    assert(app->stack->sp >= 0);

    (app->stack->sp)--;
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
    assert( SameThrId((app->lock_info->holder), (ThrSelf()) ) );
    assert(app->stack->sp >= 0);

    return ( (SameThrId( (app->stack->p[app->stack->sp]), 
			(ThrSelf()) )) ? TRUE : FALSE);
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
    app->lock = AppLock;
    app->unlock = AppUnlock;
    app->yield_lock = YieldAppLock;
    app->restore_lock = RestoreAppLock;
    app->free_lock = FreeAppLock;

    app->push_thread = PushThread;
    app->pop_thread = PopThread;
    app->is_top_thread = IsTopThread;

    app->lock_info = XtNew(LockRec);
    app->lock_info->mutex = NewMutex();
    app->lock_info->cond = NewCond();
    MutexInit(app->lock_info->mutex);
    CondInit(app->lock_info->cond);
    app->lock_info->recursion = 0;
    app->lock_info->holder = _XT_NO_THREAD_ID;
   
    app->stack = XtNew(ThreadStack);
    app->stack->size = STACK_INCR;
    app->stack->sp = -1;
    app->stack->p = (ThreadId *)XtMalloc(sizeof(ThreadId)*STACK_INCR);
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
#if defined(XTHREADS)
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
#if defined(XTHREADS)
    if(app->unlock)
	(*app->unlock)(app);
#endif
}

void XtProcessLock()
{
#if defined(XTHREADS)
    if(_XtProcessLock)
	(*_XtProcessLock)();
#endif
}

void XtProcessUnlock()
{
#if defined(XTHREADS)
    if(_XtProcessUnlock)
	(*_XtProcessUnlock)();
#endif
}

void XtToolkitThreadInitialize()
{
#if defined(XTHREADS)
    InitProcessLock();
    _XtProcessLock = ProcessLock;
    _XtProcessUnlock = ProcessUnlock;
    _XtInitAppLock = InitAppLock;
#endif
}
