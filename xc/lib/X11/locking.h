/*
 * $XConsortium: locking.h,v 1.12 94/02/09 23:19:51 rws Exp $
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
 * Used by XlibInt.c, locking.c, LockDis.c
 */

#ifndef _X_locking_H_
#define _X_locking_H_

#define xmalloc(s) Xmalloc(s)
#define xfree(s) Xfree(s)
#include <X11/Xthreads.h>

struct _XCVList {
    xcondition_t cv;
    xReply *buf;
    struct _XCVList *next;
};

extern xthread_t (*_Xthread_self_fn)( /* in XlibInt.c */
#if NeedFunctionPrototypes
    void
#endif
);

extern struct _XCVList *(*_XCreateCVL_fn)(
#if NeedFunctionPrototypes
    void
#endif
);

/* Display->lock is a pointer to one of these */

struct _XLockInfo {
	xmutex_t mutex;		/* mutex for critical sections */
	int reply_bytes_left;	/* nbytes of the reply still to read */
	Bool reply_was_read;	/* _XReadEvents read a reply for _XReply */
	struct _XCVList *reply_awaiters; /* list of CVs for _XReply */
	struct _XCVList **reply_awaiters_tail;
	struct _XCVList *event_awaiters; /* list of CVs for _XReadEvents */
	struct _XCVList **event_awaiters_tail;
	/* for XLockDisplay */
	int locking_level;	/* how many times into XLockDisplay we are */
	xthread_t locking_thread; /* thread that did XLockDisplay */
	xcondition_t cv;	/* wait if another thread has XLockDisplay */
	xthread_t reading_thread; /* cache */
	xthread_t conni_thread;	/* thread in XProcessInternalConnection */
	xcondition_t writers;	/* wait for writable */
	/* used only in XlibInt.c */
	void (*pop_reader)(
#if NeedNestedPrototypes
			   Display* /* dpy */,
			   struct _XCVList* /* cvl */,
			   struct _XCVList** /* list */,
			   struct _XCVList*** /* tail */
#endif
			   );
	struct _XCVList *(*push_reader)(
#if NeedNestedPrototypes
					struct _XCVList*** /* tail */
#endif
					);
	void (*condition_wait)(
#if NeedNestedPrototypes
			       xcondition_t /* cv */,
			       xmutex_t /* mutex */
#if defined(XTHREADS_WARN) || defined(XTHREADS_FILE_LINE)
			       , char* /* file */,
			       int /* line */
#endif
#endif
			       );
	void (*internal_lock_display)(
#if NeedNestedPrototypes
				      Display* /* dpy */,
				      Bool /* wskip */
#if defined(XTHREADS_WARN) || defined(XTHREADS_FILE_LINE)
				      , char* /* file */,
				      int /* line */
#endif
#endif
				      );
	/* used in XlibInt.c and locking.c */
	void (*condition_signal)(
#if NeedNestedPrototypes
				 xcondition_t /* cv */
#if defined(XTHREADS_WARN) || defined(XTHREADS_FILE_LINE)
				 , char* /* file */,
				 int /* line */
#endif
#endif
				 );
	void (*condition_broadcast)(
#if NeedNestedPrototypes
				 xcondition_t /* cv */
#if defined(XTHREADS_WARN) || defined(XTHREADS_FILE_LINE)
				 , char* /* file */,
				 int /* line */
#endif
#endif
				    );
	/* used in XlibInt.c and XLockDis.c */
	void (*lock_wait)(
#if NeedNestedPrototypes
			  Display* /* dpy */
#endif
			  );
	void (*user_lock_display)(
#if NeedNestedPrototypes
				  Display* /* dpy */
#endif
				  );
	void (*user_unlock_display)(
#if NeedNestedPrototypes
				    Display* /* dpy */
#endif
				    );
};

#define UnlockNextEventReader(d,c) if ((d)->lock) \
    (*(d)->lock->pop_reader)((d),(c), &(d)->lock->event_awaiters,&(d)->lock->event_awaiters_tail)

#if defined(XTHREADS_WARN) || defined(XTHREADS_FILE_LINE)
#define ConditionWait(d,c) if ((d)->lock) \
	(*(d)->lock->condition_wait)(c, (d)->lock->mutex,__FILE__,__LINE__)
#define ConditionSignal(d,c) if ((d)->lock) \
	(*(d)->lock->condition_signal)(c,__FILE__,__LINE__)
#define ConditionBroadcast(d,c) if ((d)->lock) \
	(*(d)->lock->condition_broadcast)(c,__FILE__,__LINE__)
#else
#define ConditionWait(d,c) if ((d)->lock) \
	(*(d)->lock->condition_wait)(c, (d)->lock->mutex)
#define ConditionSignal(d,c) if ((d)->lock) \
	(*(d)->lock->condition_signal)(c)
#define ConditionBroadcast(d,c) if ((d)->lock) \
	(*(d)->lock->condition_broadcast)(c)
#endif

typedef struct _LockInfoRec {
	xmutex_t	lock;
} LockInfoRec;

#endif /* _X_locking_H_ */
