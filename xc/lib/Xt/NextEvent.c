/* $XConsortium: NextEvent.c,v 1.122 93/09/09 10:14:45 kaleb Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "IntrinsicI.h"
#include <stdio.h>
#include <errno.h>

extern int errno;

static TimerEventRec* freeTimerRecs;
static WorkProcRec* freeWorkRecs;
static SignalEventRec* freeSignalRecs;

/* Some systems running NTP daemons are known to return strange usec
 * values from gettimeofday.  At present (3/90) this has only been
 * reported on SunOS...
 */

#ifndef NEEDS_NTPD_FIXUP
# if defined(sun) || defined(MOTOROLA) || (defined(__osf__) && defined(__alpha))
#  define NEEDS_NTPD_FIXUP 1
# else
#  define NEEDS_NTPD_FIXUP 0
# endif
#endif

#if NEEDS_NTPD_FIXUP
#define FIXUP_TIMEVAL(t) { \
	while ((t).tv_usec >= 1000000) { \
	    (t).tv_usec -= 1000000; \
	    (t).tv_sec++; \
	} \
	while ((t).tv_usec < 0) { \
	    if ((t).tv_sec > 0) { \
		(t).tv_usec += 1000000; \
		(t).tv_sec--; \
	    } else { \
		(t).tv_usec = 0; \
		break; \
	    } \
	}}
#else
#define FIXUP_TIMEVAL(t)
#endif /*NEEDS_NTPD_FIXUP*/

#ifdef WIN32
#define GETTIMEOFDAY(t) gettimeofday(t)
#else
#ifdef SVR4
#define GETTIMEOFDAY(t) (void) gettimeofday(t)
#else
#define GETTIMEOFDAY(t) (void) gettimeofday(t,(struct timezone*)NULL)
#endif
#endif

/*
 * Private routines
 */
#define ADD_TIME(dest, src1, src2) { \
	if(((dest).tv_usec = (src1).tv_usec + (src2).tv_usec) >= 1000000) {\
	      (dest).tv_usec -= 1000000;\
	      (dest).tv_sec = (src1).tv_sec + (src2).tv_sec + 1 ; \
	} else { (dest).tv_sec = (src1).tv_sec + (src2).tv_sec ; \
	   if(((dest).tv_sec >= 1) && (((dest).tv_usec <0))) { \
	    (dest).tv_sec --;(dest).tv_usec += 1000000; } } }


#define TIMEDELTA(dest, src1, src2) { \
	if(((dest).tv_usec = (src1).tv_usec - (src2).tv_usec) < 0) {\
	      (dest).tv_usec += 1000000;\
	      (dest).tv_sec = (src1).tv_sec - (src2).tv_sec - 1;\
	} else 	(dest).tv_sec = (src1).tv_sec - (src2).tv_sec;  }

#define IS_AFTER(t1, t2) (((t2).tv_sec > (t1).tv_sec) \
	|| (((t2).tv_sec == (t1).tv_sec)&& ((t2).tv_usec > (t1).tv_usec)))

#define IS_AT_OR_AFTER(t1, t2) (((t2).tv_sec > (t1).tv_sec) \
	|| (((t2).tv_sec == (t1).tv_sec)&& ((t2).tv_usec >= (t1).tv_usec)))

static void QueueTimerEvent(app, ptr)
    XtAppContext app;
    TimerEventRec *ptr;
{
        TimerEventRec *t,**tt;
        tt = &app->timerQueue;
        t  = *tt;
        while (t != NULL &&
                IS_AFTER(t->te_timer_value, ptr->te_timer_value)) {
          tt = &t->te_next;
          t  = *tt;
         }
         ptr->te_next = t;
         *tt = ptr;
}

static void AdjustHowLong (howlong, start_time)
	unsigned long *howlong;
	struct timeval *start_time;
{
	struct timeval new_time, time_spent, lstart_time;

	lstart_time = *start_time;
	GETTIMEOFDAY (&new_time);
	FIXUP_TIMEVAL(new_time);
	TIMEDELTA(time_spent, new_time, lstart_time);
	if(*howlong <= (time_spent.tv_sec*1000+time_spent.tv_usec/1000))
	    *howlong = (unsigned long)0;  /* Timed out */
	else
	    *howlong -= (time_spent.tv_sec*1000+time_spent.tv_usec/1000);
}

/* 
 * Routine to block in the toolkit.  This should be the only call to select.
 *
 * This routine returns when there is something to be done.
 *
 * Before calling this with ignoreInputs==False, app->outstandingQueue should
 * be checked; this routine will not verify that an alternate input source
 * has not already been enqueued.
 *
 *
 * _XtWaitForSomething( appContext, 
 *                      ignoreEvent, ignoreTimers, ignoreInputs, ignoreSignals,
 *			block, drop_lock, howlong)
 * XtAppContext app;	     (Displays to check wait on)
 *
 * Boolean ignoreEvents;     (Don't return if XEvents are available
 *                              Also implies forget XEvents exist)
 *
 * Boolean ignoreTimers;     (Ditto for timers)
 *
 * Boolean ignoreInputs;     (Ditto for input callbacks )
 *
 * Boolean ignoreSignals;    (Ditto for signals)
 *
 * Boolean block;	     (Okay to block)
 *
 * Boolean drop_lock         (drop lock before going into select/poll)
 * TimeVal howlong;	     (howlong to wait for if blocking and not
 *				doing Timers... Null mean forever.
 *				Maybe should mean shortest of both)
 * Returns display for which input is available, if any
 * and if ignoreEvents==False, else returns -1
 *
 * if ignoring everything && block=True && howlong=NULL, you'll have
 * lots of time for coffee; better not try it!  In fact, it probably
 * makes little sense to do this regardless of the value of howlong
 * (bottom line is, we don't bother checking here).
 *
 * If drop_lock is FALSE, the app->lock->mutex is not unlocked before
 * entering select/poll. It is illegal for drop_lock to be FALSE if
 * ignoreTimers, ignoreInputs, or ignoreSignals is FALSE.
 */
