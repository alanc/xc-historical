/* $XConsortium: ThreadsI.h,v 1.00 93/08/17 12:07:18 kaleb Exp $ */

#ifndef _X11_THREADSI_H 
#define _X11_THREADSI_H 
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

#if defined(XTHREADS)

#if defined(CTHREADS)
#include <cthreads.h>
typedef cthread_t ThreadId;
typedef mutex_t Mutex;
typedef condition_t Cond;
#define MutexLock(m) mutex_lock(m)
#define MutexUnlock(m) mutex_unlock(m)
#define MutexInit(m) mutex_init(m)
#define CondInit(c) condition_init(c)
#define CondWait(c,m) condition_wait(c,m)
#define CondSignal(c) condition_signal(c)
#define NewMutex() XtNew(mutex_t)
#define NewCond() XtNew(condition_t)
#define ThrSelf() cthread_self()
#define SameThrId(t1,t2) t1 == t2 /* XXX */
#define DestroyMutex(m) mutex_clear(m)
#define DestroyCond(c) condition_clear(c)
#define DeleteMutex(m) XtFree((char *)m)
#define DeleteCond(c) XtFree((char *)c);
#else
#if defined(sun)
#include <thread.h>
typedef thread_t ThreadId;
typedef mutex_t *Mutex;
typedef cond_t *Cond;
#define MutexLock(m) mutex_lock(m)
#define MutexUnlock(m) mutex_unlock(m)
#define MutexInit(m) mutex_init(m, USYNC_THREAD, 0)
#define CondInit(c) cond_init(c, USYNC_THREAD, 0)
#define CondWait(c,m) cond_wait(c,m)
#define CondSignal(c) cond_signal(c)
#define NewMutex() XtNew(mutex_t)
#define NewCond() XtNew(cond_t)
#define ThrSelf() thr_self()
#define SameThrId(t1,t2) t1 == t2
#define DestroyMutex(m) mutex_destroy(m)
#define DestroyCond(c) cond_destroy(c)
#define DeleteMutex(m) XtFree((char *)m)
#define DeleteCond(c)  XtFree((char *)c)
#else
#include <pthread.h>
typedef pthread_t ThreadId;
typedef pthread_mutex_t *Mutex;
typedef pthread_cond_t *Cond;
#define MutexLock(m) pthread_mutex_lock(m)
#define MutexUnlock(m) pthread_mutex_unlock(m)
#define MutexInit(m) pthread_mutex_init(m, pthread_mutexattr_default)
#define CondInit(c) pthread_cond_init(c, pthread_condattr_default)
#define CondWait(c,m) pthread_cond_wait(c,m)
#define CondSignal(c) pthread_cond_signal(c)
#define NewMutex() XtNew(pthread_mutex_t)
#define NewCond() XtNew(pthread_cond_t)
#define ThrSelf() pthread_self()
#define SameThrId(t1,t2) pthread_equal(t1, t2) 
#define DestroyMutex(m) pthread_mutex_destroy(m) 
#define DestroyCond(c) pthread_cond_destroy(c) 
#define DeleteMutex(m) XtFree((char *)m)
#define DeleteCond(c)  XtFree((char *)c)
#endif
#endif

/* No thread should have _XT_NO_THREAD_ID as its id*/
#ifndef _XT_NO_THREAD_ID
#define _XT_NO_THREAD_ID 0
#endif


/* NOTHING BELOW THIS SHOULD BE VENDOR SPECFIC */

/* typedefs */
typedef struct _LockRec {
    ThreadId holder;
    Mutex mutex;
    int recursion;
    Cond cond;
} LockRec, *LockPtr;

#define STACK_INCR 16

typedef struct _ThreadStack {
	unsigned int size;
	int sp;
	ThreadId *p;
} ThreadStack, *ThreadStackPtr;

typedef void (*ProcessLockProc)();
typedef void (*ProcessUnlockProc)();

typedef void (*InitAppLockProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef void (*AppLockProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef void (*AppUnlockProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef int  (*YieldAppLockProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef void (*RestoreAppLockProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */,
    int /* recursion */
#endif
);

typedef void (*FreeAppLockProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef void (*PushThreadProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef void (*PopThreadProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef Boolean (*IsTopThreadProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

/* INTERNAL FUNCTION PROTOTYPES */

extern ProcessLockProc _XtProcessLock;
extern ProcessUnlockProc _XtProcessUnlock;
extern InitAppLockProc _XtInitAppLock;

/* DEFINES */
#define INIT_APP_LOCK(app) if(_XtInitAppLock) (*_XtInitAppLock)(app)

#define LOCK_PROCESS if(_XtProcessLock)(*_XtProcessLock)()
#define UNLOCK_PROCESS if(_XtProcessUnlock)(*_XtProcessUnlock)()
#define LOCK_APP(app) if(app && app->lock)(*app->lock)(app)
#define UNLOCK_APP(app) if(app && app->unlock)(*app->unlock)(app)

#define YIELD_APP_LOCK(app) (app && app->yield_lock ? (*app->yield_lock)(app): -1)
#define RESTORE_APP_LOCK(app,r) if(app && app->restore_lock)(*app->restore_lock)(app,r)
#define FREE_APP_LOCK(app) if(app && app->free_lock)(*app->free_lock)(app)

#define PUSH_THREAD(app) if(app && app->push_thread)(*app->push_thread)(app) 
#define POP_THREAD(app) if(app && app->pop_thread)(*app->pop_thread)(app) 
#define IS_TOP_THREAD(app) (app && app->is_top_thread ? (*app->is_top_thread)(app): TRUE) 

#define WIDGET_TO_APPCON(w) \
    XtAppContext app = (_XtProcessLock ? XtWidgetToApplicationContext(w): 0)

#define DPY_TO_APPCON(d) \
    XtAppContext app = (_XtProcessLock ? XtDisplayToApplicationContext(d): 0)

#else /* defined(XTHREADS) */

#define LOCK_PROCESS /**/
#define UNLOCK_PROCESS /**/
#define LOCK_APP(app) /**/
#define UNLOCK_APP(app) /**/

#define INIT_APP_LOCK(app) /**/
#define YIELD_APP_LOCK(app) /**/
#define RESTORE_APP_LOCK(app,r) /**/
#define FREE_APP_LOCK(app) /**/

#define PUSH_THREAD(app) /**/
#define POP_THREAD(app) /**/
#define IS_TOP_THREAD(app) /**/

#define WIDGET_TO_APPCON(w) /**/
#define DPY_TO_APPCON(d) /**/

#endif /* defined(XTHREADS) */
#endif /* _X11_THREADSI_H */ 

