/* $XConsortium: ThreadsI.h,v 1.3 93/09/03 09:57:20 kaleb Exp $ */

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

#ifdef XTHREADS

typedef struct _LockRec *LockPtr;
typedef struct _ThreadStack *ThreadStackPtr;

typedef void (*ThreadAppProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef int (*ThreadAppYieldLockProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

typedef void (*ThreadAppRestoreLockProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */,
    int /* recursion */
#endif
);

typedef Boolean (*ThreadAppTopProc)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

extern void (*_XtProcessLock)(
#if NeedFunctionPrototypes
    void
#endif
);

extern void (*_XtProcessUnlock)(
#if NeedFunctionPrototypes
    void
#endif
);

extern void (*_XtInitAppLock)(
#if NeedFunctionPrototypes
    XtAppContext /* app */
#endif
);

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
