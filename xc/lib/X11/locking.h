/*
 * $XConsortium: locking.h,v 1.2 93/07/10 19:12:12 rws Exp $
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
 * locking.h - data types for C Threads locking.
 * Used only by XlibInt.c, locking.c, and XLockDis.c
 */

#ifndef _X_locking_H_
#define _X_locking_H_

#ifdef CTHREADS
#include <cthreads.h>
typedef cthread_t xthread_t;
#define condition_malloc() (condition_t)Xmalloc(sizeof(struct condition))
#define mutex_malloc() (mutex_t)Xmalloc(sizeof(struct mutex))
#define xthread_self() cthread_self()
#define xthread_init() cthread_init()
#else
#include <pthread.h>
typedef pthread_mutex_t *mutex_t;
typedef pthread_cond_t *condition_t;
typedef pthread_t xthread_t;
#define xthread_self() pthread_self()
#define condition_malloc() (condition_t)Xmalloc(sizeof(pthread_cond_t))
#define condition_init(c) pthread_cond_init(c, pthread_condattr_default)
#define condition_clear(c) pthread_cond_destroy(c)
#define condition_wait(c,m) pthread_cond_wait(c,m)
#define condition_signal(c) pthread_cond_signal(c)
#define condition_broadcast(c) pthread_cond_broadcast(c)
#define mutex_malloc() (mutex_t)Xmalloc(sizeof(pthread_mutex_t))
#define mutex_init(m) pthread_mutex_init(m, pthread_mutexattr_default)
#define mutex_clear(m) pthread_mutex_destroy(m)
#define mutex_lock(m) pthread_mutex_lock(m)
#define mutex_unlock(m) pthread_mutex_unlock(m)
#ifdef __OSF1__
extern xthread_t _X_no_thread_id;
#define xthread_have_id(id) !pthread_equal(id, _X_no_thread_id)
#define xthread_clear_id(id) id = _X_no_thread_id
#define xthread_equal(id1,id2) pthread_equal(id1, id2)
#endif
#endif
#ifndef condition_free
#define condition_free(c) Xfree((char *)c)
#endif
#ifndef mutex_free
#define mutex_free(m) Xfree((char *)m)
#endif
#ifndef xthread_have_id
#define xthread_have_id(id) id
#endif
#ifndef xthread_clear_id
#define xthread_clear_id(id) id = 0
#endif
#ifndef xthread_equal
#define xthread_equal(id1,id2) id1 == id2
#endif

struct _XCVList {
    condition_t cv;
    xReply *buf;
    struct _XCVList *next;
};

/* Display->lock is a pointer to one of these */

struct _XLockInfo {
	mutex_t mutex;		/* mutex for critical sections */
	int reply_bytes_left;	/* nbytes of the reply still to read */
	Bool reply_was_read;	/* _XReadEvents read a reply for _XReply */
	struct _XCVList *reply_awaiters; /* list of CVs for _XReply */
	struct _XCVList **reply_awaiters_tail;
	struct _XCVList *event_awaiters; /* list of CVs for _XReadEvents */
	struct _XCVList **event_awaiters_tail;
	/* for XLockDisplay */
	xthread_t locking_thread; /* thread that did XLockDisplay */
	condition_t cv;		/* wait if another thread has XLockDisplay */
};

#ifdef XTHREADS_WARN
#define ConditionWait(d,c) if ((d)->lock_fns) \
	(*(d)->lock_fns->condition_wait)((c)->cv, (d)->lock->mutex,__FILE__,__LINE__)
#define ConditionSignal(d,c) if ((d)->lock_fns) \
	(*(d)->lock_fns->condition_signal)((c)->cv,__FILE__,__LINE__)
#else
#define ConditionWait(d,c) if ((d)->lock_fns) \
	(*(d)->lock_fns->condition_wait)((c)->cv, (d)->lock->mutex)
#define ConditionSignal(d,c) if ((d)->lock_fns) \
	(*(d)->lock_fns->condition_signal)((c)->cv)
#endif

#endif /* _X_locking_H_ */