#if NeedFunctionPrototypes
int _XtWaitForSomething(
	XtAppContext app,
	_XtBoolean ignoreEvents,
	_XtBoolean ignoreTimers,
	_XtBoolean ignoreInputs,
	_XtBoolean ignoreSignals,
	_XtBoolean block,
#ifdef XTHREADS
	_XtBoolean drop_lock,
#endif
	unsigned long *howlong)
#else
int _XtWaitForSomething(app,
			ignoreEvents, ignoreTimers, ignoreInputs, ignoreSignals,
			block, 
#ifdef XTHREADS
			drop_lock, 
#endif
			howlong)
	XtAppContext app;
	Boolean ignoreEvents;
	Boolean ignoreTimers;
	Boolean ignoreInputs;
	Boolean ignoreSignals;
	Boolean block;
#ifdef XTHREADS
	Boolean drop_lock;
#endif
	unsigned long *howlong;
#endif
{
	struct timeval  cur_time;
	struct timeval  start_time;
	struct timeval  wait_time;
	struct timeval  new_time;
	struct timeval  time_spent;
	struct timeval	max_wait_time;
	int nfound, ii, dd;
#ifdef XTHREADS
	Boolean push_thread = TRUE;
#endif
#ifndef USE_POLL /* { variable declaration block */
	static struct timeval  zero_time = { 0 , 0};
	register struct timeval *wait_time_ptr;
	Fd_set rmaskfd, wmaskfd, emaskfd;
	static Fd_set zero_fd = { 0 };
#else /* }{ */
	int poll_wait, fdlistlen, fdli;
#endif /* } */

#ifdef XTHREADS
	/* assert ((ignoreTimers && ignoreInputs && ignoreSignals) || drop_lock); */

	/* If not multi-threaded, never drop lock */
	if (app->lock == (ThreadAppProc) NULL)
	    drop_lock = FALSE;
#endif

#ifndef USE_POLL /* { init timer block */
 	if (block) {
		GETTIMEOFDAY (&cur_time);
		FIXUP_TIMEVAL(cur_time);
		start_time = cur_time;
		if(howlong == NULL) { /* special case for ever */
			wait_time_ptr = 0;
		} else { /* block until at most */
			max_wait_time.tv_sec = *howlong/1000;
			max_wait_time.tv_usec = (*howlong %1000)*1000;
			wait_time_ptr = &max_wait_time;
		}
	} else {  /* don't block */
		max_wait_time = zero_time;
		wait_time_ptr = &max_wait_time;
	}
#else /* }{ */
#define X_BLOCK -1
#define X_DONT_BLOCK 0

	if (block) {
		GETTIMEOFDAY (&cur_time);
		FIXUP_TIMEVAL(cur_time);
		start_time = cur_time;
		if(howlong == NULL)
			poll_wait = X_BLOCK;
		else {
			max_wait_time.tv_sec = *howlong / 1000;
			max_wait_time.tv_usec = (*howlong % 1000) * 1000;

			poll_wait = 
				max_wait_time.tv_sec * 1000 + max_wait_time.tv_usec / 1000;
		}
	} else {
		poll_wait = X_DONT_BLOCK;
	}
#endif /* } */

#ifndef USE_POLL /* { select/poll init block */
		if( !ignoreInputs ) {
			rmaskfd = app->fds.rmask;
			wmaskfd = app->fds.wmask;
			emaskfd = app->fds.emask;
		} else
			rmaskfd = wmaskfd = emaskfd = zero_fd;

		if (!ignoreEvents) {
		    for (dd = 0; dd < app->count; dd++) {
			FD_SET (ConnectionNumber(app->list[dd]), &rmaskfd);
		    }
		}
#else /* }{ */
	fdlistlen = 0;
	if (!ignoreEvents)
	    fdlistlen += app->count;
	if (!ignoreInputs && app->input_list != NULL) {
	    for (ii = 0; ii < (int) app->input_max; ii++)
		if (app->input_list[ii] != NULL)
		    fdlistlen++;
	}
	if (fdlistlen) {
	    struct pollfd* fdlp = app->fds.fdlist;
	    InputEvent* iep;

	    if (!ignoreEvents)
		for (fdli = 0 ; fdli < app->count; fdli++, fdlp++) {
		    fdlp->fd = ConnectionNumber (app->list[fdli]);
		    fdlp->events = POLLIN;
		}
	    if (!ignoreInputs && app->input_list != NULL)
		for (ii = 0; ii < app->input_max; ii++)
		    if (app->input_list[ii] != NULL) {
			iep = app->input_list[ii];
			fdlp->fd = ii;
			fdlp->events = 0;
			for ( ; iep; iep = iep->ie_next) {
			    if (iep->ie_condition & XtInputReadMask)
				fdlp->events |= POLLIN;
			    if (iep->ie_condition & XtInputWriteMask)
				fdlp->events |= POLLOUT;
			    if (iep->ie_condition & XtInputExceptMask)
				fdlp->events |= POLLERR;
			}
			fdlp++;
		    }
	}
#endif /* } */

      WaitLoop:
	while (1) {

#ifndef USE_POLL /* { adjust timers block */
		if (app->timerQueue != NULL && !ignoreTimers && block) {
		    if(IS_AFTER(cur_time, app->timerQueue->te_timer_value)) {
			TIMEDELTA (wait_time, app->timerQueue->te_timer_value, 
				   cur_time);
			if(howlong==NULL || IS_AFTER(wait_time,max_wait_time)){
				wait_time_ptr = &wait_time;
			} else {
				wait_time_ptr = &max_wait_time;
			}
		    } else wait_time_ptr = &zero_time;
		} 
#else /* }{ */
		if (app->timerQueue != NULL && !ignoreTimers && block) {
		    if (IS_AFTER (cur_time, app->timerQueue->te_timer_value)) {
			TIMEDELTA (wait_time, app->timerQueue->te_timer_value,
				   cur_time);
			if (howlong == NULL || IS_AFTER (wait_time, max_wait_time)) {
			    poll_wait = 
				wait_time.tv_sec * 1000 + wait_time.tv_usec / 1000;
			}
		    } else {
			poll_wait = X_DONT_BLOCK;
		    }
		} 
#endif /* } */
		if (app->block_hook_list) {
		    BlockHook hook;
		    for (hook = app->block_hook_list; 
			 hook != NULL; hook = hook->next)
			(*hook->proc) (hook->closure);
		}

#ifdef XTHREADS /* { */
		if (drop_lock) {
		    int yield = 0;
		    if (push_thread) {
			PUSH_THREAD(app);
			push_thread = FALSE;
		    }
		    yield = YIELD_APP_LOCK(app);
#ifndef USE_POLL /* { select/poll block */
		    nfound = select (app->fds.nfds, 
				     (int *) &rmaskfd, 
				     (int *) &wmaskfd, 
				     (int *) &emaskfd, 
				     wait_time_ptr);
#else /* }{ */
		    nfound = poll (app->fds.fdlist, fdlistlen, poll_wait);
#endif /* } */
		    RESTORE_APP_LOCK(app, yield);

		    if (!IS_TOP_THREAD(app))
			goto WaitLoop;

		    POP_THREAD(app);
		    push_thread = TRUE;
		} else
#endif /* } */
#ifndef USE_POLL /* { select/poll block */
		    nfound = select (app->fds.nfds, 
				     (int *) &rmaskfd, 
				     (int *) &wmaskfd, 
				     (int *) &emaskfd, 
				     wait_time_ptr);
#else /* }{ */
		    nfound = poll (app->fds.fdlist, fdlistlen, poll_wait);
#endif /* } */
		if (nfound == -1) {
			/*
			 *  interrupt occured recalculate time value and select
			 *  again.
			 */
			if (errno == EINTR) {
			    errno = 0;  /* errno is not self reseting */

			    /* was it interrupted by a signal that we care about? */
			    if (!ignoreSignals && app->signalQueue != NULL) {
				SignalEventRec *se_ptr = app->signalQueue;
				while (se_ptr != NULL) {
				    if (se_ptr->se_notice) {
					if (block && howlong != NULL)
					    AdjustHowLong (howlong, &start_time);
					return -1;
				    }
				    se_ptr = se_ptr->se_next;
				}
			    }

			    if (block) {
#ifndef USE_POLL /* { adjust timeout after signal interrupt block */
				if (wait_time_ptr == NULL) /*howlong == NULL*/
				    continue;
				GETTIMEOFDAY (&new_time);
				FIXUP_TIMEVAL(new_time);
				TIMEDELTA(time_spent, new_time, cur_time);
				cur_time = new_time;
				if(IS_AFTER(time_spent, *wait_time_ptr)) {
					TIMEDELTA(wait_time, *wait_time_ptr,
						  time_spent);
					wait_time_ptr = &wait_time;
					continue;
				} else {
					/* time is up anyway */
					nfound = 0;
				}
#else /* }{ */
				if (poll_wait == X_BLOCK)
				    continue;
				GETTIMEOFDAY (&new_time);
				FIXUP_TIMEVAL(new_time);
				TIMEDELTA(time_spent, new_time, cur_time);
				cur_time = new_time;
				if ((time_spent.tv_sec * 1000 + time_spent.tv_usec / 1000) < poll_wait) {
					poll_wait -= (time_spent.tv_sec * 1000 + time_spent.tv_usec / 1000);
					continue;
				} else {
					/* time is up anyway */
					nfound = 0;
				}
#endif /* } */
			    }
			} else {
			    char Errno[12];
			    String param = Errno;
			    Cardinal param_count = 1;

			    if (!ignoreEvents) {
				/* get Xlib to detect a bad connection */
				for (dd = 0; dd < app->count; dd++) {
				    if (XEventsQueued(app->list[dd],
						      QueuedAfterReading)) {
					return dd;
				    }
				}
			    }
			    sprintf( Errno, "%d", errno);
			    XtAppWarningMsg(app, "communicationError","select",
			       XtCXtToolkitError,"Select failed; error code %s",
			       &param, &param_count);
			    continue;
			}
		} /* timed out or input available */
		break;
	}
	
	if (nfound == 0) {
		if(howlong) *howlong = (unsigned long)0;  /* Timed out */
		return -1;
	}
	if(block && howlong != NULL) { /* adjust howlong */
	    AdjustHowLong (howlong, &start_time);
	}

#ifndef USE_POLL /* { check ready file descriptors block */
	if(ignoreInputs) {
	    if (ignoreEvents) return -1; /* then only doing timers */
	    for (dd = 0; dd < app->count; dd++) {
		if (FD_ISSET(ConnectionNumber(app->list[dd]), &rmaskfd)) {
		    if (XEventsQueued( app->list[dd], QueuedAfterReading ))
			return dd;
		    /*
		     * An error event could have arrived
		     * without any real events, or events
		     * could have been swallowed by Xlib,
		     * or the connection may be broken.
		     * We can't tell the difference, so
		     * ssume Xlib will eventually discover
		     * a broken connection.
		     */
		}
	    }
	    goto WaitLoop;	/* must have been only error events */
        }
	{
	int ret = -1;
	Boolean found_input = False;

	for (ii = 0; ii < app->fds.nfds && nfound > 0; ii++) {
	    XtInputMask condition = 0;
	    if (FD_ISSET (ii, &rmaskfd)) {
		nfound--;
		if (!ignoreEvents) {
		    for (dd = 0; dd < app->count; dd++) {
			if (ii == ConnectionNumber(app->list[dd])) {
			    if (ret == -1) {
				if (XEventsQueued( app->list[dd],
						   QueuedAfterReading ))
				    ret = dd;
				/*
				 * An error event could have arrived
				 * without any real events, or events
				 * could have been swallowed by Xlib,
				 * or the connection may be broken.
				 * We can't tell the difference, so
				 * assume Xlib will eventually discover
				 * a broken connection.
				 */
			    }
			    goto ENDILOOP;
			}
		    }
		}
		condition = XtInputReadMask;
	    }
	    if (FD_ISSET (ii, &wmaskfd)) {
		condition |= XtInputWriteMask;
		nfound--;
	    }
	    if (FD_ISSET (ii, &emaskfd)) {
		condition |= XtInputExceptMask;
		nfound--;
	    }
	    if (condition) {
		InputEvent *ep;
		for (ep = app->input_list[ii]; ep; ep = ep->ie_next) {
		    if (condition & ep->ie_condition) {
			ep->ie_oq = app->outstandingQueue;
			app->outstandingQueue = ep;
		    }
		}
		found_input = True;
	    }
ENDILOOP:   ;
	} /* endfor */
	if (ret >= 0 || found_input)
	    return ret;
	goto WaitLoop;		/* must have been only error events */
	}
#else /* }{ */
	{
	struct pollfd* fdlp = app->fds.fdlist;
	int ret = -1;
	Boolean found_input = False;

	fdli = 0;
	if (!ignoreEvents)
	    for ( ; fdli < app->count; fdli++, fdlp++)
		if (fdlp->revents & POLLIN &&
			XEventsQueued (app->list[fdli], QueuedAfterReading))
		    ret = fdli;

	if (!ignoreInputs) {
	    InputEvent *ep;
	    XtInputMask condition;
	    for ( ; fdli < fdlistlen; fdli++, fdlp++) {
		condition = 0;
		if (fdlp->revents) {
		    if (fdlp->revents & POLLIN)
			condition = XtInputReadMask;
		    if (fdlp->revents & POLLOUT)
			condition |= XtInputWriteMask;
		    if (fdlp->revents & POLLERR)
			condition |= XtInputExceptMask;
		}
		if (condition) {
		    found_input = True;
		    for (ep = app->input_list[fdlp->fd]; ep; ep = ep->ie_next)
			if (condition & ep->ie_condition) {
			    ep->ie_oq = app->outstandingQueue;
			    app->outstandingQueue = ep;
			}
		}
	    }
	}
	if (ret >= 0 || found_input)
	    return ret;
	goto WaitLoop;	/* must have been only error events */
	}
#undef X_BLOCK
#undef X_DONT_BLOCK
#endif /* } */
}

