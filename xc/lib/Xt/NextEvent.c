#ifndef lint
static char rcsid[] = "$Header: NextEvent.c,v 1.27 87/12/02 16:58:57 swick Locked $";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#ifndef VMS
#include <stdio.h>
#include <errno.h>
#include <X11/Xlib.h>
#else
#include stdio
#include errno
#include Xlib
#endif
#include "Intrinsic.h"
#ifndef VMS
#include <sys/time.h>
#else
#include <types.h>
#include <time.h>
#endif
#ifndef VMS
#include <sys/file.h>
#else
#include file
#endif
#ifndef VMS
#include <sys/param.h>
#endif
#include "fd.h"

#ifdef VMS
#include iodef
#include ssdef
#define  TIMER_EVENT_FLAG        13
#define  TIMER_EVENT_MASK	1 << TIMER_EVENT_FLAG
#endif

extern int errno;

/*
 * Private definitions
 */


typedef struct _vms_time {
     unsigned long low;
     unsigned long high;
}vms_time;

typedef struct _TimerEventRec {

#ifdef VMS
        vms_time timer_value;
#else
        struct timeval   timer_value;
#endif

	struct _TimerEventRec *next;
	Display *dpy;
	Widget   widget;
	XtIntervalId id;
}TimerEventRec;

static struct Select_event {
	Widget	Se_widget;
	struct	Select_event	*Se_next;
	XClientMessageEvent	Se_event;
	struct  Select_event	*Se_oq;
};


/*
 * Private data
 */

static TimerEventRec *TimerQueue = NULL;

#ifdef VMS
#define NOFILE	64
#endif

static struct Select_event *Select_rqueue[NOFILE], *Select_wqueue[NOFILE],
  *Select_equeue[NOFILE];
static struct  Select_event *outstanding_queue = NULL;


static struct 
{
  	Fd_set rmask;
	Fd_set wmask;
	Fd_set emask;
	int	nfds;
} composite;


  

/*
 * Private routines
 */
#ifdef unix
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

#else /* not correct - but good enough for testing */
#define ADD_TIME(dest, src1, src2) { \
       dest.low = src1.low+src2.low; \
       dest.high = src1.high+src2.high; }

#define IS_AFTER(t1,t2) (((t2).high > (t1).high) \
       ||(((t2).high == (t1).high)&& ((t2).low > (t1).low)))

#endif


#ifdef VMS
static void StartTimer(t)
    TimerEventRec *t;
{
   int status;

   status = sys$setimr(TIMER_EVENT_FLAG ,&(t->timer_value), 0, 0);
}
#endif

static void QueueTimerEvent(ptr)
    TimerEventRec *ptr;
{
        TimerEventRec *t,**tt;
        tt = &TimerQueue;
        t  = *tt;
        while (t != NULL &&
                IS_AFTER(t->timer_value, ptr->timer_value)) {
          tt = &t->next;
          t  = *tt;
         }
         ptr->next = t;
         *tt = ptr;
#ifdef VMS
         if (TimerQueue == ptr)
    	     StartTimer(TimerQueue);
#endif
}



/*
 * Public Routines
 */

XtIntervalId
XtAddTimeOut(widget, interval)
Widget widget;
int interval;
{
	TimerEventRec *tptr;
#ifdef unix
        struct timeval current_time, timezone;
#else
        vms_time current_time;
#endif    
	tptr = (TimerEventRec *)XtMalloc((unsigned) sizeof(TimerEventRec));
	tptr->next = NULL;
#ifdef unix
	tptr->timer_value.tv_sec = interval/1000;
	tptr->timer_value.tv_usec = (interval%1000)*1000;
        (void) gettimeofday(&current_time,&timezone);
        
#else  /* make this mod something for overflow */
        tptr->timer_value.low = interval * 10000;
        tptr->timer_value.high = 0;
        sys$gettim(&current_time);
#endif
        ADD_TIME(tptr->timer_value,tptr->timer_value,current_time);
	tptr->widget = widget;
	tptr->id = (XtIntervalId) tptr;
	QueueTimerEvent(tptr);
	return(tptr->id);
}

void  XtRemoveTimeOut(id)
    XtIntervalId id;
{
   TimerEventRec *t, **tt;
   tt = &TimerQueue;
   t = *tt;

   /* find it */
   while (t != NULL && t->id != id) {
     tt = &t->next;
     t  = *tt;
   }
   if (t == NULL) return; /* couldn't find it */
#ifdef VMS
   if (t == TimerQueue){
       sys$cantim();
       sys$clref(TIMER_EVENT_FLAG);
       if (t->next != NULL)
           StartTimer(t->next);
       }
#endif
   *tt = t ->next;
    XtFree((char*)t);
    return;
}

