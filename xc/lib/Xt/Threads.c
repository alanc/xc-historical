/* $XConsortium: Threads.c,v 1.14 93/11/22 13:19:14 kaleb Exp $ */

/************************************************************
Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the name Sun not be used in advertising 
or publicity pertaining to distribution  of  the software  
without specific prior written permission. Sun makes no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/*

Copyright (c) 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#include "IntrinsicI.h"

#ifdef XTHREADS

#define xmalloc XtMalloc
#define xfree XtFree
#include <X11/Xthreads.h>

#define NDEBUG
#include <assert.h>

typedef struct _ThreadStack {
    unsigned int size;
    int sp;
    struct _Tstack {
	xthread_t t;
	xcondition_t c;
    } *st;
} ThreadStack;

typedef struct _LockRec {
    xmutex_t mutex;
    int level;
    ThreadStack stack;
#ifndef _XMUTEX_NESTS
    xthread_t holder;
    xcondition_t cond;
#endif
} LockRec;


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
    assert(xthread_equal(process_lock->holder, xthread_self()));
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
	if(app_lock->stack.st != (struct _Tstack *)NULL) {
	    for (ii = 0; ii < app_lock->stack.size; ii++) {
		xcondition_clear(app_lock->stack.st[ii].c);
		xcondition_free(app_lock->stack.st[ii].c);
	    }
	    XtFree((char *)app_lock->stack.st);
	}
	XtFree((char *)app_lock);
	app->lock_info = NULL;
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
    LockPtr app_lock = app->lock_info;

    xmutex_lock(app_lock->mutex);
    assert(xthread_equal((app_lock->holder), (xthread_self()) ) );
    if(app_lock->stack.sp == app_lock->stack.size - 1) {
	app_lock->stack.st = (struct _Tstack *) 
	    XtRealloc ((char *)app_lock->stack.st, 
		(app_lock->stack.size + STACK_INCR) * sizeof (struct _Tstack));
	ii = app_lock->stack.size;
	app_lock->stack.size += STACK_INCR;
	for ( ; ii < app_lock->stack.size; ii++) {
	    app_lock->stack.st[ii].c = xcondition_malloc();
	    xcondition_init(app_lock->stack.st[ii].c);
	}
    }
    app_lock->stack.st[++(app_lock->stack.sp)].t = xthread_self();
    xmutex_unlock(app_lock->mutex);
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
    LockPtr app_lock = app->lock_info;

    xmutex_lock(app_lock->mutex);
    assert(xthread_equal((app_lock->holder), (xthread_self()) ) );
    assert(app_lock->stack.sp >= 0);

    (app_lock->stack.sp)--;
    if (app_lock->stack.sp >= 0)
	xcondition_signal (app_lock->stack.st[app_lock->stack.sp].c);
    xmutex_unlock(app_lock->mutex);
}

static Boolean
#if NeedFunctionPrototypes
IsTopThread(
    XtAppContext app)
#else
IsTopThread(app)
    XtAppContext app;
#endif
{
    Boolean retval;
    LockPtr app_lock = app->lock_info;
    xthread_t self = xthread_self();

    xmutex_lock(app_lock->mutex);
    assert(app_lock->stack.sp >= 0);
    if (xthread_equal(app_lock->stack.st[app_lock->stack.sp].t, self))
	retval = TRUE;
    else {
	int ii;
	retval = FALSE;
	for (ii = app_lock->stack.sp - 1; ii >= 0; ii--)
	    if (xthread_equal(app_lock->stack.st[ii].t, self)) {
		xcondition_wait(app_lock->stack.st[ii].c, app_lock->mutex);
		break;
	    }
    }
    xmutex_unlock(app_lock->mutex);
    return retval;
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
    LockPtr app_lock;

    app->lock = AppLock;
    app->unlock = AppUnlock;
    app->yield_lock = YieldAppLock;
    app->restore_lock = RestoreAppLock;
    app->free_lock = FreeAppLock;

    app->push_thread = PushThread;
    app->pop_thread = PopThread;
    app->is_top_thread = IsTopThread;

    app_lock = app->lock_info = XtNew(LockRec);
    app_lock->mutex = xmutex_malloc();
    xmutex_init(app_lock->mutex);
    app_lock->level = 0;
#ifndef _XMUTEX_NESTS
    app_lock->cond = xcondition_malloc();
    xcondition_init(app_lock->cond);
    xthread_clear_id(app_lock->holder);
#endif   
    app_lock->stack.size = STACK_INCR;
    app_lock->stack.sp = -1;
    app_lock->stack.st = 
	(struct _Tstack *)XtMalloc(sizeof(struct _Tstack)*STACK_INCR);
    for (ii = 0; ii < STACK_INCR; ii++) {
	app_lock->stack.st[ii].c = xcondition_malloc();
	xcondition_init(app_lock->stack.st[ii].c);
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