#define IeCallProc(ptr) \
    (*ptr->ie_proc) (ptr->ie_closure, &ptr->ie_source, (XtInputId*)&ptr);

#define TeCallProc(ptr) \
    (*ptr->te_proc) (ptr->te_closure, (XtIntervalId*)&ptr);

#define SeCallProc(ptr) \
    (*ptr->se_proc) (ptr->se_closure, (XtSignalId*)&ptr);

/*
 * Public Routines
 */

XtIntervalId XtAddTimeOut(interval, proc, closure)
	unsigned long interval;
	XtTimerCallbackProc proc;
	XtPointer closure;
{
	return XtAppAddTimeOut(_XtDefaultAppContext(), 
		interval, proc, closure); 
}


XtIntervalId XtAppAddTimeOut(app, interval, proc, closure)
	XtAppContext app;
	unsigned long interval;
	XtTimerCallbackProc proc;
	XtPointer closure;
{
	TimerEventRec *tptr;
	struct timeval current_time;

	LOCK_APP(app);
	LOCK_PROCESS;
	if (freeTimerRecs) {
	    tptr = freeTimerRecs;
	    freeTimerRecs = tptr->te_next;
	}
	else tptr = XtNew(TimerEventRec);
	UNLOCK_PROCESS;

	tptr->te_next = NULL;
	tptr->te_closure = closure;
	tptr->te_proc = proc;
	tptr->app = app;
	tptr->te_timer_value.tv_sec = interval/1000;
	tptr->te_timer_value.tv_usec = (interval%1000)*1000;
        GETTIMEOFDAY (&current_time);
	FIXUP_TIMEVAL(current_time);
        ADD_TIME(tptr->te_timer_value,tptr->te_timer_value,current_time);
	QueueTimerEvent(app, tptr);
	UNLOCK_APP(app);
	return( (XtIntervalId) tptr);
}