void 
XtAddInput(widget,source,condition)
Widget widget;
int source;
int condition;
{
#ifdef unix
	struct Select_event *sptr;
	
	if(((int)condition &(XtInputReadMask|XtInputWriteMask|XtInputExceptMask))==0) {
	  return; /* error */ /* XXX */
	}
	if(condition&XtInputReadMask){
	    sptr = (struct Select_event *)XtMalloc((unsigned) sizeof (*sptr));
	    sptr->Se_widget = widget;
	    sptr->Se_next = Select_rqueue[source];
	    Select_rqueue[source] = sptr;
	    FD_SET(source, &composite.rmask);
	    sptr->Se_event.type = ClientMessage;
	    sptr->Se_event.display = XtDisplay(widget);
	    sptr->Se_event.window = widget->core.window;
	    sptr->Se_event.message_type = XtHasInput;
	    sptr->Se_event.format = 32;
	    sptr->Se_event.data.l[0] = (int)source;
	    sptr->Se_event.data.l[1] = XtInputReadMask;
	}
	
	if(condition&XtInputWriteMask) {
	    sptr = (struct Select_event *) XtMalloc((unsigned) sizeof (*sptr));
	    sptr->Se_widget = widget;
	    sptr->Se_next = Select_wqueue[source];
	    Select_wqueue[source] = sptr;
	    FD_SET(source, &composite.wmask);
	    sptr->Se_event.type = ClientMessage;
	    sptr->Se_event.display = XtDisplay(widget);
	    sptr->Se_event.window = widget->core.window;
	    sptr->Se_event.message_type = XtHasInput;
	    sptr->Se_event.format = 32;
	    sptr->Se_event.data.l[0] = (int)source;
	    sptr->Se_event.data.l[1] = XtInputWriteMask;
	}
	
	if(condition&XtInputExceptMask) {
	    sptr = (struct Select_event *) XtMalloc((unsigned) sizeof (*sptr));
	    sptr->Se_widget = widget;
	    sptr->Se_next = Select_equeue[source];
	    Select_equeue[source] = sptr;
	    FD_SET(source, &composite.emask);
	    sptr->Se_event.type = ClientMessage;
	    sptr->Se_event.display = XtDisplay(widget);
	    sptr->Se_event.window = widget->core.window;
	    sptr->Se_event.message_type = XtHasInput;
	    sptr->Se_event.format = 32;
	    sptr->Se_event.data.l[0] = (int)source;
	    sptr->Se_event.data.l[1] = XtInputExceptMask;
	}
	if (composite.nfds < (source+1))
	    composite.nfds = source+1;
#else
	XtWarning("XtAddInput is not implemented.");
#endif
}

void XtRemoveInput(widget, source, condition)
Widget widget;
int source;
int condition;
{
#ifdef unix
  	register struct Select_event *sptr, *lptr;

	if(((int)condition &(XtInputReadMask|XtInputWriteMask|XtInputExceptMask))==0) {
	    return; /* error */ /* XXX */
	}
	if(condition&XtInputReadMask){
	    if((sptr = Select_rqueue[source]) == NULL)
	      return; /* error */ /* XXX */
	    for(lptr = NULL;sptr; sptr = sptr->Se_next){
		if(sptr->Se_widget == widget) {
		    if(lptr == NULL) {
			Select_rqueue[source] = sptr->Se_next;
			FD_CLR(source, &composite.rmask);
		    } else {
			lptr->Se_next = sptr->Se_next;
		    }
		    XtFree((char *) sptr);
		    return;
		}
		lptr = sptr;	      
	    }
	}
	if(condition&XtInputWriteMask){
	    if((sptr = Select_wqueue[source]) == NULL)
	      return; /* error */ /* XXX */
	    for(lptr = NULL;sptr; sptr = sptr->Se_next){
		if(sptr->Se_widget == widget) {
		    if(lptr == NULL){
			Select_wqueue[source] = sptr->Se_next;
			FD_CLR(source, &composite.wmask);
		    }else {
			lptr->Se_next = sptr->Se_next;
		    }
		    XtFree((char *) sptr);
		    return;
		}
		lptr = sptr;
	    }
	    
	}
	if(condition&XtInputExceptMask){
	    if((sptr = Select_equeue[source]) == NULL)
	      return; /* error */ /* XXX */
	    for(lptr = NULL;sptr; sptr = sptr->Se_next){
		if(sptr->Se_widget == widget) {
		    if(lptr == NULL){
			Select_equeue[source] = sptr->Se_next;
			FD_CLR(source, &composite.emask);
		    }else {
			lptr->Se_next = sptr->Se_next;
		    }
		    XtFree((char *) sptr);
		    return;
		}
		lptr = sptr;
	    }
	    
	}
	return;
	 /* error */
#else
	XtWarning("XtRemoveInput is not implemented.");
#endif
}

