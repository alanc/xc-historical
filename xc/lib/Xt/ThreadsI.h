/* $XConsortium: ThreadsI.h,v 1.1 93/08/27 08:55:56 kaleb Exp $ */

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

#define xmalloc XtMalloc
#define xfree   XtFree

#include <X11/Xthreads.h>
#define NDEBUG
#include <assert.h>

/* No thread should have _XT_NO_THREAD_ID as its id*/
#ifndef _XT_NO_THREAD_ID
#define _XT_NO_THREAD_ID 0
#endif


/* NOTHING BELOW THIS SHOULD BE VENDOR SPECFIC */

/* typedefs */
typedef struct _LockRec {
    xthread_t holder;
    xmutex_t mutex;
    int recursion;
    xcondition_t cond;
} LockRec, *LockPtr;

#define STACK_INCR 16

typedef struct _ThreadStack {
	unsigned int size;
	int sp;
	xthread_t *p;
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

#endif /* !defined(XTHREADS) */