void  XtRemoveTimeOut(id)
	XtIntervalId id;
{
	TimerEventRec *t, *last, *tid = (TimerEventRec *) id;
	XtAppContext app = tid->app;

	/* find it */
	LOCK_APP(app);
	for(t = tid->app->timerQueue, last = NULL;
	    t != NULL && t != tid;
	    t = t->te_next) last = t;

	if (t == NULL) {
	    UNLOCK_APP(app);
	    return; /* couldn't find it */
	}
	if(last == NULL) { /* first one on the list */
	    t->app->timerQueue = t->te_next;
	} else last->te_next = t->te_next;

	LOCK_PROCESS;
	t->te_next = freeTimerRecs;
	freeTimerRecs = t;
	UNLOCK_PROCESS;
	UNLOCK_APP(app);
}

XtWorkProcId XtAddWorkProc(proc, closure)
	XtWorkProc proc;
	XtPointer closure;
{
	return XtAppAddWorkProc(_XtDefaultAppContext(), proc, closure);
}

XtWorkProcId XtAppAddWorkProc(app, proc, closure)
	XtAppContext app;
	XtWorkProc proc;
	XtPointer closure;
{
	WorkProcRec *wptr;

	LOCK_APP(app);
	LOCK_PROCESS;
	if (freeWorkRecs) {
	    wptr = freeWorkRecs;
	    freeWorkRecs = wptr->next;
	} else wptr = XtNew(WorkProcRec);
	UNLOCK_PROCESS;
	wptr->next = app->workQueue;
	wptr->closure = closure;
	wptr->proc = proc;
	wptr->app = app;
	app->workQueue = wptr;
	UNLOCK_APP(app);
	return (XtWorkProcId) wptr;
}