#ifdef VMS
extern int      dpy_event_flag_cluster;
#endif


     
/*
 * XtNextEvent()
 * return next event;
 */

void XtNextEvent(event)
XEvent *event;
#ifdef unix
{
    struct Select_event *se_ptr;
    TimerEventRec *te_ptr;
    struct timeval  cur_time;
    struct timezone cur_timezone;
    Fd_set rmaskfd, wmaskfd, emaskfd;
    int     nfound,
            i;
    struct timeval  wait_time;
    struct timeval *wait_time_ptr;
    int     Claims_X_is_pending = 0;
    XClientMessageEvent * ev = (XClientMessageEvent *) event;

    if (DestroyList != NULL) {
	_XtCallCallbacks (&DestroyList, (Opaque) NULL);
	_XtRemoveAllCallbacks (&DestroyList);
    }

    for (;;) {
	if (XPending (toplevelDisplay) || Claims_X_is_pending) {
	    XNextEvent (toplevelDisplay, event);
	    return;
	}
	if ((se_ptr = outstanding_queue) != NULL) {
	 if(se_ptr->Se_event.window == NULL) {
 	      /* input event added before widget was realized */
 	    	se_ptr->Se_event.window = XtWindow(se_ptr->Se_widget);
 	    }
	    *event = *((XEvent *) & se_ptr->Se_event);
	    outstanding_queue = se_ptr->Se_oq;
	    return;
	}
	(void) gettimeofday (&cur_time, &cur_timezone);
	if (TimerQueue!= NULL) {	/* check timeout queue */
	    if (IS_AFTER (TimerQueue->timer_value, cur_time)) {
	        /* timer has expired */
		ev->type = ClientMessage;
		ev->display = toplevelDisplay;
		ev->window = TimerQueue->widget->core.window;
		ev->message_type = XtTimerExpired;
		ev->format = 32;
		ev->data.l[0] = (int) TimerQueue->id;
		te_ptr = TimerQueue;
		TimerQueue = TimerQueue->next;
		te_ptr->next = NULL;
		XtFree((char*)te_ptr);
		return;
	    }
	}
	/* No timers ready time to wait */
	/* should be done only once */
	if (ConnectionNumber (toplevelDisplay) + 1 > composite.nfds)
	    composite.nfds = ConnectionNumber (toplevelDisplay) + 1;
	while (1) {
	    FD_SET (ConnectionNumber (toplevelDisplay), &composite.rmask);
	    if (TimerQueue != NULL) {
		TIMEDELTA (wait_time, TimerQueue->timer_value, cur_time);
		wait_time_ptr = &wait_time;
	    } else wait_time_ptr = (struct timeval *) 0;
	    rmaskfd = composite.rmask;
	    wmaskfd = composite.wmask;
	    emaskfd = composite.emask;
	    if ((nfound = select (composite.nfds,
	        (int *) & rmaskfd, (int *) & wmaskfd,
		(int *) & emaskfd, wait_time_ptr)) == -1) {
		if (errno == EINTR) continue;
	    }
	    if (nfound == -1) XtError("Select failed.");
	    break;
	}
	if (nfound == 0)
	    continue;
	for (i = 0; i < composite.nfds && nfound > 0; i++) {
	    if (FD_ISSET (i, &rmaskfd)) {
		if (i == ConnectionNumber (toplevelDisplay)) {
		    Claims_X_is_pending = 1;
		    nfound--;
		} else {
		    Select_rqueue[i]->Se_oq = outstanding_queue;
		    outstanding_queue = Select_rqueue[i];
		    nfound--;
		}
	    }
	    if (FD_ISSET (i, &wmaskfd)) {
		Select_wqueue[i]->Se_oq = outstanding_queue;
		outstanding_queue = Select_wqueue[i];
		nfound--;
	    }
	    if (FD_ISSET (i, &emaskfd)) {
		Select_equeue[i]->Se_oq = outstanding_queue;
		outstanding_queue = Select_equeue[i];
		nfound--;
	    }
	}
    }
}
#else    /* VMS */
{
    XClientMessageEvent * ev = (XClientMessageEvent *) event;
    TimerEventRec *te_ptr;
    unsigned long eventFlags;    
    int status;

    if (DestroyList != NULL) {
	_XtCallCallbacks (&DestroyList, (Opaque) NULL);
	_XtRemoveAllCallbacks (&DestroyList);
    }

    for (;;) {
        if (XPending (toplevelDisplay) || (TimerQueue == NULL))  {
            XNextEvent (toplevelDisplay, event);
            return;
        }
	status = sys$readef(0, &eventFlags);
        if ((status == SS$_WASSET) && (eventFlags & TIMER_EVENT_MASK)) {
        /* timer expired*/
                ev->type = ClientMessage;
                ev->display = toplevelDisplay;
                ev->window = TimerQueue->widget->core.window;
                ev->message_type = XtTimerExpired;
                ev->format = 32;
                ev->data.l[0] = (int) TimerQueue->id;
                te_ptr = TimerQueue;
                TimerQueue = TimerQueue->next;
                XtFree((char*)te_ptr);
                if (TimerQueue != NULL) {
                     StartTimer(TimerQueue);
                }
                return;

	}
 /* no events to process and no timers expired, wait for either to happen*/ 
/*       sys$qio(1,toplevelDisplay->fd,IO$_READVBLK);
       sys$wflor(TIMER_EVENT_FLAG ,3);  */
       status = sys$wflor(dpy_event_flag_cluster << 5,
                TIMER_EVENT_MASK | (1 << toplevelDisplay->fd) );
   }
}
#endif

