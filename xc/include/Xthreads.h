/*
 * $XConsortium: Xthreads.h,v 1.12 94/02/25 18:49:05 rws Exp $
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
 */

#ifndef _XTHREADS_H_
#define _XTHREADS_H_

/* Redefine these to XtMalloc/XtFree or whatever you want before including
 * this header file.
 */
#ifndef xmalloc
#define xmalloc malloc
#endif
#ifndef xfree
#define xfree free
#endif

#ifdef CTHREADS
#include <cthreads.h>
typedef cthread_t xthread_t;
typedef condition_t xcondition_t;
typedef mutex_t xmutex_t;
#define xthread_init() cthread_init()
#define xthread_self cthread_self
#define xthread_fork(func,closure) cthread_fork(func,closure)
#define xthread_yield() cthread_yield()
#define xthread_exit(v) cthread_exit(v)
#define xmutex_malloc() (xmutex_t)xmalloc(sizeof(struct mutex))
#define xmutex_init(m) mutex_init(m)
#define xmutex_clear(m) mutex_clear(m)
#define xmutex_lock(m) mutex_lock(m)
#define xmutex_unlock(m) mutex_unlock(m)
#define xcondition_malloc() (xcondition_t)xmalloc(sizeof(struct condition))
#define xcondition_init(cv) condition_init(cv)
#define xcondition_clear(cv) condition_clear(cv)
#define xcondition_wait(cv,m) condition_wait(cv,m)
#define xcondition_signal(cv) condition_signal(cv)
#define xcondition_broadcast(cv) condition_broadcast(cv)
#else
#ifdef sun
#include <thread.h>
typedef thread_t xthread_t;
typedef cond_t *xcondition_t;
typedef mutex_t *xmutex_t;
#define xthread_self thr_self
#define xthread_fork(func,closure) thr_create(NULL,0,func,closure,THR_DETACHED,NULL)
#define xthread_yield() thr_yield()
#define xthread_exit(v) thr_exit(v)
#define xmutex_malloc() (xmutex_t)xmalloc(sizeof(mutex_t))
#define xmutex_init(m) mutex_init(m,USYNC_THREAD,0)
#define xmutex_clear(m) mutex_destroy(m)
#define xmutex_lock(m) mutex_lock(m)
#define xmutex_unlock(m) mutex_unlock(m)
#define xcondition_malloc() (xcondition_t)xmalloc(sizeof(cond_t))
#define xcondition_init(cv) cond_init(cv,USYNC_THREAD,0)
#define xcondition_clear(cv) cond_destroy(cv)
#define xcondition_wait(cv,m) cond_wait(cv,m)
#define xcondition_signal(cv) cond_signal(cv)
#define xcondition_broadcast(cv) cond_broadcast(cv)
#else
#ifdef WIN32
#define BOOL wBOOL
#ifdef Status
#undef Status
#define Status wStatus
#endif
#include <windows.h>
#ifdef Status
#undef Status
#define Status int
#endif
#undef BOOL
typedef DWORD xthread_t;
struct _xthread_waiter {
    HANDLE sem;
    struct _xthread_waiter *next;
};
typedef struct _xcondition_t {
    CRITICAL_SECTION cs;
    struct _xthread_waiter *waiters;
} *xcondition_t;
typedef CRITICAL_SECTION *xmutex_t;
#define xthread_init() _Xthread_init()
#define xthread_self GetCurrentThreadId
#define xthread_fork(func,closure) { \
    DWORD _tmptid; \
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)closure, 0, \
		 &_tmptid); \
}
#define xthread_yield() Sleep(0)
#define xthread_exit(v) ExitThread((DWORD)(v))
#define xmutex_malloc() (xmutex_t)xmalloc(sizeof(CRITICAL_SECTION))
#define xmutex_init(m) InitializeCriticalSection(m)
#define xmutex_clear(m) DeleteCriticalSection(m)
#define _XMUTEX_NESTS
#define xmutex_lock(m) EnterCriticalSection(m)
#define xmutex_unlock(m) LeaveCriticalSection(m)
#define xcondition_malloc() (xcondition_t)xmalloc(sizeof(struct _xcondition_t))
#define xcondition_init(cv) { \
    InitializeCriticalSection(&(cv)->cs); \
    (cv)->waiters = NULL; \
}
#define xcondition_clear(cv) DeleteCriticalSection(&(cv)->cs)
extern struct _xthread_waiter *_Xthread_waiter();
#define xcondition_wait(cv,m) { \
    struct _xthread_waiter *_tmpthr = _Xthread_waiter(); \
    EnterCriticalSection(&(cv)->cs); \
    _tmpthr->next = (cv)->waiters; \
    (cv)->waiters = _tmpthr; \
    LeaveCriticalSection(&(cv)->cs); \
    LeaveCriticalSection(m); \
    WaitForSingleObject(_tmpthr->sem, INFINITE); \
    EnterCriticalSection(m); \
}
#define xcondition_signal(cv) { \
    EnterCriticalSection(&(cv)->cs); \
    if ((cv)->waiters) { \
        ReleaseSemaphore((cv)->waiters->sem, 1, NULL); \
	(cv)->waiters = (cv)->waiters->next; \
    } \
    LeaveCriticalSection(&(cv)->cs); \
}
#define xcondition_broadcast(cv) { \
    struct _xthread_waiter *_tmpthr; \
    EnterCriticalSection(&(cv)->cs); \
    for (_tmpthr = (cv)->waiters; _tmpthr; _tmpthr = _tmpthr->next) \
	ReleaseSemaphore(_tmpthr->sem, 1, NULL); \
    (cv)->waiters = NULL; \
    LeaveCriticalSection(&(cv)->cs); \
}
#else
#include <pthread.h>
typedef pthread_t xthread_t;
typedef pthread_cond_t *xcondition_t;
typedef pthread_mutex_t *xmutex_t;
#define xthread_self pthread_self
#define xthread_fork(func,closure) { pthread_t _tmpxthr; \
	pthread_create(&_tmpxthr,pthread_attr_default,func,closure); }