void  XtRemoveWorkProc(id)
	XtWorkProcId id;
{
	WorkProcRec *wid= (WorkProcRec *) id, *w, *last;
	XtAppContext app = wid->app;

	LOCK_APP(app);
	/* find it */
	for(w = wid->app->workQueue, last = NULL; w != NULL && w != wid; w = w->next) last = w;

	if (w == NULL) {
	    UNLOCK_APP(app);
	    return; /* couldn't find it */
	}

	if(last == NULL) wid->app->workQueue = w->next;
	else last->next = w->next;
	LOCK_PROCESS;
	w->next = freeWorkRecs;
	freeWorkRecs = w;
	UNLOCK_PROCESS;
	UNLOCK_APP(app);
}

XtSignalId XtAddSignal(proc, closure)
	XtSignalCallbackProc proc;
	XtPointer closure;
{
	return XtAppAddSignal(_XtDefaultAppContext(), proc, closure);
}

XtSignalId XtAppAddSignal(app, proc, closure)
	XtAppContext app;
	XtSignalCallbackProc proc;
	XtPointer closure;
{
	SignalEventRec *sptr;

	LOCK_APP(app);
	LOCK_PROCESS;
	if (freeSignalRecs) {
	    sptr = freeSignalRecs;
	    freeSignalRecs = sptr->se_next;
	} else
	    sptr = XtNew(SignalEventRec);
	UNLOCK_PROCESS;
	sptr->se_next = app->signalQueue;
	sptr->se_closure = closure;
	sptr->se_proc = proc;
	sptr->app = app;
	sptr->se_notice = FALSE;
	app->signalQueue = sptr;
	UNLOCK_APP(app);
	return (XtSignalId) sptr;
}

void XtRemoveSignal(id)
	XtSignalId id;
{
	SignalEventRec *sid = (SignalEventRec*) id, *s, *last = NULL;
	XtAppContext app = sid->app;

	LOCK_APP(app);
	for (s = sid->app->signalQueue; s != NULL && s != sid; s = s->se_next)
	    last = s;
	if (s == NULL) {
	    UNLOCK_APP(app);
	    return;
	}
	if (last == NULL)
	    sid->app->signalQueue = s->se_next;
	else
	    last->se_next = s->se_next;
	LOCK_PROCESS;
	s->se_next = freeSignalRecs;
	freeSignalRecs = s;
	UNLOCK_PROCESS;
	UNLOCK_APP(app);
}

void XtNoticeSignal(id)
	XtSignalId id;
{
	/*
	 * It would be overkill to lock the app to set this flag.
	 * In the worst case, 2..n threads would be modifying this
	 * flag. The last one wins. Since signals occur asynchronously
	 * anyway, this can occur with or without threads.
	 *
	 * The other issue is that thread t1 sets the flag in a
	 * signalrec that has been deleted in thread t2. We rely
	 * on a detail of the implementation, i.e. free'd signalrecs
	 * aren't really free'd, they're just moved to a list of
	 * free recs, so deref'ing one won't hurt anything.
	 *
	 * Lastly, and perhaps most importantly, since POSIX threads
	 * says that the handling of asynchronous signals in a synchronous
	 * threads environment is undefined. Therefor it would be an
	 * error for both signals and threads to be in use in the same
	 * program.
	 */
	SignalEventRec *sid = (SignalEventRec*) id;
	sid->se_notice = TRUE;
}

XtInputId XtAddInput( source, Condition, proc, closure)
	int source;
	XtPointer Condition;
	XtInputCallbackProc proc;
	XtPointer closure;
{
	return XtAppAddInput(_XtDefaultAppContext(),
		source, Condition, proc, closure);
}

XtInputId XtAppAddInput(app, source, Condition, proc, closure)
	XtAppContext app;
	int source;
	XtPointer Condition;
	XtInputCallbackProc proc;
	XtPointer closure;
{
	InputEvent* sptr;
	XtInputMask condition = (XtInputMask) Condition;

	LOCK_APP(app);
	if (!condition ||
	    condition & ~(XtInputReadMask|XtInputWriteMask|XtInputExceptMask))
	    XtAppErrorMsg(app,"invalidParameter","xtAddInput",XtCXtToolkitError,
			  "invalid condition passed to XtAppAddInput",
			  (String *)NULL, (Cardinal *)NULL);

	if (app->input_max <= source) {
	    Cardinal n = source + 1;
	    app->input_list = (InputEvent**)XtRealloc((char*) app->input_list,
						      n * sizeof(InputEvent*));
	    bzero((char *) &app->input_list[app->input_max],
		  (unsigned) (n - app->input_max) * sizeof(InputEvent*));
	    app->input_max = n;
	}
	sptr = XtNew(InputEvent);
	sptr->ie_proc = proc;
	sptr->ie_closure = closure;
	sptr->app = app;
	sptr->ie_oq = NULL;
	sptr->ie_source = source;
	sptr->ie_condition = condition;
	sptr->ie_next = app->input_list[source];
	app->input_list[source] = sptr;

#ifndef USE_POLL
	if (condition & XtInputReadMask)   FD_SET(source, &app->fds.rmask);
	if (condition & XtInputWriteMask)  FD_SET(source, &app->fds.wmask);
	if (condition & XtInputExceptMask) FD_SET(source, &app->fds.emask);

	if (app->fds.nfds < (source+1)) app->fds.nfds = source+1;
#else
	if (sptr->ie_next == NULL) {
	    app->fds.nfds++;
	    app->fds.fdlist = (struct pollfd *)
		XtRealloc ((char *)app->fds.fdlist,
			   app->fds.nfds * sizeof (struct pollfd));
	}
#endif
	app->fds.count++;
	UNLOCK_APP(app);
	return((XtInputId)sptr);
}