Boolean XtPending()
#ifdef unix
{
    Fd_set rmask, wmask, emask;
    struct timeval cur_time, wait_time;
    struct timezone curzone;
    Boolean ret;

    (void) gettimeofday(&cur_time, &curzone);
    
    if(ret = (Boolean) XPending(toplevelDisplay))
      return(ret);

    if(outstanding_queue)
      return TRUE;
    
    if(IS_AFTER(cur_time, (TimerQueue->timer_value)))
	return TRUE;

    FD_SET(ConnectionNumber(toplevelDisplay),&composite.rmask); /*should be done only once */
    if(ConnectionNumber(toplevelDisplay) +1 > composite.nfds) 
      composite.nfds = ConnectionNumber(toplevelDisplay) + 1;
    wait_time.tv_sec = 0;
    wait_time.tv_usec = 0;
    rmask = composite.rmask;
    wmask = composite.wmask;
    emask = composite.emask;
    if(select(composite.nfds,(int *)&rmask,(int *)&wmask,(int*)&emask,&wait_time) > 0)
	return TRUE;
      
    return FALSE;  
}
#else
{ return XPending(toplevelDisplay); }
#endif

XtPeekEvent(event)
XEvent *event;
#ifdef unix
{
    Fd_set rmask, wmask, emask;
    int nfound, i;
    struct timeval cur_time, wait_time;
    struct timezone curzone;
    int Claims_X_is_pending = 0;
    XClientMessageEvent *ev = (XClientMessageEvent *)event;

    if(XPending(toplevelDisplay)){
	XPeekEvent(toplevelDisplay, event); /* Xevents */
	return(1);
    }
    if(outstanding_queue){
	*event = *((XEvent *)&outstanding_queue->Se_event);
	return(1);
    }
    (void) gettimeofday(&cur_time, &curzone);
    if(IS_AFTER(cur_time, (TimerQueue->timer_value))) {
	ev->type = ClientMessage;
	ev->display = toplevelDisplay;
	ev->window =  TimerQueue->widget->core.window;
		  ev->format = 32;
	ev->message_type = XtTimerExpired;
	ev->format = 32;
	ev->data.l[0] = (int)TimerQueue->id;
	return(1);
    }
    
    FD_SET(ConnectionNumber(toplevelDisplay),&composite.rmask);/* should be done only once */
    if(ConnectionNumber(toplevelDisplay) +1 > composite.nfds) 
      composite.nfds = ConnectionNumber(toplevelDisplay) + 1;
    TIMEDELTA(wait_time, TimerQueue->timer_value, cur_time);
    rmask = composite.rmask;
    wmask = composite.wmask;
    emask = composite.emask;
    nfound=select(composite.nfds,(int *)&rmask,(int *)&wmask,(int *)&emask,&wait_time);
    
    for(i = 0; i < composite.nfds && nfound > 0;i++) {
	if(FD_ISSET(i,&rmask)) {
	    if(i == ConnectionNumber(toplevelDisplay)) {
		Claims_X_is_pending= 1;
	      } else {
		Select_rqueue[i] -> Se_oq = outstanding_queue;
		outstanding_queue = Select_rqueue[i];
		nfound--;
	      }
	}
	if(FD_ISSET(i,&wmask)) {
	    Select_wqueue[i] -> Se_oq = outstanding_queue;
	    outstanding_queue = Select_wqueue[i];
	    nfound--;
	}
	if(FD_ISSET(i,&emask)) {
	    Select_equeue[i] -> Se_oq = outstanding_queue;
	    outstanding_queue = Select_equeue[i];
	    nfound--;
	}

      }
    if(Claims_X_is_pending && XPending(toplevelDisplay)) {
      XPeekEvent(toplevelDisplay, event);
      return(1);
    }

    if(outstanding_queue){
	*event = *((XEvent *)&outstanding_queue->Se_event);
	return(1);
    }
    return(0);
}	
#else
{
    if (XPending(toplevelDisplay)) {
	XPeekEvent(toplevelDisplay, event);
	return(1);
    } else return(0);
}
#endif