#define xthread_yield() pthread_yield()
#define xthread_exit(v) pthread_exit(v)
#define xmutex_malloc() (xmutex_t)xmalloc(sizeof(pthread_mutex_t))
#define xmutex_init(m) pthread_mutex_init(m, pthread_mutexattr_default)
#define xmutex_clear(m) pthread_mutex_destroy(m)
#define xmutex_lock(m) pthread_mutex_lock(m)
#define xmutex_unlock(m) pthread_mutex_unlock(m)
#define xcondition_malloc() (xcondition_t)xmalloc(sizeof(pthread_cond_t))
#define xcondition_init(c) pthread_cond_init(c, pthread_condattr_default)
#define xcondition_clear(c) pthread_cond_destroy(c)
#define xcondition_wait(c,m) pthread_cond_wait(c,m)
#define xcondition_signal(c) pthread_cond_signal(c)
#define xcondition_broadcast(c) pthread_cond_broadcast(c)
#ifdef _DECTHREADS_
static xthread_t _X_no_thread_id;
#define xthread_have_id(id) !pthread_equal(id, _X_no_thread_id)
#define xthread_clear_id(id) id = _X_no_thread_id
#define xthread_equal(id1,id2) pthread_equal(id1, id2)
#endif /* _DECTHREADS_ */
#endif /* WIN32 */
#endif /* sun */
#endif /* CTHREADS */
#ifndef xcondition_free
#define xcondition_free(c) xfree((char *)c)
#endif
#ifndef xmutex_free
#define xmutex_free(m) xfree((char *)m)
#endif
#ifndef xthread_have_id
#define xthread_have_id(id) id
#endif
#ifndef xthread_clear_id
#define xthread_clear_id(id) id = 0
#endif
#ifndef xthread_equal
#define xthread_equal(id1,id2) ((id1) == (id2))
#endif

#endif /* _XTHREADS_H_ */