void XtRemoveInput( id )
	register XtInputId  id;
{
  	register InputEvent *sptr, *lptr;
	XtAppContext app = ((InputEvent *)id)->app;
	register int source = ((InputEvent *)id)->ie_source;
	Boolean found = False;

	LOCK_APP(app);
	sptr = app->outstandingQueue;
	lptr = NULL;
	for (; sptr != NULL; sptr = sptr->ie_oq) {
	    if (sptr == (InputEvent *)id) {
		if (lptr == NULL) app->outstandingQueue = sptr->ie_oq;
		else lptr->ie_oq = sptr->ie_oq;
	    }
	    lptr = sptr;
	}

	if(app->input_list && (sptr = app->input_list[source]) != NULL) {
		for( lptr = NULL ; sptr; sptr = sptr->ie_next ){
			if(sptr == (InputEvent *) id) {
				XtInputMask condition = 0;
				if(lptr == NULL) {
				    app->input_list[source] = sptr->ie_next;
				} else {
				    lptr->ie_next = sptr->ie_next;
				}
#ifndef USE_POLL
				for (lptr = app->input_list[source];
				     lptr; lptr = lptr->ie_next)
				    condition |= lptr->ie_condition;
				if ((sptr->ie_condition & XtInputReadMask) &&
				    !(condition & XtInputReadMask))
				   FD_CLR(source, &app->fds.rmask);
				if ((sptr->ie_condition & XtInputWriteMask) &&
				    !(condition & XtInputWriteMask))
				   FD_CLR(source, &app->fds.wmask);
				if ((sptr->ie_condition & XtInputExceptMask) &&
				    !(condition & XtInputExceptMask))
				   FD_CLR(source, &app->fds.emask);
#endif
				XtFree((char *) sptr);
				found = True;
				break;
			}
			lptr = sptr;	      
		}
	}

	if (found)
	    app->fds.count--;
	else
	    XtAppWarningMsg(app, "invalidProcedure","inputHandler",
			    XtCXtToolkitError, 
			    "XtRemoveInput: Input handler not found",
			    (String *)NULL, (Cardinal *)NULL);
	UNLOCK_APP(app);
}

void _XtRemoveAllInputs(app)
    XtAppContext app;
{
    int i;
    for (i = 0; i < app->input_max; i++) {
	InputEvent* ep = app->input_list[i];
	while (ep) {
	    InputEvent *next = ep->ie_next;
	    XtFree( (char*)ep );
	    ep = next;
	}
    }
    XtFree((char *) app->input_list);
}

/* Do alternate input and timer callbacks if there are any */

static void DoOtherSources(app)
	XtAppContext app;
{
	TimerEventRec *te_ptr;
	InputEvent *ie_ptr;
	struct timeval  cur_time;

#define DrainQueue() \
	for (ie_ptr = app->outstandingQueue; ie_ptr != NULL;) { \
	    app->outstandingQueue = ie_ptr->ie_oq;		\
	    ie_ptr ->ie_oq = NULL;				\
	    IeCallProc(ie_ptr);					\
	    ie_ptr = app->outstandingQueue;			\
	}
/*enddef*/
	DrainQueue();
	if (app->fds.count > 0) {
	    /* Call _XtWaitForSomething to get input queued up */
	    (void) _XtWaitForSomething (app,
					TRUE, TRUE, FALSE, TRUE, 
					FALSE, 
#ifdef XTHREADS
					TRUE, 
#endif
					(unsigned long *)NULL);
	    DrainQueue();
	}
	if (app->timerQueue != NULL) {	/* check timeout queue */
	    GETTIMEOFDAY (&cur_time);
	    FIXUP_TIMEVAL(cur_time);
	    while(IS_AT_OR_AFTER (app->timerQueue->te_timer_value, cur_time)) {
		te_ptr = app->timerQueue;
		app->timerQueue = te_ptr->te_next;
		te_ptr->te_next = NULL;
		if (te_ptr->te_proc != NULL)
		    TeCallProc(te_ptr);
		LOCK_PROCESS;
		te_ptr->te_next = freeTimerRecs;
		freeTimerRecs = te_ptr;
		UNLOCK_PROCESS;
		if (app->timerQueue == NULL) break;
	    }
	}
	if (app->signalQueue != NULL) {
	    SignalEventRec *se_ptr = app->signalQueue;
	    while (se_ptr != NULL) {
		if (se_ptr->se_notice) {
		    se_ptr->se_notice = FALSE;
		    if (se_ptr->se_proc != NULL)
			SeCallProc(se_ptr);
		}
		se_ptr = se_ptr->se_next;
	    }
	}
#undef DrainQueue
}

/* If there are any work procs, call them.  Return whether we did so */

static Boolean CallWorkProc(app)
	XtAppContext app;
{
	register WorkProcRec *w = app->workQueue;
	Boolean delete;

	if (w == NULL) return FALSE;

	app->workQueue = w->next;

	delete = (*(w->proc)) (w->closure);

	if (delete) {
	    LOCK_PROCESS;
	    w->next = freeWorkRecs;
	    freeWorkRecs = w;
	    UNLOCK_PROCESS;
	}
	else {
	    w->next = app->workQueue;
	    app->workQueue = w;
	}
	return TRUE;
}

/*
 * XtNextEvent()
 * return next event;
 */

void XtNextEvent(event)
	XEvent *event;
{
	XtAppNextEvent(_XtDefaultAppContext(), event);
}

void _XtRefreshMapping(event, dispatch)
    XEvent *event;
    Boolean dispatch;
{
    XtPerDisplay pd;

    LOCK_PROCESS;
    pd = _XtGetPerDisplay(event->xmapping.display);
    if (event->xmapping.request != MappingPointer &&
	pd && pd->keysyms && (event->xmapping.serial >= pd->keysyms_serial))
	_XtBuildKeysymTables( event->xmapping.display, pd );
    XRefreshKeyboardMapping(&event->xmapping);
    if (dispatch && pd && pd->mapping_callbacks)
	XtCallCallbackList((Widget) NULL,
			   (XtCallbackList)pd->mapping_callbacks,
			   (XtPointer)event );
    UNLOCK_PROCESS;
}

void XtAppNextEvent(app, event)
	XtAppContext app;
	XEvent *event;
{
    int i, d;

    LOCK_APP(app);
    for (;;) {
	if (app->count == 0)
	    DoOtherSources(app);
	else {
	    for (i = 1; i <= app->count; i++) {
		d = (i + app->last) % app->count;
		if (d == 0) DoOtherSources(app);
		if (XEventsQueued(app->list[d], QueuedAfterReading))
		    goto GotEvent;
	    }
	    for (i = 1; i <= app->count; i++) {
		d = (i + app->last) % app->count;
		if (XEventsQueued(app->list[d], QueuedAfterFlush))
		    goto GotEvent;
	    }
	}

	/* We're ready to wait...if there is a work proc, call it */
	if (CallWorkProc(app)) continue;

	d = _XtWaitForSomething (app,
				 FALSE, FALSE, FALSE, FALSE,
				 TRUE, 
#ifdef XTHREADS
				 TRUE, 
#endif
				 (unsigned long *) NULL);

	if (d != -1) {
	  GotEvent:
	    XNextEvent (app->list[d], event);
#ifdef XTHREADS
	    /* assert(app->list[d] == event->xany.display); */
#endif
	    app->last = d;
	    if (event->xany.type == MappingNotify)
		_XtRefreshMapping(event, False);
	    UNLOCK_APP(app);
	    return;
	} 

    } /* for */
}
    
void XtProcessEvent(mask)
	XtInputMask mask;
{
	XtAppProcessEvent(_XtDefaultAppContext(), mask);
}

void XtAppProcessEvent(app, mask)
	XtAppContext app;
	XtInputMask mask;
{
	int i, d;
	XEvent event;
	struct timeval cur_time;

	LOCK_APP(app);
	if (mask == 0) {
	    UNLOCK_APP(app);
	    return;
	}

	for (;;) {

	    if (mask & XtIMSignal && app->signalQueue != NULL) {
		SignalEventRec *se_ptr = app->signalQueue;
		while (se_ptr != NULL) {
		    if (se_ptr->se_notice) {
			se_ptr->se_notice = FALSE;
			SeCallProc(se_ptr);
			UNLOCK_APP(app);
			return;
		    }
		    se_ptr = se_ptr->se_next;
		}
	    }

	    if (mask & XtIMTimer && app->timerQueue != NULL) {
		GETTIMEOFDAY (&cur_time);
		FIXUP_TIMEVAL(cur_time);
		if (IS_AT_OR_AFTER(app->timerQueue->te_timer_value, cur_time)){
		    TimerEventRec *te_ptr = app->timerQueue;
		    app->timerQueue = app->timerQueue->te_next;
		    te_ptr->te_next = NULL;
                    if (te_ptr->te_proc != NULL)
		        TeCallProc(te_ptr);
		    LOCK_PROCESS;
		    te_ptr->te_next = freeTimerRecs;
		    freeTimerRecs = te_ptr;
		    UNLOCK_PROCESS;
		    UNLOCK_APP(app);
		    return;
		}
	    }
    
	    if (mask & XtIMAlternateInput) {
		if (app->fds.count > 0 && app->outstandingQueue == NULL) {
		    /* Call _XtWaitForSomething to get input queued up */
		    (void) _XtWaitForSomething (app, 
						TRUE, TRUE, FALSE, TRUE, 
						FALSE, 
#ifdef XTHREADS
						TRUE, 
#endif
						(unsigned long *)NULL);
		}
		if (app->outstandingQueue != NULL) {
		    InputEvent *ie_ptr = app->outstandingQueue;
		    app->outstandingQueue = ie_ptr->ie_oq;
		    ie_ptr->ie_oq = NULL;
		    IeCallProc(ie_ptr);
		    UNLOCK_APP(app);
		    return;
		}
	    }
    
	    if (mask & XtIMXEvent) {
		for (i = 1; i <= app->count; i++) {
		    d = (i + app->last) % app->count;
		    if (XEventsQueued(app->list[d], QueuedAfterReading))
			goto GotEvent;
		}
		for (i = 1; i <= app->count; i++) {
		    d = (i + app->last) % app->count;
		    if (XEventsQueued(app->list[d], QueuedAfterFlush))
			goto GotEvent;
		}
	    }

	    /* Nothing to do...wait for something */

	    if (CallWorkProc(app)) continue;

	    d = _XtWaitForSomething (app,
				    (mask & XtIMXEvent ? FALSE : TRUE),
				    (mask & XtIMTimer ? FALSE : TRUE),
				    (mask & XtIMAlternateInput ? FALSE : TRUE),
				    (mask & XtIMSignal ? FALSE : TRUE),
				    TRUE, 
#ifdef XTHREADS
				    TRUE, 
#endif
				    (unsigned long *) NULL);

	    if (mask & XtIMXEvent && d != -1) {
	      GotEvent:
		XNextEvent(app->list[d], &event);
#ifdef XTHREADS
		/* assert(app->list[d] == event.xany.display); */
#endif
		app->last = d;
		if (event.xany.type == MappingNotify) {
		    _XtRefreshMapping(&event, False);
		}
		XtDispatchEvent(&event);
		UNLOCK_APP(app);
		return;
	    } 
	
	}
}

XtInputMask XtPending()
{
	return XtAppPending(_XtDefaultAppContext());
}

XtInputMask XtAppPending(app)
	XtAppContext app;
{
	struct timeval cur_time;
	int d;
	XtInputMask ret = 0;

/*
 * Check for pending X events
 */
	LOCK_APP(app);
	for (d = 0; d < app->count; d++) {
	    if (XEventsQueued(app->list[d], QueuedAfterReading)) {
		ret = XtIMXEvent;
		break;
	    }
	}
	if (ret == 0) {
	    for (d = 0; d < app->count; d++) {
		if (XEventsQueued(app->list[d], QueuedAfterFlush)) {
		    ret = XtIMXEvent;
		    break;
		}
	    }
	}

	if (app->signalQueue != NULL) {
	    SignalEventRec *se_ptr = app->signalQueue;
	    while (se_ptr != NULL) {
		if (se_ptr->se_notice) {
		    ret |= XtIMSignal;
		    break;
		}
		se_ptr = se_ptr->se_next;
	    }
	}

/*
 * Check for pending alternate input
 */
	if (app->timerQueue != NULL) {	/* check timeout queue */ 
	    GETTIMEOFDAY (&cur_time);
	    FIXUP_TIMEVAL(cur_time);
	    if ((IS_AT_OR_AFTER(app->timerQueue->te_timer_value, cur_time))  &&
                (app->timerQueue->te_proc != 0)) {
		ret |= XtIMTimer;
	    }
	}

	if (app->outstandingQueue != NULL) ret |= XtIMAlternateInput;
	else {
	    /* This won't cause a wait, but will enqueue any input */

	    if(_XtWaitForSomething (app,
				    FALSE, TRUE, FALSE, TRUE, 
				    FALSE, 
#ifdef XTHREADS
				    TRUE, 
#endif
				    (unsigned long *) NULL) != -1) 
		ret |= XtIMXEvent;
	    if (app->outstandingQueue != NULL) ret |= XtIMAlternateInput;
	}
	UNLOCK_APP(app);
	return ret;
}

/* Peek at alternate input and timer callbacks if there are any */

static Boolean PeekOtherSources(app)
	XtAppContext app;
{
	struct timeval  cur_time;

	if (app->outstandingQueue != NULL) return TRUE;

	if (app->signalQueue != NULL) {
	    SignalEventRec *se_ptr = app->signalQueue;
	    while (se_ptr != NULL) {
		if (se_ptr->se_notice)
		    return TRUE;
		se_ptr = se_ptr->se_next;
	    }
	}

	if (app->fds.count > 0) {
	    /* Call _XtWaitForSomething to get input queued up */
	    (void) _XtWaitForSomething (app,
					TRUE, TRUE, FALSE, TRUE, 
					FALSE, 
#ifdef XTHREADS
					TRUE, 
#endif
					(unsigned long *)NULL);
	    if (app->outstandingQueue != NULL) return TRUE;
	}

	if (app->timerQueue != NULL) {	/* check timeout queue */
	    GETTIMEOFDAY (&cur_time);
	    FIXUP_TIMEVAL(cur_time);
	    if (IS_AT_OR_AFTER (app->timerQueue->te_timer_value, cur_time))
		return TRUE;
	}

	return FALSE;
}

Boolean XtPeekEvent(event)
	XEvent *event;
{
	return XtAppPeekEvent(_XtDefaultAppContext(), event);
}

Boolean XtAppPeekEvent(app, event)
	XtAppContext app;
	XEvent *event;
{
	int i, d;
	Boolean foundCall = FALSE;

	LOCK_APP(app);
	for (i = 1; i <= app->count; i++) {
	    d = (i + app->last) % app->count;
	    if (d == 0) foundCall = PeekOtherSources(app);
	    if (XEventsQueued(app->list[d], QueuedAfterReading))
		goto GotEvent;
	}
	for (i = 1; i <= app->count; i++) {
	    d = (i + app->last) % app->count;
	    if (XEventsQueued(app->list[d], QueuedAfterFlush))
		goto GotEvent;
	}
	
	if (foundCall) {
	    event->xany.type = 0;
	    event->xany.display = NULL;
	    event->xany.window = 0;
	    UNLOCK_APP(app);
	    return FALSE;
	}
	
	d = _XtWaitForSomething (app,
				 FALSE, FALSE, FALSE, FALSE,
				 TRUE, 
#ifdef XTHREADS
				 TRUE, 
#endif
				 (unsigned long *) NULL);
	
	if (d != -1) {
	  GotEvent:
	    XPeekEvent(app->list[d], event);
	    app->last = (d == 0 ? app->count : d) - 1;
	    UNLOCK_APP(app);
	    return TRUE;
	}
	event->xany.type = 0;	/* Something else must be ready */
	event->xany.display = NULL;
	event->xany.window = 0;
	UNLOCK_APP(app);
	return FALSE;
}	
