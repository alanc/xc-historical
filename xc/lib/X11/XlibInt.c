/*
 * $XConsortium: XlibInt.c,v 11.190 93/10/13 14:28:33 rws Exp $
 */

/* Copyright    Massachusetts Institute of Technology    1985, 1986, 1987 */

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

/*
 *	XlibInt.c - Internal support routines for the C subroutine
 *	interface library (Xlib) to the X Window System Protocol V11.0.
 */
#define NEED_EVENTS
#define NEED_REPLIES

#include "Xlibint.h"
#include "Xlibnet.h"
#include <X11/Xos.h>
#include <stdio.h>
#ifdef WIN32
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef EINTR
#define EINTR WSAEINTR
#endif

#ifdef XTHREADS
#include "locking.h"

/* these pointers get initialized by XInitThreads */
void (*_XLockMutex_fn)() = NULL;
void (*_XUnlockMutex_fn)() = NULL;
xthread_t (*_Xthread_self_fn)() = NULL;

#define XThread_Self()	((*_Xthread_self_fn)())

#define UnlockNextReplyReader(d) if ((d)->lock_fns) \
    (*(d)->lock_fns->pop_reader)((d), &(d)->lock->reply_awaiters,&(d)->lock->reply_awaiters_tail)
#define UnlockNextEventReader(d) if ((d)->lock_fns) \
    (*(d)->lock_fns->pop_reader)((d), &(d)->lock->event_awaiters,&(d)->lock->event_awaiters_tail)

#define QueueReplyReaderLock(d) ((d)->lock_fns ? \
    (*(d)->lock_fns->push_reader)(&(d)->lock->reply_awaiters_tail) : NULL)
#define QueueEventReaderLock(d) ((d)->lock_fns ? \
    (*(d)->lock_fns->push_reader)(&(d)->lock->event_awaiters_tail) : NULL)

#define DisplayLockWait(d) if ((d)->lock_fns && (d)->lock_fns->lock_wait) \
    (*(d)->lock_fns->lock_wait)(d)

#else /* XTHREADS else */

#define UnlockNextReplyReader(d)   
#define UnlockNextEventReader(d)
#define QueueReplyReaderLock(d) NULL
#define QueueEventReaderLock(d) NULL
#define ConditionWait(d,c)
#define ConditionSignal(d,c)
#define DisplayLockWait(d)

#endif /* XTHREADS else */ 

/* check for both EAGAIN and EWOULDBLOCK, because some supposedly POSIX
 * systems are broken and return EWOULDBLOCK when they should return EAGAIN
 */
#ifdef WIN32
#define ETEST() (WSAGetLastError() == WSAEWOULDBLOCK)
#else
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define ETEST() (errno == EAGAIN || errno == EWOULDBLOCK)
#else
#ifdef EAGAIN
#define ETEST() (errno == EAGAIN)
#else
#define ETEST() (errno == EWOULDBLOCK)
#endif
#endif
#endif
#ifdef WIN32
#define ECHECK(err) (WSAGetLastError() == err)
#define ESET(val) WSASetLastError(val)
#else
#define ECHECK(err) (errno == err)
#define ESET(val) errno = val
#endif

#if defined(LOCALCONN) || defined(LACHMAN)
#ifdef EMSGSIZE
#define ESZTEST() (ECHECK(EMSGSIZE) || ECHECK(ERANGE))
#else
#define ESZTEST() ECHECK(ERANGE)
#endif
#else
#ifdef EMSGSIZE
#define ESZTEST() ECHECK(EMSGSIZE)
#endif
#endif

#ifdef MUSTCOPY

#define STARTITERATE(tpvar,type,start,endcond) \
  { register char *cpvar; \
  for (cpvar = (char *) (start); endcond; ) { \
    type dummy; memcpy ((char *) &dummy, cpvar, SIZEOF(type)); \
    tpvar = &dummy;
#define ITERPTR(tpvar) cpvar
#define RESETITERPTR(tpvar,type,start) cpvar = start
#define INCITERPTR(tpvar,type) cpvar += SIZEOF(type)
#define ENDITERATE }}

#else

#define STARTITERATE(tpvar,type,start,endcond) \
  for (tpvar = (type *) (start); endcond; )
#define ITERPTR(tpvar) (char *)tpvar
#define RESETITERPTR(tpvar,type,start) tpvar = (type *) (start)
#define INCITERPTR(tpvar,type) tpvar++
#define ENDITERATE

#endif /* MUSTCOPY */

typedef union {
    xReply rep;
    char buf[BUFSIZE];
} _XAlignedBuffer;

static char *_XAsyncReply();
static void _XProcessInternalConnection();
extern int _XSyncFunction();
#define SEQLIMIT (65535 - (BUFSIZE / SIZEOF(xReq)) - 10)

/*
 * The following routines are internal routines used by Xlib for protocol
 * packet transmission and reception.
 *
 * _XIOError(Display *) will be called if any sort of system call error occurs.
 * This is assumed to be a fatal condition, i.e., XIOError should not return.
 *
 * _XError(Display *, xError *) will be called whenever an X_Error event is
 * received.  This is not assumed to be a fatal condition, i.e., it is
 * acceptable for this procedure to return.  However, XError should NOT
 * perform any operations (directly or indirectly) on the DISPLAY.
 *
 * Routines declared with a return type of 'Status' return 0 on failure,
 * and non 0 on success.  Routines with no declared return type don't 
 * return anything.  Whenever possible routines that create objects return
 * the object they have created.
 */

static int padlength[4] = {0, 3, 2, 1};
    /* lookup table for adding padding bytes to data that is read from
    	or written to the X socket.  */

static xReq _dummy_request = {
	0, 0, 0
};

/*
 * This is an OS dependent routine which:
 * 1) returns as soon as the connection can be written on....
 * 2) if the connection can be read, must enqueue events and handle errors,
 * until the connection is writable.
 */
static void
_XWaitForWritable(dpy
#ifdef XTHREADS
		  , cv
#endif
		  )
    Display *dpy;
#ifdef XTHREADS
    xcondition_t cv;		/* our reading condition variable */
#endif
{
#ifdef USE_POLL
    struct pollfd filedes;
#else
    FdSet r_mask;
    FdSet w_mask;
#endif
    int nfound;

#ifdef USE_POLL
    filedes.fd = dpy->fd;
    filedes.events = 0;
#else
    CLEARBITS(r_mask);
    CLEARBITS(w_mask);
#endif

    for (;;) {
#ifdef XTHREADS
	/* We allow only one thread at a time to read, to minimize
	   passing of read data between threads.
	   Now, who is it?  If there is a non-NULL reply_awaiters and
	   we (i.e., our cv) are not at the head of it, then whoever
	   is at the head is the reader, and we don't read.
	   Otherwise there is no reply_awaiters or we are at the
	   head, having just appended ourselves.
	   In this case, if there is a event_awaiters, then whoever
	   is at the head of it got there before we did, and they are the
	   reader.

	   Last cases: no event_awaiters and we are at the head of
	   reply_awaiters or reply_awaiters is NULL: we are the reader,
	   since there is obviously no one else involved.

	   XXX - what if cv is NULL and someone else comes along after
	   us while we are waiting?
	   */
	   
	if (!dpy->lock ||
	    (!dpy->lock->event_awaiters &&
	     (!dpy->lock->reply_awaiters || dpy->lock->reply_awaiters->cv == cv)))
#endif
#ifdef USE_POLL
	    filedes.events = POLLIN;
	filedes.events |= POLLOUT;
#else
	    BITSET(r_mask, dpy->fd);
        BITSET(w_mask, dpy->fd);
#endif

	do {
#ifdef USE_POLL
	    nfound = poll (&filedes, 1, -1);
#else
#ifdef WIN32
	    nfound = select (0, &r_mask, &w_mask, NULL, NULL);
#else
	    nfound = select (dpy->fd + 1, r_mask, w_mask, NULL, NULL);
#endif
#endif
	    if (nfound < 0 && !ECHECK(EINTR))
		_XIOError(dpy);
	} while (nfound <= 0);

#ifdef USE_POLL
	if (filedes.revents & POLLIN)
#else
	if (GETBIT(r_mask, dpy->fd))
#endif
	{
	    _XAlignedBuffer buf;
	    BytesReadable_t pend;
	    register BytesReadable_t len;
	    register xReply *rep;

	    /* find out how much data can be read */
	    if (BytesReadable(dpy->fd, &pend) < 0)
		_XIOError(dpy);
	    len = pend;

	    /* must read at least one xEvent; if none is pending, then
	       we'll just block waiting for it */
	    if (len < SIZEOF(xReply)) len = SIZEOF(xReply);
		
	    /* but we won't read more than the max buffer size */
	    if (len > BUFSIZE) len = BUFSIZE;

	    /* round down to an integral number of XReps */
	    len = (len / SIZEOF(xReply)) * SIZEOF(xReply);

	    _XRead (dpy, buf.buf, (long) len);

	    STARTITERATE(rep,xReply,buf.buf,len > 0) {
		if (rep->generic.type == X_Reply) {
		    pend = len;
		    RESETITERPTR(rep,xReply,
				 _XAsyncReply (dpy, rep,
					       ITERPTR(rep), &pend, True));
		    len = pend;
		} else {
		    if (rep->generic.type == X_Error)
			_XError (dpy, (xError *)rep);
		    else	/* must be an event packet */
			_XEnq (dpy, (xEvent *)rep);
		    INCITERPTR(rep,xReply);
		    len -= SIZEOF(xReply);
		}
	    } ENDITERATE
	}
#ifdef USE_POLL
	if (filedes.revents & (POLLOUT|POLLHUP))
#else
	if (GETBIT(w_mask, dpy->fd))
#endif
	    return;
    }
}


#define POLLFD_CACHE_SIZE 5

/* initialize the struct array passed to poll() below */
Bool _XPollfdCacheInit(dpy)
    Display *dpy;
{
#ifdef USE_POLL
    struct pollfd *pfp;

    pfp = (struct pollfd *)Xmalloc(POLLFD_CACHE_SIZE * sizeof(struct pollfd));
    if (!pfp)
	return False;
    pfp[0].fd = dpy->fd;
    pfp[0].events = POLLIN;

    dpy->filedes = (XPointer)pfp;
#endif
    return True;
}

void _XPollfdCacheAdd(dpy, fd)
    Display *dpy;
    int fd;
{
#ifdef USE_POLL
    struct pollfd *pfp = (struct pollfd *)dpy->filedes;

    if (dpy->im_fd_length <= POLLFD_CACHE_SIZE) {
	pfp[dpy->im_fd_length].fd = fd;
	pfp[dpy->im_fd_length].events = POLLIN;
    }
#endif
}

/* ARGSUSED */
void _XPollfdCacheDel(dpy, fd)
    Display *dpy;
    int fd;			/* not used */
{
#ifdef USE_POLL
    struct pollfd *pfp = (struct pollfd *)dpy->filedes;
    struct _XConnectionInfo *conni;

    /* just recalculate whole list */
    if (dpy->im_fd_length <= POLLFD_CACHE_SIZE) {
	int loc = 1;
	for (conni = dpy->im_fd_info; conni; conni=conni->next) {
	    pfp[loc].fd = conni->fd;
	    pfp[loc].events = POLLIN;
	    loc++;
	}
    }
#endif
}

static void
_XWaitForReadable(dpy)
  Display *dpy;
{
    int result;
    int fd = dpy->fd;
    struct _XConnectionInfo *ilist;  
#ifdef USE_POLL
    struct pollfd *filedes;
#else
    FdSet r_mask;
    int highest_fd = fd;
#endif

#ifdef USE_POLL
    if (dpy->im_fd_length + 1 > POLLFD_CACHE_SIZE && !dpy->in_process_conni) {
	/* XXX - this fallback is gross */
	int i;

	filedes = (struct pollfd *)Xmalloc(dpy->im_fd_length * sizeof(struct pollfd));
	filedes[0].fd = fd;
	filedes[0].events = POLLIN;
	for (ilist=dpy->im_fd_info, i=1; ilist; ilist=ilist->next, i++) {
	    filedes[i].fd = ilist->fd;
	    filedes[i].events = POLLIN;
	}
    } else {
	filedes = (struct pollfd *)dpy->filedes;
    }
#else
    CLEARBITS(r_mask);
#endif
    for (;;) {
#ifndef USE_POLL
	BITSET(r_mask, fd);
	if (!dpy->in_process_conni)
	    for (ilist=dpy->im_fd_info; ilist; ilist=ilist->next) {
		BITSET(r_mask, ilist->fd);
		if (ilist->fd > highest_fd)
		    highest_fd = ilist->fd;
	    }
#endif
	UnlockDisplay(dpy);
#ifdef USE_POLL
	result = poll(filedes,
		      dpy->in_process_conni ? 1 : 1+dpy->im_fd_length,
		      -1);
#else
#ifdef WIN32
	result = select (0, &r_mask, NULL, NULL, NULL);
#else
	result = select(highest_fd + 1, r_mask, NULL, NULL, NULL);
#endif
#endif
	LockDisplay(dpy);
	if (result == -1 && !ECHECK(EINTR)) _XIOError(dpy);
	if (result <= 0)
	    continue;
#ifdef USE_POLL
	if (filedes[0].revents & (POLLIN|POLLHUP))
	    break;
	if (!dpy->in_process_conni) {
	    int i;
	    struct _XConnectionInfo *ilist;  

	    for (ilist=dpy->im_fd_info, i=1; ilist; ilist=ilist->next, i++) {
		if (filedes[i].revents & POLLIN) {
		    _XProcessInternalConnection(dpy, ilist);
		}
	    }
	    if (dpy->im_fd_length + 1 > POLLFD_CACHE_SIZE)
		Xfree(filedes);
	}
#else
	if (GETBIT(r_mask, fd))
	    break;
	if (!dpy->in_process_conni)
	    for (ilist=dpy->im_fd_info; ilist; ilist=ilist->next) {
		if (GETBIT(r_mask, ilist->fd)) {
		    _XProcessInternalConnection(dpy, ilist);
		}
	    }
#endif
    }
#ifdef XTHREADS
#ifdef XTHREADS_DEBUG
    printf("thread %x _XWaitForReadable returning\n", xthread_self());
#endif
#endif
}

#ifdef XTHREADS
static _XFlushInt();
#endif

/*
 * _XFlush - Flush the X request buffer.  If the buffer is empty, no
 * action is taken.  This routine correctly handles incremental writes.
 * This routine may have to be reworked if int < long.
 */
_XFlush (dpy)
	register Display *dpy;
{
#ifdef XTHREADS
    /* With multi-threading we introduce an internal routine to which
       we can pass a condition variable to do locking correctly. */

    _XFlushInt(dpy, NULL);
}

/* _XFlushInt - Internal version of _XFlush used to do multi-threaded
 * locking correctly.
 */

static _XFlushInt (dpy, cv)
	register Display *dpy;
        register xcondition_t cv;
{
#endif /* XTHREADS*/
	register long size, todo;
	register int write_stat;
	register char *bufindex;
	_XExtension *ext;

	if (dpy->flags & XlibDisplayIOError) return;

	size = todo = dpy->bufptr - dpy->buffer;
	bufindex = dpy->bufptr = dpy->buffer;
	for (ext = dpy->flushes; ext; ext = ext->next_flush)
	    (*ext->before_flush)(dpy, &ext->codes, bufindex, size);
	/*
	 * While write has not written the entire buffer, keep looping
	 * until the entire buffer is written.  bufindex will be incremented
	 * and size decremented as buffer is written out.
	 */
	while (size) {
	    ESET(0);
	    write_stat = WriteToServer(dpy->fd, bufindex, (int) todo);
	    if (write_stat >= 0) {
		size -= write_stat;
		todo = size;
		bufindex += write_stat;
	    } else if (ETEST()) {
#ifdef XTHREADS
		_XWaitForWritable(dpy, cv);
#else
		_XWaitForWritable(dpy);
#endif
#ifdef SUNSYSV
	    } else if (ECHECK(0)) {
#ifdef XTHREADS
		_XWaitForWritable(dpy, cv);
#else
		_XWaitForWritable(dpy);
#endif
#endif
#ifdef ESZTEST
	    } else if (ESZTEST()) {
		if (todo > 1) 
		    todo >>= 1;
		else {
#ifdef XTHREADS
		    _XWaitForWritable(dpy, cv);
#else
		    _XWaitForWritable(dpy);
#endif
		}
#endif
	    } else if (!ECHECK(EINTR)) {
		/* Write failed! */
		/* errno set by write system call. */
		_XIOError(dpy);
	    }
	}
	dpy->last_req = (char *)&_dummy_request;
	if (!dpy->savedsynchandler &&
	    (dpy->request - dpy->last_request_read) >= SEQLIMIT) {
	    dpy->savedsynchandler = dpy->synchandler;
	    dpy->synchandler = _XSyncFunction;
	}
}

int
_XEventsQueued (dpy, mode)
    register Display *dpy;
    int mode;
{
	register BytesReadable_t len;
	BytesReadable_t pend;
	_XAlignedBuffer buf;
	register xReply *rep;
	char *read_buf;
#ifdef XTHREADS
	int entry_event_serial_num;
	struct _XCVList *cvl;

#ifdef XTHREADS_DEBUG
	printf("_XEventsQueued called in thread %x\n", xthread_self());
#endif
#endif /* XTHREADS*/

	if (mode == QueuedAfterFlush)
	{
	    _XFlush(dpy);
	    if (dpy->qlen)
		return(dpy->qlen);
	}
	if (dpy->flags & XlibDisplayIOError) return(dpy->qlen);

#ifdef XTHREADS
	/* In the multi-threaded case, if there is someone else
	   reading events, then there aren't any available, so
	   we just return.  If we waited we would block.
	   */
	if (dpy->lock && dpy->lock->event_awaiters)
	    return dpy->qlen;

	/* nobody here but us, so lock out any newcomers */
	cvl = QueueEventReaderLock(dpy);

	/* note which events we have already seen so we'll know
	   if _XReply (in another thread) reads one */
	entry_event_serial_num = dpy->next_event_serial_num;

	if (dpy->lock && dpy->lock->reply_awaiters) { /* event_awaiters must have only us */
	    ConditionWait(dpy, cvl);
	}
	
	/* did _XReply read an event we can return? */
	if (dpy->next_event_serial_num > entry_event_serial_num) {
	    /* yes, find the event */
	    _XQEvent *qev = dpy->head;
	    while (qev) {
		if (qev->qserial_num > entry_event_serial_num) {
		    UnlockNextEventReader(dpy);
		    return;
		}
		qev = qev->next;
	    }
	}
#endif /* XTHREADS*/

	if (BytesReadable(dpy->fd, &pend) < 0)
	    _XIOError(dpy);
#ifdef XCONN_CHECK_FREQ
	/* This is a crock, required because FIONREAD or equivalent is
	 * not guaranteed to detect a broken connection.
	 */
	if (!pend && !dpy->qlen && ++dpy->conn_checker >= XCONN_CHECK_FREQ)
	{
#ifdef USE_POLL
	    struct pollfd filedes;
#else
	    FdSet r_mask;
	    static struct timeval zero_time;
#endif

	    dpy->conn_checker = 0;
#ifdef USE_POLL
	    filedes.fd = dpy->fd;
	    filedes.events = POLLIN;
	    if (pend = poll(&filedes, 1, 0))
#else
	    CLEARBITS(r_mask);
	    BITSET(r_mask, dpy->fd);
#ifdef WIN32
	    if (pend = select (0, &r_mask, NULL, NULL, &zero_time))
#else
	    if (pend = select(dpy->fd + 1, r_mask, NULL, NULL, &zero_time))
#endif
#endif
	    {
		if (pend > 0)
		{
		    if (BytesReadable(dpy->fd, &pend) < 0)
			_XIOError(dpy);
		    /* we should not get zero, if we do, force a read */
		    if (!pend)
			pend = SIZEOF(xReply);
		}
		else if (pend < 0 && !ECHECK(EINTR))
		    _XIOError(dpy);
	    }
	}
#endif /* XCONN_CHECK_FREQ */
	if (!(len = pend)) {
	    /* _XFlush can enqueue events */
	    UnlockNextEventReader(dpy);
	    return(dpy->qlen);
	}
      /* Force a read if there is not enough data.  Otherwise,
       * a select() loop at a higher-level will spin undesirably,
       * and we've seen at least one OS that appears to not update
       * the result from FIONREAD once it has returned nonzero.
       */
#ifdef XTHREADS
	if (dpy->lock && dpy->lock->reply_awaiters) {
	    read_buf = (char *)dpy->lock->reply_awaiters->buf;
	    len = SIZEOF(xReply);
	} else
#endif /* XTHREADS*/
	{
	    read_buf = buf.buf;
	    
	    if (len < SIZEOF(xReply))
		len = SIZEOF(xReply);
	    else if (len > BUFSIZE)
		len = BUFSIZE;
	    len = (len / SIZEOF(xReply)) * SIZEOF(xReply);
	}
#ifdef XCONN_CHECK_FREQ
	dpy->conn_checker = 0;
#endif

	_XRead (dpy, read_buf, (long) len);
	
#ifdef XTHREADS
	/* what did we actually read: reply or event? */
	if (dpy->lock && dpy->lock->reply_awaiters) {
	    if (((xReply *)read_buf)->generic.type == X_Reply)
	    {
		dpy->lock->reply_was_read = True;
		if (read_buf != (char *)dpy->lock->reply_awaiters->buf)
		    memcpy(dpy->lock->reply_awaiters->buf, read_buf,
			   len);
		ConditionSignal(dpy, dpy->lock->reply_awaiters);
		UnlockNextEventReader(dpy);
		return(dpy->qlen); /* we read, so we can return */
	    } else if (read_buf != buf.buf)
		memcpy(buf.buf, read_buf, len);
	}
#endif /* XTHREADS*/

	STARTITERATE(rep,xReply,buf.buf,len > 0) {
	    if (rep->generic.type == X_Reply) {
		pend = len;
		RESETITERPTR(rep,xReply,
			     _XAsyncReply (dpy, rep,
					   ITERPTR(rep), &pend, True));
		len = pend;
	    } else {
		if (rep->generic.type == X_Error)
		    _XError (dpy, (xError *)rep);
		else   /* must be an event packet */
		    _XEnq (dpy, (xEvent *)rep);
		INCITERPTR(rep,xReply);
		len -= SIZEOF(xReply);
	    }
	} ENDITERATE

	UnlockNextEventReader(dpy);
	return(dpy->qlen);
}

/* _XReadEvents - Flush the output queue,
 * then read as many events as possible (but at least 1) and enqueue them
 */
_XReadEvents(dpy)
	register Display *dpy;
{
	_XAlignedBuffer buf;
	BytesReadable_t pend;
	register BytesReadable_t len;
	register xReply *rep;
	Bool not_yet_flushed = True;
	char *read_buf;
#ifdef XTHREADS
	struct _XCVList *cvl = NULL;
	int entry_event_serial_num;
	Bool first_time = True;
	xthread_t self;

#ifdef XTHREADS_DEBUG
	printf("_XReadEvents called in thread %x\n",
	       xthread_self());
#endif
	/* create our condition variable and append to list,
	 * unless we were called from within XProcessInternalConnection
	 */
	xthread_clear_id(self);
	if (dpy->lock && xthread_have_id (dpy->lock->conni_thread))
	    /* some thread is in XProcessInternalConnection,
	       so we have to see if it is us */
	    self = XThread_Self();
	if (!xthread_have_id(self) || !xthread_equal(self, dpy->lock->conni_thread))
	    cvl = QueueEventReaderLock(dpy);

	/* note which events we have already seen so we'll know
	   if _XReply (in another thread) reads one */
	entry_event_serial_num = dpy->next_event_serial_num;
#endif /* XTHREADS*/

	do {
	    
#ifdef XTHREADS
	    /* if it is not our turn to read an event off the wire,
	       wait til we're at head of list */
	    if (first_time && dpy->lock && cvl &&
		(dpy->lock->event_awaiters != cvl || dpy->lock->reply_awaiters)) {
		ConditionWait(dpy, cvl);
	    }

	    /* did _XReply read an event we can return? */
	    if (dpy->next_event_serial_num > entry_event_serial_num) {
		/* yes, find the event */
		_XQEvent *qev = dpy->head;
		while (qev) {
		    if (qev->qserial_num > entry_event_serial_num) {
			goto got_event;
		    }
		    qev = qev->next;
		}
	    }
	    if (!first_time)
		ConditionWait(dpy, cvl);

	    first_time = False;
#endif /* XTHREADS*/

	    /* find out how much data can be read */
	    if (BytesReadable(dpy->fd, &pend) < 0)
	    	_XIOError(dpy);
	    len = pend;

	    /* must read at least one xEvent; if none is pending, then
	       we'll just flush and block waiting for it */
	    if (len < SIZEOF(xEvent)) {
	    	len = SIZEOF(xEvent);
		/* don't flush until the first time we would block */
		if (not_yet_flushed) {
		    int qlen = dpy->qlen;
		    _XFlush (dpy);
		    if (qlen != dpy->qlen) {
			/* _XReply has read an event for us */
			goto got_event;
		    }
		    not_yet_flushed = False;
		}
	    }

#ifdef XTHREADS
	    /* If someone is waiting for a reply, gamble that
	       the reply will be the next thing on the wire
	       and read it into their buffer. */
	    if (dpy->lock && dpy->lock->reply_awaiters) {
		read_buf = (char *)dpy->lock->reply_awaiters->buf;
		len = SIZEOF(xReply);
	    } else
#endif /* XTHREADS*/
	    {
		read_buf = buf.buf;

		/* but we won't read more than the max buffer size */
		if (len > BUFSIZE)
		    len = BUFSIZE;

		/* round down to an integral number of XReps */
		len = (len / SIZEOF(xEvent)) * SIZEOF(xEvent);
	    }

#ifdef XTHREADS
	    if (xthread_have_id(self))
		/* save value we may have to stick in conni_thread */
		dpy->lock->reading_thread = self;
#endif /* XTHREADS */
	    _XRead (dpy, read_buf, (long) len);
#ifdef XTHREADS
	    if (xthread_have_id(self))
		xthread_clear_id(dpy->lock->reading_thread);

	    /* what did we actually read: reply or event? */
	    if (dpy->lock && dpy->lock->reply_awaiters) {
		if (((xReply *)read_buf)->generic.type == X_Reply)
		{
		    dpy->lock->reply_was_read = True;
		    if (read_buf != (char *)dpy->lock->reply_awaiters->buf)
			memcpy(dpy->lock->reply_awaiters->buf,
			       read_buf, len);
		    ConditionSignal(dpy, dpy->lock->reply_awaiters);
		    /* useless to us, so keep trying */
		    continue;
		} else if (read_buf != buf.buf)
		    memcpy(buf.buf, read_buf, len);
	    }
#endif /* XTHREADS */

	    STARTITERATE(rep,xReply,buf.buf,len > 0) {
		if (rep->generic.type == X_Reply) {
		    pend = len;
		    RESETITERPTR(rep,xReply,
				 _XAsyncReply (dpy, rep,
					       ITERPTR(rep), &pend, True));
		    len = pend;
		} else {
		    if (rep->generic.type == X_Error)
			_XError (dpy, (xError *) rep);
		    else   /* must be an event packet */
			_XEnq (dpy, (xEvent *)rep);
		    INCITERPTR(rep,xReply);
		    len -= SIZEOF(xReply);
		}
	    } ENDITERATE
	} while (dpy->head == NULL);

    got_event:
	UnlockNextEventReader(dpy);
	DisplayLockWait(dpy);
}

/* 
 * _XRead - Read bytes from the socket taking into account incomplete
 * reads.  This routine may have to be reworked if int < long.
 */
_XRead (dpy, data, size)
	register Display *dpy;
	register char *data;
	register long size;
{
	register long bytes_read;
#ifdef XTHREADS
	int original_size = size;
#endif

	if ((dpy->flags & XlibDisplayIOError) || size == 0) return;
	ESET(0);
	while ((bytes_read = ReadFromServer(dpy->fd, data, (int)size))
		!= size) {

	    	if (bytes_read > 0) {
		    size -= bytes_read;
		    data += bytes_read;
		    }
		else if (ETEST()) {
		    _XWaitForReadable(dpy);
		    ESET(0);
		}
#ifdef SUNSYSV
		else if (ECHECK(0)) {
		    _XWaitForReadable(dpy);
		}
#endif
		else if (bytes_read == 0) {
		    /* Read failed because of end of file! */
		    ESET(EPIPE);
		    _XIOError(dpy);
		    }

		else  /* bytes_read is less than 0; presumably -1 */ {
		    /* If it's a system call interrupt, it's not an error. */
		    if (!ECHECK(EINTR))
		    	_XIOError(dpy);
		    }
	    	 }
#ifdef XTHREADS
       if (dpy->lock && dpy->lock->reply_bytes_left > 0)
       {
           dpy->lock->reply_bytes_left -= original_size;
           if (dpy->lock->reply_bytes_left == 0)
               UnlockNextReplyReader(dpy);
       }
#endif /* XTHREADS*/
}

#ifdef LONG64
_XRead32 (dpy, data, len)
    Display *dpy;
    register long *data;
    long len;
{
    register int *buf;
    register long i;

    if (len) {
	_XRead(dpy, (char *)data, len);
	i = len >> 2;
	buf = (int *)data + i;
	data += i;
	while (--i >= 0)
	    *data-- = *buf--;
    }
}
#endif /* LONG64 */

#ifdef WORD64

/*
 * XXX This is a *really* stupid way of doing this....
 * PACKBUFFERSIZE must be a multiple of 4.
 */

#define PACKBUFFERSIZE 4096


/*
 * _XRead32 - Read bytes from the socket unpacking each 32 bits
 *            into a long (64 bits on a CRAY computer).
 * 
 */
static _doXRead32 (dpy, data, size, packbuffer)
        register Display *dpy;
        register long *data;
        register long size;
	register char *packbuffer;
{
 long *lpack,*lp;
 long mask32 = 0x00000000ffffffff;
 long maskw, nwords, i, bits;

        _XReadPad (dpy, packbuffer, size);

        lp = data;
        lpack = (long *) packbuffer;
        nwords = size >> 2;
        bits = 32;

        for(i=0;i<nwords;i++){
            maskw = mask32 << bits;
           *lp++ = ( *lpack & maskw ) >> bits;
            bits = bits ^32;
            if(bits){
               lpack++;
            }
        }
}

_XRead32 (dpy, data, len)
    Display *dpy;
    long *data;
    long len;
{
    char packbuffer[PACKBUFFERSIZE];
    unsigned nunits = PACKBUFFERSIZE >> 2;

    for (; len > PACKBUFFERSIZE; len -= PACKBUFFERSIZE, data += nunits) {
	_doXRead32 (dpy, data, PACKBUFFERSIZE, packbuffer);
    }
    if (len) _doXRead32 (dpy, data, len, packbuffer);
}



/*
 * _XRead16 - Read bytes from the socket unpacking each 16 bits
 *            into a long (64 bits on a CRAY computer).
 *
 */
static _doXRead16 (dpy, data, size, packbuffer)
        register Display *dpy;
        register short *data;
        register long size;
	char *packbuffer;
{
	long *lpack,*lp;
	long mask16 = 0x000000000000ffff;
	long maskw, nwords, i, bits;

        _XRead(dpy,packbuffer,size);	/* don't do a padded read... */

        lp = (long *) data;
        lpack = (long *) packbuffer;
        nwords = size >> 1;  /* number of 16 bit words to be unpacked */
        bits = 48;
        for(i=0;i<nwords;i++){
            maskw = mask16 << bits;
           *lp++ = ( *lpack & maskw ) >> bits;
            bits -= 16;
            if(bits < 0){
               lpack++;
               bits = 48;
            }
        }
}

_XRead16 (dpy, data, len)
    Display *dpy;
    short *data;
    long len;
{
    char packbuffer[PACKBUFFERSIZE];
    unsigned nunits = PACKBUFFERSIZE >> 1;

    for (; len > PACKBUFFERSIZE; len -= PACKBUFFERSIZE, data += nunits) {
	_doXRead16 (dpy, data, PACKBUFFERSIZE, packbuffer);
    }
    if (len) _doXRead16 (dpy, data, len, packbuffer);
}

_XRead16Pad (dpy, data, size)
    Display *dpy;
    short *data;
    long size;
{
    int slop = (size & 3);
    short slopbuf[3];

    _XRead16 (dpy, data, size);
    if (slop > 0) {
	_XRead16 (dpy, slopbuf, 4 - slop);
    }
}
#endif /* WORD64 */


/*
 * _XReadPad - Read bytes from the socket taking into account incomplete
 * reads.  If the number of bytes is not 0 mod 4, read additional pad
 * bytes. This routine may have to be reworked if int < long.
 */
_XReadPad (dpy, data, size)
    	register Display *dpy;	
	register char *data;
	register long size;
{
    	register long bytes_read;
	struct iovec iov[2];
	char pad[3];
#ifdef XTHREADS
        int original_size;
#endif

	if ((dpy->flags & XlibDisplayIOError) || size == 0) return;
	iov[0].iov_len = (int)size;
	iov[0].iov_base = data;
	/* 
	 * The following hack is used to provide 32 bit long-word
	 * aligned padding.  The [1] vector is of length 0, 1, 2, or 3,
	 * whatever is needed.
	 */

	iov[1].iov_len = padlength[size & 3];
	iov[1].iov_base = pad;
	size += iov[1].iov_len;
#ifdef XTHREADS
	original_size = size;
#endif
	ESET(0);
	while ((bytes_read = ReadvFromServer (dpy->fd, iov, 2)) != size) {

	    if (bytes_read > 0) {
		size -= bytes_read;
	    	if ((iov[0].iov_len -= bytes_read) < 0) {
		    iov[1].iov_len += iov[0].iov_len;
		    iov[1].iov_base -= iov[0].iov_len;
		    iov[0].iov_len = 0;
		    }
	    	else
	    	    iov[0].iov_base += bytes_read;
	    	}
	    else if (ETEST()) {
		_XWaitForReadable(dpy);
		ESET(0);
	    }
#ifdef SUNSYSV
	    else if (ECHECK(0)) {
		_XWaitForReadable(dpy);
	    }
#endif
	    else if (bytes_read == 0) {
		/* Read failed because of end of file! */
		ESET(EPIPE);
		_XIOError(dpy);
		}
	    
	    else  /* bytes_read is less than 0; presumably -1 */ {
		/* If it's a system call interrupt, it's not an error. */
		if (!ECHECK(EINTR))
		    _XIOError(dpy);
		}
	    }
#ifdef XTHREADS
       if (dpy->lock && dpy->lock->reply_bytes_left > 0)
       {
           dpy->lock->reply_bytes_left -= original_size;
           if (dpy->lock->reply_bytes_left == 0)
               UnlockNextReplyReader(dpy);
       }
#endif /* XTHREADS*/
}

/*
 * _XSend - Flush the buffer and send the client data. 32 bit word aligned
 * transmission is used, if size is not 0 mod 4, extra bytes are transmitted.
 * This routine may have to be reworked if int < long;
 */
_XSend (dpy, data, size)
	register Display *dpy;
	char *data;
	register long size;
{
	struct iovec iov[3];
	static char pad[3] = {0, 0, 0};
           /* XText8 and XText16 require that the padding bytes be zero! */

	long skip = 0;
	long dpybufsize = (dpy->bufptr - dpy->buffer);
	long padsize = padlength[size & 3];
	long total = dpybufsize + size + padsize;
	long todo = total;
	_XExtension *ext;

	if (dpy->flags & XlibDisplayIOError) return;

	for (ext = dpy->flushes; ext; ext = ext->next_flush) {
	    (*ext->before_flush)(dpy, &ext->codes, dpy->bufptr, dpybufsize);
	    (*ext->before_flush)(dpy, &ext->codes, data, size);
	    if (padsize)
		(*ext->before_flush)(dpy, &ext->codes, pad, padsize);
	}

	/*
	 * There are 3 pieces that may need to be written out:
	 *
	 *     o  whatever is in the display buffer
	 *     o  the data passed in by the user
	 *     o  any padding needed to 32bit align the whole mess
	 *
	 * This loop looks at all 3 pieces each time through.  It uses skip
	 * to figure out whether or not a given piece is needed.
	 */
	while (total) {
	    long before = skip;		/* amount of whole thing written */
	    long remain = todo;		/* amount to try this time, <= total */
	    int i = 0;
	    long len;

	    /* You could be very general here and have "in" and "out" iovecs
	     * and write a loop without using a macro, but what the heck.  This
	     * translates to:
	     *
	     *     how much of this piece is new?
	     *     if more new then we are trying this time, clamp
	     *     if nothing new
	     *         then bump down amount already written, for next piece
	     *         else put new stuff in iovec, will need all of next piece
	     *
	     * Note that todo had better be at least 1 or else we'll end up
	     * writing 0 iovecs.
	     */
#define InsertIOV(pointer, length) \
	    len = (length) - before; \
	    if (len > remain) \
		len = remain; \
	    if (len <= 0) { \
		before = (-len); \
	    } else { \
		iov[i].iov_len = len; \
		iov[i].iov_base = (pointer) + before; \
		i++; \
		remain -= len; \
		before = 0; \
	    }

	    InsertIOV (dpy->buffer, dpybufsize)
	    InsertIOV (data, size)
	    InsertIOV (pad, padsize)
    
	    ESET(0);
	    if ((len = WritevToServer(dpy->fd, iov, i)) >= 0) {
		skip += len;
		total -= len;
		todo = total;
	    } else if (ETEST()) {
#ifdef XTHREADS
		_XWaitForWritable(dpy, NULL);
#else
		_XWaitForWritable(dpy);
#endif
#ifdef SUNSYSV
	    } else if (ECHECK(0)) {
#ifdef XTHREADS
		_XWaitForWritable(dpy, NULL);
#else
		_XWaitForWritable(dpy);
#endif
#endif
#ifdef ESZTEST
	    } else if (ESZTEST()) {
		if (todo > 1) 
		  todo >>= 1;
		else {
#ifdef XTHREADS
		    _XWaitForWritable(dpy, NULL);
#else
		    _XWaitForWritable(dpy);
#endif /* XTHREADS*/
		}
#endif
	    } else if (!ECHECK(EINTR)) {
		_XIOError(dpy);
	    }
	}

	dpy->bufptr = dpy->buffer;
	dpy->last_req = (char *) & _dummy_request;
	if (!dpy->savedsynchandler &&
	    (dpy->request - dpy->last_request_read) >= SEQLIMIT) {
	    dpy->savedsynchandler = dpy->synchandler;
	    dpy->synchandler = _XSyncFunction;
	}
}

/*
 * _XAllocID - normal resource ID allocation routine.  A client
 * can roll his own and instatantiate it if he wants, but must
 * follow the rules.
 */
XID _XAllocID(dpy)
register Display *dpy;
{
   XID id;

   id = dpy->resource_id << dpy->resource_shift;
   if (id <= dpy->resource_mask) {
       dpy->resource_id++;
       return (dpy->resource_base + id);
   }
   if (id != 0x10000000) {
       (void) fprintf(stderr,
		      "Xlib: resource ID allocation space exhausted!\n");
       id = 0x10000000;
       dpy->resource_id = id >> dpy->resource_shift;
   }
   return id;
}

/*
 * The hard part about this is that we only get 16 bits from a reply.
 * We have three values that will march along, with the following invariant:
 *	dpy->last_request_read <= rep->sequenceNumber <= dpy->request
 * We have to keep
 *	dpy->request - dpy->last_request_read < 2^16
 * or else we won't know for sure what value to use in events.  We do this
 * by forcing syncs when we get close.
 */

unsigned long
_XSetLastRequestRead(dpy, rep)
    register Display *dpy;
    register xGenericReply *rep;
{
    register unsigned long	newseq, lastseq;

    lastseq = dpy->last_request_read;
    /*
     * KeymapNotify has no sequence number, but is always guaranteed
     * to immediately follow another event, except when generated via
     * SendEvent (hmmm).
     */
    if ((rep->type & 0x7f) == KeymapNotify)
	return(lastseq);

    newseq = (lastseq & ~((unsigned long)0xffff)) | rep->sequenceNumber;

    if (newseq < lastseq) {
	newseq += 0x10000;
	if (newseq > dpy->request) {
	    (void) fprintf (stderr, 
	    "Xlib: sequence lost (0x%lx > 0x%lx) in reply type 0x%x!\n",
			    newseq, dpy->request, 
			    (unsigned int) rep->type);
	    newseq -= 0x10000;
	}
    }

    dpy->last_request_read = newseq;
    return(newseq);
}

/*
 * _XReply - Wait for a reply packet and copy its contents into the
 * specified rep.  Meanwhile we must handle error and event packets that
 * we may encounter.
 */
Status _XReply (dpy, rep, extra, discard)
    register Display *dpy;
    register xReply *rep;
    int extra;		/* number of 32-bit words expected after the reply */
    Bool discard;	/* should I discard data following "extra" words? */
{
    /* Pull out the serial number now, so that (currently illegal) requests
     * generated by an error handler don't confuse us.
     */
    unsigned long cur_request = dpy->request;
#ifdef XTHREADS
    struct _XCVList *cvl;
#endif

    if (dpy->flags & XlibDisplayIOError)
	return 0;

#ifdef XTHREADS
    /* create our condition variable and append to list */
    cvl = QueueReplyReaderLock(dpy);

#ifdef XTHREADS_DEBUG
    printf("_XReply called in thread %x, adding %x to cvl\n",
	   xthread_self(), cvl);
#endif

    _XFlushInt(dpy, cvl ? cvl->cv : NULL);

    /* if it is not our turn to read a reply off the wire,
       wait til we're at head of list */
    if(dpy->lock &&
       (dpy->lock->reply_awaiters != cvl  ||  dpy->lock->event_awaiters)) {
	cvl->buf = rep;
	ConditionWait(dpy, cvl);
    }
#else /* XTHREADS else */
    _XFlush(dpy);
#endif

    for (;;) {
#ifdef XTHREADS
	/* Did another thread's _XReadEvents get our reply by accident? */
	if (!dpy->lock || !dpy->lock->reply_was_read)
#endif
	    _XRead(dpy, (char *)rep, (long)SIZEOF(xReply));
#ifdef XTHREADS
	if (dpy->lock)
	    dpy->lock->reply_was_read = False;
#endif

	switch ((int)rep->generic.type) {

	    case X_Reply:
	        /* Reply received.  Fast update for synchronous replies,
		 * but deal with multiple outstanding replies.
		 */
	        if (rep->generic.sequenceNumber == (cur_request & 0xffff))
		    dpy->last_request_read = cur_request;
		else {
		    int pend = SIZEOF(xReply);
		    if (_XAsyncReply(dpy, rep, (char *)rep, &pend, False)
			!= (char *)rep)
			continue;
		}
		if (extra <= rep->generic.length) {
		    if (extra > 0)
			/* 
			 * Read the extra data into storage immediately
			 * following the GenericReply structure. 
			 */
			_XRead (dpy, (char *) (NEXTPTR(rep,xReply)),
				((long)extra) << 2);
		    if (discard) {
			if (extra < rep->generic.length)
			    _XEatData(dpy, (rep->generic.length - extra) << 2);
		    }
#ifdef XTHREADS
		    if (dpy->lock) {
			if (discard) {
			    dpy->lock->reply_bytes_left = 0;
			} else {
			    dpy->lock->reply_bytes_left =
				(rep->generic.length - extra) << 2;
			}
			if (dpy->lock->reply_bytes_left == 0) {
			    UnlockNextReplyReader(dpy);
			}
		    }
#endif
		    return 1;
		}
		/* 
		 *if we get here, then extra > rep->generic.length--meaning we
		 * read a reply that's shorter than we expected.  This is an 
		 * error,  but we still need to figure out how to handle it...
		 */
		_XRead (dpy, (char *) (NEXTPTR(rep,xReply)),
			((long) rep->generic.length) << 2);
		UnlockNextReplyReader(dpy);
		_XIOError (dpy);
		return (0);

    	    case X_Error:
	    	{
	        register _XExtension *ext;
		register Bool ret = False;
		int ret_code;
		xError *err = (xError *) rep;
		unsigned long serial;

		serial = _XSetLastRequestRead(dpy, (xGenericReply *)rep);
		if (serial == cur_request)
			/* do not die on "no such font", "can't allocate",
			   "can't grab" failures */
			switch ((int)err->errorCode) {
			case BadName:
			    switch (err->majorCode) {
				case X_LookupColor:
				case X_AllocNamedColor:
				    UnlockNextReplyReader(dpy);
				    return(0);
			    }
			    break;
			case BadFont:
			    if (err->majorCode == X_QueryFont) {
				UnlockNextReplyReader(dpy);
				return (0);
			    }
			    break;
			case BadAlloc:
			case BadAccess:
			    UnlockNextReplyReader(dpy);
			    return (0);
			}
		/* 
		 * we better see if there is an extension who may
		 * want to suppress the error.
		 */
		for (ext = dpy->ext_procs; !ret && ext; ext = ext->next) {
		    if (ext->error) 
		       ret = (*ext->error)(dpy, err, &ext->codes, &ret_code);
		}
		if (!ret) {
		    _XError(dpy, err);
		    ret_code = 0;
		}
		if (serial == cur_request) {
		    UnlockNextReplyReader(dpy);
		    return(ret_code);
		}

		} /* case X_Error */
		break;
	    default:
		_XEnq(dpy, (xEvent *) rep);
		break;
	    }
	}
}   

static char *
_XAsyncReply(dpy, rep, buf, lenp, discard)
    Display *dpy;
    register xReply *rep;
    char *buf;
    register int *lenp;
    Bool discard;
{
    register _XAsyncHandler *async, *next;
    register int len;
    register Bool consumed = False;
    char *nbuf;

    (void) _XSetLastRequestRead(dpy, &rep->generic);
    len = SIZEOF(xReply) + (rep->generic.length << 2);

    for (async = dpy->async_handlers; async; async = next) {
	next = async->next;
	if (consumed = (*async->handler)(dpy, rep, buf, *lenp, async->data))
	    break;
    }
    if (!consumed) {
	if (!discard)
	    return buf;
	(void) fprintf(stderr, 
		       "Xlib: unexpected async reply (sequence 0x%lx)!\n",
		       dpy->last_request_read);
#ifdef XTHREADS
#ifdef XTHREADS_DEBUG
	printf("thread %x, unexpected async reply\n", xthread_self());
#endif
#endif
	if (len > *lenp)
	    _XEatData(dpy, len - *lenp);
    }
    if (len >= *lenp) {
	buf += *lenp;
	*lenp = 0;
	return buf;
    }
    *lenp -= len;
    buf += len;
    len = *lenp;
    nbuf = buf;
    while (len > SIZEOF(xReply)) {
	if (*buf == X_Reply)
	    return nbuf;
	buf += SIZEOF(xReply);
	len -= SIZEOF(xReply);
    }
    if (len > 0 && len < SIZEOF(xReply)) {
	buf = nbuf;
	len = SIZEOF(xReply) - len;
	nbuf -= len;
	memmove(nbuf, buf, *lenp);
	_XRead(dpy, nbuf + *lenp, (long)len);
	*lenp += len;
    }
    return nbuf;
}

/*
 * Support for internal connections, such as an IM might use.
 * By Stephen Gildea, X Consortium, September 1993
 */

/* _XRegisterInternalConnection
 * Each IM (or Xlib extension) that opens a file descriptor that Xlib should
 * include in its select/poll mask must call this function to register the
 * fd with Xlib.  Any XConnectionWatchProc registered by XAddConnectionWatch
 * will also be called.
 *
 * Whenever Xlib detects input available on fd, it will call callback
 * with call_data to process it.  If non-Xlib code calls select/poll
 * and detects input available, it must call XProcessInternalConnection,
 * which will call the associated callback.
 *
 * Non-Xlib code can learn about these additional fds by calling
 * XInternalConnectionNumbers or, more typically, by registering
 * a XConnectionWatchProc with XAddConnectionWatch
 * to be called when fds are registered or unregistered.
 *
 * Returns True if registration succeeded, False if not, typically
 * because could not allocate memory.
 * Assumes Display locked when called.
 */
#if NeedFunctionPrototypes
Status _XRegisterInternalConnection(
    Display* dpy,
    int fd,
    _XInternalConnectionProc callback,
    XPointer call_data
)
#else
Status
_XRegisterInternalConnection(dpy, fd, callback, call_data)
    Display *dpy;
    int fd;
    _XInternalConnectionProc callback;
    XPointer call_data;
#endif
{
    struct _XConnectionInfo *new_conni;
    struct _XConnWatchInfo *watchers;
    XPointer *wd;

    new_conni = (struct _XConnectionInfo*)Xmalloc(sizeof(struct _XConnectionInfo));
    if (!new_conni)
	return 0;
    new_conni->watch_data = (XPointer *)Xmalloc(dpy->watcher_count * sizeof(XPointer));
    if (!new_conni->watch_data) {
	Xfree(new_conni);
	return 0;
    }
    new_conni->fd = fd;
    new_conni->read_callback = callback;
    new_conni->call_data = call_data;

    /* link new structure into list */
    new_conni->next = dpy->im_fd_info;
    dpy->im_fd_info = new_conni;
    dpy->im_fd_length++;
    _XPollfdCacheAdd(dpy, fd);

    for (watchers=dpy->conn_watchers, wd=new_conni->watch_data;
	 watchers;
	 watchers=watchers->next, wd++) {
	*wd = NULL;		/* for cleanliness */
	(*watchers->fn) (dpy, watchers->client_data, fd, True, wd);
    }

    return 1;
}

/* _XUnregisterInternalConnection
 * Each IM (or Xlib extension) that closes a file descriptor previously
 * registered with _XRegisterInternalConnection must call this function.
 * Any XConnectionWatchProc registered by XAddConnectionWatch
 * will also be called.
 *
 * Assumes Display locked when called.
 */
#if NeedFunctionPrototypes
void _XUnregisterInternalConnection(
    Display* dpy,
    int fd
)
#else
void
_XUnregisterInternalConnection(dpy, fd)
    Display *dpy;
    int fd;
#endif
{
    struct _XConnectionInfo *info_list, *previous=NULL;
    struct _XConnWatchInfo *watch;
    XPointer *wd;

    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next) {
	if (info_list->fd == fd) {
	    if (previous)
		previous->next = info_list->next;
	    else
		dpy->im_fd_info = info_list->next;
	    dpy->im_fd_length--;
	    for (watch=dpy->conn_watchers, wd=info_list->watch_data;
		 watch;
		 watch=watch->next, wd++) {
		(*watch->fn) (dpy, watch->client_data, fd, False, wd);
	    }
	    if (info_list->watch_data)
		Xfree (info_list->watch_data);
	    Xfree (info_list);
	    break;
	}
	previous = info_list;
    }
    _XPollfdCacheDel(dpy, fd);
}

/* XInternalConnectionNumbers
 * Returns an array of fds and an array of corresponding call data.
 * Typically a XConnectionWatchProc registered with XAddConnectionWatch
 * will be used instead of this function to discover
 * additional fds to include in the select/poll mask.
 *
 * The list is allocated with Xmalloc and should be freed by the caller
 * with Xfree;
 */
#if NeedFunctionPrototypes
Status XInternalConnectionNumbers(
    Display *dpy,
    int **fd_return,
    int *count_return
)
#else
Status
XInternalConnectionNumbers(dpy, fd_return, count_return)
    Display *dpy;
    int **fd_return;
    int *count_return;
#endif
{
    int count;
    struct _XConnectionInfo *info_list;
    int *fd_list;

    LockDisplay(dpy);
    count = 0;
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next)
	count++;
    fd_list = (int*) Xmalloc (count * sizeof(int));
    if (!fd_list) {
	UnlockDisplay(dpy);
	return 0;
    }
    count = 0;
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next) {
	fd_list[count] = info_list->fd;
	count++;
    }
    UnlockDisplay(dpy);

    *fd_return = fd_list;
    *count_return = count;
    return 1;
}

static void _XProcessInternalConnection(dpy, conn_info)
    Display *dpy;
    struct _XConnectionInfo *conn_info;
{
    dpy->in_process_conni = True;
#ifdef XTHREADS
    if (dpy->lock) {
	/* check cache to avoid call to thread_self */
	if (xthread_have_id(dpy->lock->reading_thread))
	    dpy->lock->conni_thread = dpy->lock->reading_thread;
	else
	    dpy->lock->conni_thread = XThread_Self();
    }
#endif /* XTHREADS */
    UnlockDisplay(dpy);
    (*conn_info->read_callback) (dpy, conn_info->fd, conn_info->call_data);
    LockDisplay(dpy);
#ifdef XTHREADS
    if (dpy->lock)
	xthread_clear_id(dpy->lock->conni_thread);
#endif /* XTHREADS */
    dpy->in_process_conni = False;
}

/* XProcessInternalConnection
 * Call the _XInternalConnectionProc registered by _XRegisterInternalConnection
 * for this fd.
 * The Display is NOT locked during the call.
 */
#if NeedFunctionPrototypes
void XProcessInternalConnection(
    Display* dpy,
    int fd
)
#else
void
XProcessInternalConnection(dpy, fd)
    Display *dpy;
    int fd;
#endif
{
    struct _XConnectionInfo *info_list;

    LockDisplay(dpy);
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next) {
	if (info_list->fd == fd) {
	    _XProcessInternalConnection(dpy, info_list);
	    break;
	}
    }
    UnlockDisplay(dpy);
}

/* XAddConnectionWatch
 * Register a callback to be called whenever _XRegisterInternalConnection
 * or _XUnregisterInternalConnection is called.
 * Callbacks are called with the Display locked.
 * If any connections are already registered, the callback is immediately
 * called for each of them.
 */
#if NeedFunctionPrototypes
Status XAddConnectionWatch(
    Display* dpy,
    XConnectionWatchProc callback,
    XPointer client_data
)
#else
Status
XAddConnectionWatch(dpy, callback, client_data)
    Display *dpy;
    XConnectionWatchProc callback;
    XPointer client_data;
#endif
{
    struct _XConnWatchInfo *new_watcher;
    struct _XConnectionInfo *info_list;
    XPointer *wd_array;

    new_watcher = (struct _XConnWatchInfo*)Xmalloc(sizeof(struct _XConnWatchInfo));
    if (!new_watcher)
	return 0;
    new_watcher->fn = callback;
    new_watcher->client_data = client_data;

    /* link new structure into list */
    LockDisplay(dpy);
    new_watcher->next = dpy->conn_watchers;
    dpy->conn_watchers = new_watcher;
    dpy->watcher_count++;

    /* call new watcher on all currently registered fds */
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next) {
	wd_array = (XPointer *)Xmalloc(dpy->watcher_count * sizeof(XPointer));
	if (!wd_array)
	    return 0;
	if (dpy->watcher_count > 1) {
	    memcpy(wd_array+1, info_list->watch_data, dpy->watcher_count-1);
	    Xfree(info_list->watch_data);
	}
	/* new element is at front of list now */
	*wd_array = NULL;	/* for cleanliness */
	info_list->watch_data = wd_array;

	(*callback) (dpy, client_data, info_list->fd, True, wd_array);
    }

    UnlockDisplay(dpy);
    return 1;
}

/* XRemoveConnectionWatch
 * Unregister a callback registered by XAddConnectionWatch.
 * Both callback and client_data must match what was passed to
 * XAddConnectionWatch.
 */ 
#if NeedFunctionPrototypes
void XRemoveConnectionWatch(
    Display* dpy,
    XConnectionWatchProc callback,
    XPointer client_data
)
#else
void
XRemoveConnectionWatch(dpy, callback, client_data)
    Display *dpy;
    XConnectionWatchProc callback;
    XPointer client_data;
#endif
{
    struct _XConnWatchInfo *watch;
    struct _XConnWatchInfo *previous = NULL;
    struct _XConnectionInfo *conni;
    int counter = 0;

    LockDisplay(dpy);
    for (watch=dpy->conn_watchers; watch; watch=watch->next) {
	if (watch->fn == callback  &&  watch->client_data == client_data) {
	    if (previous)
		previous->next = watch->next;
	    else
		dpy->conn_watchers = watch->next;
	    Xfree (watch);
	    dpy->watcher_count--;
	    /* remove our watch_data for each connection */
	    for (conni=dpy->im_fd_info; conni; conni=conni->next) {
		/* don't bother realloc'ing; these arrays are small anyway */
		/* overlapping */
		memmove(conni->watch_data+counter,
			conni->watch_data+counter+1,
			dpy->watcher_count - counter);
	    }
	    break;
	}
	previous = watch;
	counter++;
    }
    UnlockDisplay(dpy);
}

/* end of internal connections support */


/* Read and discard "n" 8-bit bytes of data */

void _XEatData (dpy, n)
    Display *dpy;
    register unsigned long n;
{
#define SCRATCHSIZE 2048
    char buf[SCRATCHSIZE];

    while (n > 0) {
	register long bytes_read = (n > SCRATCHSIZE) ? SCRATCHSIZE : n;
	_XRead (dpy, buf, bytes_read);
	n -= bytes_read;
    }
#undef SCRATCHSIZE
}


/*
 * _XEnq - Place event packets on the display's queue.
 * note that no squishing of move events in V11, since there
 * is pointer motion hints....
 */
_XEnq (dpy, event)
	register Display *dpy;
	register xEvent *event;
{
	register _XQEvent *qelt;

	if ((qelt = dpy->qfree)) {
		/* If dpy->qfree is non-NULL do this, else malloc a new one. */
		dpy->qfree = qelt->next;
	}
	else if ((qelt = 
	    (_XQEvent *) Xmalloc((unsigned)sizeof(_XQEvent))) == NULL) {
		/* Malloc call failed! */
		ESET(ENOMEM);
		_XIOError(dpy);
	}
	qelt->next = NULL;
	/* go call through display to find proper event reformatter */
	if ((*dpy->event_vec[event->u.u.type & 0177])(dpy, &qelt->event, event)) {
	    qelt->qserial_num = dpy->next_event_serial_num++;
	    if (dpy->tail)	dpy->tail->next = qelt;
	    else 		dpy->head = qelt;
    
	    dpy->tail = qelt;
	    dpy->qlen++;
	} else {
	    /* ignored, or stashed away for many-to-one compression */
	    qelt->next = dpy->qfree;
	    dpy->qfree = qelt;
	}
}

/*
 * _XDeq - Remove event packet from the display's queue.
 */
_XDeq (dpy, prev, qelt)
    register Display *dpy;
    register _XQEvent *prev;	/* element before qelt */
    register _XQEvent *qelt;	/* element to be unlinked */
{
    if (prev) {
	if ((prev->next = qelt->next) == NULL)
	    dpy->tail = prev;
    } else {
	/* no prev, so removing first elt */
	if ((dpy->head = qelt->next) == NULL)
	    dpy->tail = NULL;
    }
    qelt->qserial_num = 0;
    qelt->next = dpy->qfree;
    dpy->qfree = qelt;
    dpy->qlen--;
}

/*
 * EventToWire in separate file in that often not needed.
 */

/*ARGSUSED*/
Bool
_XUnknownWireEvent(dpy, re, event)
register Display *dpy;	/* pointer to display structure */
register XEvent *re;	/* pointer to where event should be reformatted */
register xEvent *event;	/* wire protocol event */
{
#ifdef notdef
	(void) fprintf(stderr, 
	    "Xlib: unhandled wire event! event number = %d, display = %x\n.",
			event->u.u.type, dpy);
#endif
	return(False);
}

/*ARGSUSED*/
Status
_XUnknownNativeEvent(dpy, re, event)
register Display *dpy;	/* pointer to display structure */
register XEvent *re;	/* pointer to where event should be reformatted */
register xEvent *event;	/* wire protocol event */
{
#ifdef notdef
	(void) fprintf(stderr, 
 	   "Xlib: unhandled native event! event number = %d, display = %x\n.",
			re->type, dpy);
#endif
	return(0);
}
/*
 * reformat a wire event into an XEvent structure of the right type.
 */
Bool
_XWireToEvent(dpy, re, event)
register Display *dpy;	/* pointer to display structure */
register XEvent *re;	/* pointer to where event should be reformatted */
register xEvent *event;	/* wire protocol event */
{

	re->type = event->u.u.type & 0x7f;
	((XAnyEvent *)re)->serial = _XSetLastRequestRead(dpy,
					(xGenericReply *)event);
	((XAnyEvent *)re)->send_event = ((event->u.u.type & 0x80) != 0);
	((XAnyEvent *)re)->display = dpy;
	
	/* Ignore the leading bit of the event type since it is set when a
		client sends an event rather than the server. */

	switch (event-> u.u.type & 0177) {
	      case KeyPress:
	      case KeyRelease:
	        {
			register XKeyEvent *ev = (XKeyEvent*) re;
			ev->root 	= event->u.keyButtonPointer.root;
			ev->window 	= event->u.keyButtonPointer.event;
			ev->subwindow 	= event->u.keyButtonPointer.child;
			ev->time 	= event->u.keyButtonPointer.time;
			ev->x 		= cvtINT16toInt(event->u.keyButtonPointer.eventX);
			ev->y 		= cvtINT16toInt(event->u.keyButtonPointer.eventY);
			ev->x_root 	= cvtINT16toInt(event->u.keyButtonPointer.rootX);
			ev->y_root 	= cvtINT16toInt(event->u.keyButtonPointer.rootY);
			ev->state	= event->u.keyButtonPointer.state;
			ev->same_screen	= event->u.keyButtonPointer.sameScreen;
			ev->keycode 	= event->u.u.detail;
		}
	      	break;
	      case ButtonPress:
	      case ButtonRelease:
	        {
			register XButtonEvent *ev =  (XButtonEvent *) re;
			ev->root 	= event->u.keyButtonPointer.root;
			ev->window 	= event->u.keyButtonPointer.event;
			ev->subwindow 	= event->u.keyButtonPointer.child;
			ev->time 	= event->u.keyButtonPointer.time;
			ev->x 		= cvtINT16toInt(event->u.keyButtonPointer.eventX);
			ev->y 		= cvtINT16toInt(event->u.keyButtonPointer.eventY);
			ev->x_root 	= cvtINT16toInt(event->u.keyButtonPointer.rootX);
			ev->y_root 	= cvtINT16toInt(event->u.keyButtonPointer.rootY);
			ev->state	= event->u.keyButtonPointer.state;
			ev->same_screen	= event->u.keyButtonPointer.sameScreen;
			ev->button 	= event->u.u.detail;
		}
	        break;
	      case MotionNotify:
	        {
			register XMotionEvent *ev =   (XMotionEvent *)re;
			ev->root 	= event->u.keyButtonPointer.root;
			ev->window 	= event->u.keyButtonPointer.event;
			ev->subwindow 	= event->u.keyButtonPointer.child;
			ev->time 	= event->u.keyButtonPointer.time;
			ev->x 		= cvtINT16toInt(event->u.keyButtonPointer.eventX);
			ev->y 		= cvtINT16toInt(event->u.keyButtonPointer.eventY);
			ev->x_root 	= cvtINT16toInt(event->u.keyButtonPointer.rootX);
			ev->y_root 	= cvtINT16toInt(event->u.keyButtonPointer.rootY);
			ev->state	= event->u.keyButtonPointer.state;
			ev->same_screen	= event->u.keyButtonPointer.sameScreen;
			ev->is_hint 	= event->u.u.detail;
		}
	        break;
	      case EnterNotify:
	      case LeaveNotify:
		{
			register XCrossingEvent *ev   = (XCrossingEvent *) re;
			ev->root	= event->u.enterLeave.root;
			ev->window	= event->u.enterLeave.event;
			ev->subwindow	= event->u.enterLeave.child;
			ev->time	= event->u.enterLeave.time;
			ev->x		= cvtINT16toInt(event->u.enterLeave.eventX);
			ev->y		= cvtINT16toInt(event->u.enterLeave.eventY);
			ev->x_root	= cvtINT16toInt(event->u.enterLeave.rootX);
			ev->y_root	= cvtINT16toInt(event->u.enterLeave.rootY);
			ev->state	= event->u.enterLeave.state;
			ev->mode	= event->u.enterLeave.mode;
			ev->same_screen = (event->u.enterLeave.flags & 
				ELFlagSameScreen) && True;
			ev->focus	= (event->u.enterLeave.flags &
			  	ELFlagFocus) && True;
			ev->detail	= event->u.u.detail;
		}
		  break;
	      case FocusIn:
	      case FocusOut:
		{
			register XFocusChangeEvent *ev = (XFocusChangeEvent *) re;
			ev->window 	= event->u.focus.window;
			ev->mode	= event->u.focus.mode;
			ev->detail	= event->u.u.detail;
		}
		  break;
	      case KeymapNotify:
		{
			register XKeymapEvent *ev = (XKeymapEvent *) re;
			ev->window	= None;
			memcpy(&ev->key_vector[1],
			       (char *)((xKeymapEvent *) event)->map,
			       sizeof (((xKeymapEvent *) event)->map));
		}
		break;
	      case Expose:
		{
			register XExposeEvent *ev = (XExposeEvent *) re;
			ev->window	= event->u.expose.window;
			ev->x		= event->u.expose.x;
			ev->y		= event->u.expose.y;
			ev->width	= event->u.expose.width;
			ev->height	= event->u.expose.height;
			ev->count	= event->u.expose.count;
		}
		break;
	      case GraphicsExpose:
		{
		    register XGraphicsExposeEvent *ev =
			(XGraphicsExposeEvent *) re;
		    ev->drawable	= event->u.graphicsExposure.drawable;
		    ev->x		= event->u.graphicsExposure.x;
		    ev->y		= event->u.graphicsExposure.y;
		    ev->width		= event->u.graphicsExposure.width;
		    ev->height		= event->u.graphicsExposure.height;
		    ev->count		= event->u.graphicsExposure.count;
		    ev->major_code	= event->u.graphicsExposure.majorEvent;
		    ev->minor_code	= event->u.graphicsExposure.minorEvent;
		}
		break;
	      case NoExpose:
		{
		    register XNoExposeEvent *ev = (XNoExposeEvent *) re;
		    ev->drawable	= event->u.noExposure.drawable;
		    ev->major_code	= event->u.noExposure.majorEvent;
		    ev->minor_code	= event->u.noExposure.minorEvent;
		}
		break;
	      case VisibilityNotify:
		{
		    register XVisibilityEvent *ev = (XVisibilityEvent *) re;
		    ev->window		= event->u.visibility.window;
		    ev->state		= event->u.visibility.state;
		}
		break;
	      case CreateNotify:
		{
		    register XCreateWindowEvent *ev =
			 (XCreateWindowEvent *) re;
		    ev->window		= event->u.createNotify.window;
		    ev->parent		= event->u.createNotify.parent;
		    ev->x		= cvtINT16toInt(event->u.createNotify.x);
		    ev->y		= cvtINT16toInt(event->u.createNotify.y);
		    ev->width		= event->u.createNotify.width;
		    ev->height		= event->u.createNotify.height;
		    ev->border_width	= event->u.createNotify.borderWidth;
		    ev->override_redirect	= event->u.createNotify.override;
		}
		break;
	      case DestroyNotify:
		{
		    register XDestroyWindowEvent *ev =
				(XDestroyWindowEvent *) re;
		    ev->window		= event->u.destroyNotify.window;
		    ev->event		= event->u.destroyNotify.event;
		}
		break;
	      case UnmapNotify:
		{
		    register XUnmapEvent *ev = (XUnmapEvent *) re;
		    ev->window		= event->u.unmapNotify.window;
		    ev->event		= event->u.unmapNotify.event;
		    ev->from_configure	= event->u.unmapNotify.fromConfigure;
		}
		break;
	      case MapNotify:
		{
		    register XMapEvent *ev = (XMapEvent *) re;
		    ev->window		= event->u.mapNotify.window;
		    ev->event		= event->u.mapNotify.event;
		    ev->override_redirect	= event->u.mapNotify.override;
		}
		break;
	      case MapRequest:
		{
		    register XMapRequestEvent *ev = (XMapRequestEvent *) re;
		    ev->window		= event->u.mapRequest.window;
		    ev->parent		= event->u.mapRequest.parent;
		}
		break;
	      case ReparentNotify:
		{
		    register XReparentEvent *ev = (XReparentEvent *) re;
		    ev->event		= event->u.reparent.event;
		    ev->window		= event->u.reparent.window;
		    ev->parent		= event->u.reparent.parent;
		    ev->x		= cvtINT16toInt(event->u.reparent.x);
		    ev->y		= cvtINT16toInt(event->u.reparent.y);
		    ev->override_redirect	= event->u.reparent.override;
		}
		break;
	      case ConfigureNotify:
		{
		    register XConfigureEvent *ev = (XConfigureEvent *) re;
		    ev->event	= event->u.configureNotify.event;
		    ev->window	= event->u.configureNotify.window;
		    ev->above	= event->u.configureNotify.aboveSibling;
		    ev->x	= cvtINT16toInt(event->u.configureNotify.x);
		    ev->y	= cvtINT16toInt(event->u.configureNotify.y);
		    ev->width	= event->u.configureNotify.width;
		    ev->height	= event->u.configureNotify.height;
		    ev->border_width  = event->u.configureNotify.borderWidth;
		    ev->override_redirect = event->u.configureNotify.override;
		}
		break;
	      case ConfigureRequest:
		{
		    register XConfigureRequestEvent *ev =
		        (XConfigureRequestEvent *) re;
		    ev->window		= event->u.configureRequest.window;
		    ev->parent		= event->u.configureRequest.parent;
		    ev->above		= event->u.configureRequest.sibling;
		    ev->x		= cvtINT16toInt(event->u.configureRequest.x);
		    ev->y		= cvtINT16toInt(event->u.configureRequest.y);
		    ev->width		= event->u.configureRequest.width;
		    ev->height		= event->u.configureRequest.height;
		    ev->border_width	= event->u.configureRequest.borderWidth;
		    ev->value_mask	= event->u.configureRequest.valueMask;
		    ev->detail  	= event->u.u.detail;
		}
		break;
	      case GravityNotify:
		{
		    register XGravityEvent *ev = (XGravityEvent *) re;
		    ev->window		= event->u.gravity.window;
		    ev->event		= event->u.gravity.event;
		    ev->x		= cvtINT16toInt(event->u.gravity.x);
		    ev->y		= cvtINT16toInt(event->u.gravity.y);
		}
		break;
	      case ResizeRequest:
		{
		    register XResizeRequestEvent *ev =
			(XResizeRequestEvent *) re;
		    ev->window		= event->u.resizeRequest.window;
		    ev->width		= event->u.resizeRequest.width;
		    ev->height		= event->u.resizeRequest.height;
		}
		break;
	      case CirculateNotify:
		{
		    register XCirculateEvent *ev = (XCirculateEvent *) re;
		    ev->window		= event->u.circulate.window;
		    ev->event		= event->u.circulate.event;
		    ev->place		= event->u.circulate.place;
		}
		break;
	      case CirculateRequest:
		{
		    register XCirculateRequestEvent *ev =
		        (XCirculateRequestEvent *) re;
		    ev->window		= event->u.circulate.window;
		    ev->parent		= event->u.circulate.event;
		    ev->place		= event->u.circulate.place;
		}
		break;
	      case PropertyNotify:
		{
		    register XPropertyEvent *ev = (XPropertyEvent *) re;
		    ev->window		= event->u.property.window;
		    ev->atom		= event->u.property.atom;
		    ev->time		= event->u.property.time;
		    ev->state		= event->u.property.state;
		}
		break;
	      case SelectionClear:
		{
		    register XSelectionClearEvent *ev =
			 (XSelectionClearEvent *) re;
		    ev->window		= event->u.selectionClear.window;
		    ev->selection	= event->u.selectionClear.atom;
		    ev->time		= event->u.selectionClear.time;
		}
		break;
	      case SelectionRequest:
		{
		    register XSelectionRequestEvent *ev =
		        (XSelectionRequestEvent *) re;
		    ev->owner		= event->u.selectionRequest.owner;
		    ev->requestor	= event->u.selectionRequest.requestor;
		    ev->selection	= event->u.selectionRequest.selection;
		    ev->target		= event->u.selectionRequest.target;
		    ev->property	= event->u.selectionRequest.property;
		    ev->time		= event->u.selectionRequest.time;
		}
		break;
	      case SelectionNotify:
		{
		    register XSelectionEvent *ev = (XSelectionEvent *) re;
		    ev->requestor	= event->u.selectionNotify.requestor;
		    ev->selection	= event->u.selectionNotify.selection;
		    ev->target		= event->u.selectionNotify.target;
		    ev->property	= event->u.selectionNotify.property;
		    ev->time		= event->u.selectionNotify.time;
		}
		break;
	      case ColormapNotify:
		{
		    register XColormapEvent *ev = (XColormapEvent *) re;
		    ev->window		= event->u.colormap.window;
		    ev->colormap	= event->u.colormap.colormap;
		    ev->new		= event->u.colormap.new;
		    ev->state		= event->u.colormap.state;
	        }
		break;
	      case ClientMessage:
		{
		   register int i;
		   register XClientMessageEvent *ev 
		   			= (XClientMessageEvent *) re;
		   ev->window		= event->u.clientMessage.window;
		   ev->format		= event->u.u.detail;
		   switch (ev->format) {
			case 8:	
			   ev->message_type = event->u.clientMessage.u.b.type;
			   for (i = 0; i < 20; i++) 	
			     ev->data.b[i] = event->u.clientMessage.u.b.bytes[i];
			   break;
			case 16:
			   ev->message_type = event->u.clientMessage.u.s.type;
			   ev->data.s[0] = cvtINT16toShort(event->u.clientMessage.u.s.shorts0);
			   ev->data.s[1] = cvtINT16toShort(event->u.clientMessage.u.s.shorts1);
			   ev->data.s[2] = cvtINT16toShort(event->u.clientMessage.u.s.shorts2);
			   ev->data.s[3] = cvtINT16toShort(event->u.clientMessage.u.s.shorts3);
			   ev->data.s[4] = cvtINT16toShort(event->u.clientMessage.u.s.shorts4);
			   ev->data.s[5] = cvtINT16toShort(event->u.clientMessage.u.s.shorts5);
			   ev->data.s[6] = cvtINT16toShort(event->u.clientMessage.u.s.shorts6);
			   ev->data.s[7] = cvtINT16toShort(event->u.clientMessage.u.s.shorts7);
			   ev->data.s[8] = cvtINT16toShort(event->u.clientMessage.u.s.shorts8);
			   ev->data.s[9] = cvtINT16toShort(event->u.clientMessage.u.s.shorts9);
			   break;
			case 32:
			   ev->message_type = event->u.clientMessage.u.l.type;
			   ev->data.l[0] = cvtINT32toLong(event->u.clientMessage.u.l.longs0);
			   ev->data.l[1] = cvtINT32toLong(event->u.clientMessage.u.l.longs1);
			   ev->data.l[2] = cvtINT32toLong(event->u.clientMessage.u.l.longs2);
			   ev->data.l[3] = cvtINT32toLong(event->u.clientMessage.u.l.longs3);
			   ev->data.l[4] = cvtINT32toLong(event->u.clientMessage.u.l.longs4);
			   break;
			default: /* XXX should never occur */
				break;
		    }
	        }
		break;
	      case MappingNotify:
		{
		   register XMappingEvent *ev = (XMappingEvent *)re;
		   ev->window		= 0;
		   ev->first_keycode 	= event->u.mappingNotify.firstKeyCode;
		   ev->request 		= event->u.mappingNotify.request;
		   ev->count 		= event->u.mappingNotify.count;
		}
		break;
	      default:
		return(_XUnknownWireEvent(dpy, re, event));
	}
	return(True);
}


#ifndef USL_SHARELIB

static char *_SysErrorMsg (n)
    int n;
{
#ifndef WIN32
    extern char *sys_errlist[];
    extern int sys_nerr;
#endif
    char *s = ((n >= 0 && n < sys_nerr) ? sys_errlist[n] : "unknown error");

    return (s ? s : "no such error");
}

#endif 	/* USL sharedlibs in don't define for SVR3.2 */


/*
 * _XDefaultIOError - Default fatal system error reporting routine.  Called 
 * when an X internal system error is encountered.
 */
_XDefaultIOError (dpy)
	Display *dpy;
{
	if (ECHECK(EPIPE)) {
	    (void) fprintf (stderr,
	"X connection to %s broken (explicit kill or server shutdown).\r\n",
			    DisplayString (dpy));
	} else {
	    (void) fprintf (stderr, 
			"XIO:  fatal IO error %d (%s) on X server \"%s\"\r\n",
#ifdef WIN32
			WSAGetLastError(), strerror(WSAGetLastError()),
#else
			errno, _SysErrorMsg (errno),
#endif
			DisplayString (dpy));
	    (void) fprintf (stderr, 
	 "      after %lu requests (%lu known processed) with %d events remaining.\r\n",
			NextRequest(dpy) - 1, LastKnownRequestProcessed(dpy),
			QLength(dpy));

	}
	exit(1);
}


static int _XPrintDefaultError (dpy, event, fp)
    Display *dpy;
    XErrorEvent *event;
    FILE *fp;
{
    char buffer[BUFSIZ];
    char mesg[BUFSIZ];
    char number[32];
    char *mtype = "XlibMessage";
    register _XExtension *ext = (_XExtension *)NULL;
    _XExtension *bext = (_XExtension *)NULL;
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
    (void) fprintf(fp, "%s:  %s\n  ", mesg, buffer);
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", 
	mesg, BUFSIZ);
    (void) fprintf(fp, mesg, event->request_code);
    if (event->request_code < 128) {
	sprintf(number, "%d", event->request_code);
	XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, BUFSIZ);
    } else {
	for (ext = dpy->ext_procs;
	     ext && (ext->codes.major_opcode != event->request_code);
	     ext = ext->next)
	  ;
	if (ext)
	    strcpy(buffer, ext->name);
	else
	    buffer[0] = '\0';
    }
    (void) fprintf(fp, " (%s)\n", buffer);
    if (event->request_code >= 128) {
	XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code %d",
			      mesg, BUFSIZ);
	fputs("  ", fp);
	(void) fprintf(fp, mesg, event->minor_code);
	if (ext) {
	    sprintf(mesg, "%s.%d", ext->name, event->minor_code);
	    XGetErrorDatabaseText(dpy, "XRequest", mesg, "", buffer, BUFSIZ);
	    (void) fprintf(fp, " (%s)", buffer);
	}
	fputs("\n", fp);
    }
    if (event->error_code >= 128) {
	/* kludge, try to find the extension that caused it */
	buffer[0] = '\0';
	for (ext = dpy->ext_procs; ext; ext = ext->next) {
	    if (ext->error_string) 
		(*ext->error_string)(dpy, event->error_code, &ext->codes,
				     buffer, BUFSIZ);
	    if (buffer[0]) {
		bext = ext;
		break;
	    }
	    if (ext->codes.first_error &&
		ext->codes.first_error < event->error_code &&
		(!bext || ext->codes.first_error > bext->codes.first_error))
		bext = ext;
	}    
	if (bext)
	    sprintf(buffer, "%s.%d", bext->name,
		    event->error_code - bext->codes.first_error);
	else
	    strcpy(buffer, "Value");
	XGetErrorDatabaseText(dpy, mtype, buffer, "", mesg, BUFSIZ);
	if (mesg[0]) {
	    fputs("  ", fp);
	    (void) fprintf(fp, mesg, event->resourceid);
	    fputs("\n", fp);
	}
	/* let extensions try to print the values */
	for (ext = dpy->ext_procs; ext; ext = ext->next) {
	    if (ext->error_values)
		(*ext->error_values)(dpy, event, fp);
	}
    } else if ((event->error_code == BadWindow) ||
	       (event->error_code == BadPixmap) ||
	       (event->error_code == BadCursor) ||
	       (event->error_code == BadFont) ||
	       (event->error_code == BadDrawable) ||
	       (event->error_code == BadColor) ||
	       (event->error_code == BadGC) ||
	       (event->error_code == BadIDChoice) ||
	       (event->error_code == BadValue) ||
	       (event->error_code == BadAtom)) {
	if (event->error_code == BadValue)
	    XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x",
				  mesg, BUFSIZ);
	else if (event->error_code == BadAtom)
	    XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x",
				  mesg, BUFSIZ);
	else
	    XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
				  mesg, BUFSIZ);
	fputs("  ", fp);
	(void) fprintf(fp, mesg, event->resourceid);
	fputs("\n", fp);
    }
    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", 
			  mesg, BUFSIZ);
    fputs("  ", fp);
    (void) fprintf(fp, mesg, event->serial);
    XGetErrorDatabaseText(dpy, mtype, "CurrentSerial", "Current Serial #%d",
			  mesg, BUFSIZ);
    fputs("\n  ", fp);
    (void) fprintf(fp, mesg, dpy->request);
    fputs("\n", fp);
    if (event->error_code == BadImplementation) return 0;
    return 1;
}

int _XDefaultError(dpy, event)
	Display *dpy;
	XErrorEvent *event;
{
    if (_XPrintDefaultError (dpy, event, stderr) == 0) return 0;
    exit(1);
    /*NOTREACHED*/
}

/*ARGSUSED*/
Bool _XDefaultWireError(display, he, we)
    Display     *display;
    XErrorEvent *he;
    xError      *we;
{
    return True;
}

/*
 * _XError - upcall internal or user protocol error handler
 */
int _XError (dpy, rep)
    Display *dpy;
    register xError *rep;
{
    /* 
     * X_Error packet encountered!  We need to unpack the error before
     * giving it to the user.
     */
    XEvent event; /* make it a large event */
    register _XAsyncHandler *async, *next;

    event.xerror.serial = _XSetLastRequestRead(dpy, (xGenericReply *)rep);

    for (async = dpy->async_handlers; async; async = next) {
	next = async->next;
	if ((*async->handler)(dpy, (xReply *)rep,
			      (char *)rep, SIZEOF(xError), async->data))
	    return 0;
    }

    event.xerror.display = dpy;
    event.xerror.type = X_Error;
    event.xerror.resourceid = rep->resourceID;
    event.xerror.error_code = rep->errorCode;
    event.xerror.request_code = rep->majorCode;
    event.xerror.minor_code = rep->minorCode;
    if (dpy->error_vec &&
	!(*dpy->error_vec[rep->errorCode])(dpy, &event.xerror, rep))
	return 0;
    if (_XErrorFunction != NULL) {
      	return ((*_XErrorFunction)(dpy, &event));	/* upcall */
    } else {
	return _XDefaultError(dpy, &event);
    }
}
    
/*
 * _XIOError - call user connection error handler and exit
 */
int _XIOError (dpy)
    Display *dpy;
{
    dpy->flags |= XlibDisplayIOError;
#ifdef WIN32
    errno = WSAGetLastError();
#endif

    if (_XIOErrorFunction != NULL)
	(*_XIOErrorFunction)(dpy);
    else
	_XDefaultIOError(dpy);
    exit (1);
}


/*
 * This routine can be used to (cheaply) get some memory within a single
 * Xlib routine for scratch space.  It is reallocated from the same place
 * each time, unless the library needs a large scratch space.
 */
char *_XAllocScratch (dpy, nbytes)
	register Display *dpy;
	unsigned long nbytes;
{
	if (nbytes > dpy->scratch_length) {
	    if (dpy->scratch_buffer) Xfree (dpy->scratch_buffer);
	    if (dpy->scratch_buffer = Xmalloc((unsigned) nbytes))
		dpy->scratch_length = nbytes;
	    else dpy->scratch_length = 0;
	}
	return (dpy->scratch_buffer);
}

/*
 * Given a visual id, find the visual structure for this id on this display.
 */
Visual *_XVIDtoVisual (dpy, id)
	Display *dpy;
	VisualID id;
{
	register int i, j, k;
	register Screen *sp;
	register Depth *dp;
	register Visual *vp;
	for (i = 0; i < dpy->nscreens; i++) {
		sp = &dpy->screens[i];
		for (j = 0; j < sp->ndepths; j++) {
			dp = &sp->depths[j];
			/* if nvisuals == 0 then visuals will be NULL */
			for (k = 0; k < dp->nvisuals; k++) {
				vp = &dp->visuals[k];
				if (vp->visualid == id) return (vp);
			}
		}
	}
	return (NULL);
}

#if NeedFunctionPrototypes
XFree (void *data)
#else
XFree (data)
	char *data;
#endif
{
	Xfree (data);
}

#ifdef _XNEEDBCOPYFUNC
void _Xbcopy(b1, b2, length)
    register char *b1, *b2;
    register length;
{
    if (b1 < b2) {
	b2 += length;
	b1 += length;
	while (length--)
	    *--b2 = *--b1;
    } else {
	while (length--)
	    *b2++ = *b1++;
    }
}
#endif

#ifdef DataRoutineIsProcedure
void Data (dpy, data, len)
	Display *dpy;
	char *data;
	long len;
{
	if (dpy->bufptr + (len) <= dpy->bufmax) {
		memcpy(dpy->bufptr, data, (int)len);
		dpy->bufptr += ((len) + 3) & ~3;
	} else {
		_XSend(dpy, data, len);
	}
}
#endif /* DataRoutineIsProcedure */


#ifdef LONG64
_XData32 (dpy, data, len)
    Display *dpy;
    register long *data;
    unsigned len;
{
    register int *buf;
    register long i;

    while (len) {
	buf = (int *)dpy->bufptr;
	i = dpy->bufmax - (char *)buf;
	if (!i) {
	    _XFlush(dpy);
	    continue;
	}
	if (len < i)
	    i = len;
	dpy->bufptr = (char *)buf + i;
	len -= i;
	i >>= 2;
	while (--i >= 0)
	    *buf++ = *data++;
    }
}
#endif /* LONG64 */

#ifdef WORD64

/*
 * XXX This is a *really* stupid way of doing this.  It should just use 
 * dpy->bufptr directly, taking into account where in the word it is.
 */

/*
 * Data16 - Place 16 bit data in the buffer.
 *
 * "dpy" is a pointer to a Display.
 * "data" is a pointer to the data.
 * "len" is the length in bytes of the data.
 */

static doData16(dpy, data, len, packbuffer)
    register Display *dpy;
    short *data;
    unsigned len;
    char *packbuffer;
{
    long *lp,*lpack;
    long i, nwords,bits;
    long mask16 = 0x000000000000ffff;

        lp = (long *)data;
        lpack = (long *)packbuffer;

/*  nwords is the number of 16 bit values to be packed,
 *  the low order 16 bits of each word will be packed
 *  into 64 bit words
 */
        nwords = len >> 1;
        bits = 48;

        for(i=0;i<nwords;i++){
	   if (bits == 48) *lpack = 0;
           *lpack ^= (*lp & mask16) << bits;
           bits -= 16 ;
           lp++;
           if(bits < 0){
               lpack++;
               bits = 48;
           }
        }
        Data(dpy, packbuffer, len);
}

_XData16 (dpy, data, len)
    Display *dpy;
    short *data;
    unsigned len;
{
    char packbuffer[PACKBUFFERSIZE];
    unsigned nunits = PACKBUFFERSIZE >> 1;

    for (; len > PACKBUFFERSIZE; len -= PACKBUFFERSIZE, data += nunits) {
	doData16 (dpy, data, PACKBUFFERSIZE, packbuffer);
    }
    if (len) doData16 (dpy, data, len, packbuffer);
}

/*
 * Data32 - Place 32 bit data in the buffer.
 *
 * "dpy" is a pointer to a Display.
 * "data" is a pointer to the data.
 * "len" is the length in bytes of the data.
 */

static doData32 (dpy, data, len, packbuffer)
    register Display *dpy;
    long *data;
    unsigned len;
    char *packbuffer;
{
    long *lp,*lpack;
    long i,bits,nwords;
    long mask32 = 0x00000000ffffffff;

        lpack = (long *) packbuffer;
        lp = data;

/*  nwords is the number of 32 bit values to be packed
 *  the low order 32 bits of each word will be packed
 *  into 64 bit words
 */
        nwords = len >> 2;
        bits = 32;

        for(i=0;i<nwords;i++){
	   if (bits == 32) *lpack = 0;
           *lpack ^= (*lp & mask32) << bits;
           bits = bits ^32;
           lp++;
           if(bits)
              lpack++;
        }
        Data(dpy, packbuffer, len);
}

_XData32 (dpy, data, len)
    Display *dpy;
    long *data;
    unsigned len;
{
    char packbuffer[PACKBUFFERSIZE];
    unsigned nunits = PACKBUFFERSIZE >> 2;

    for (; len > PACKBUFFERSIZE; len -= PACKBUFFERSIZE, data += nunits) {
	doData32 (dpy, data, PACKBUFFERSIZE, packbuffer);
    }
    if (len) doData32 (dpy, data, len, packbuffer);
}

#endif /* WORD64 */


/* Make sure this produces the same string as DefineLocal/DefineSelf in xdm.
 * Otherwise, Xau will not be able to find your cookies in the Xauthority file.
 *
 * Note: POSIX says that the ``nodename'' member of utsname does _not_ have
 *       to have sufficient information for interfacing to the network,
 *       and so, you may be better off using gethostname (if it exists).
 */

#if (defined(_POSIX_SOURCE) && !defined(AIXV3)) || defined(hpux) || defined(USG) || defined(SVR4)
#define NEED_UTSNAME
#include <sys/utsname.h>
#endif

/*
 * _XGetHostname - similar to gethostname but allows special processing.
 */
int _XGetHostname (buf, maxlen)
    char *buf;
    int maxlen;
{
    int len;

#ifdef NEED_UTSNAME
    struct utsname name;

    uname (&name);
    len = strlen (name.nodename);
    if (len >= maxlen) len = maxlen - 1;
    strncpy (buf, name.nodename, len);
    buf[len] = '\0';
#else
    buf[0] = '\0';
    (void) gethostname (buf, maxlen);
    buf [maxlen - 1] = '\0';
    len = strlen(buf);
#endif /* NEED_UTSNAME */
    return len;
}


/*
 * _XScreenOfWindow - get the Screen of a given window
 */

Screen *_XScreenOfWindow (dpy, w)
    Display *dpy;
    Window w;
{
    register int i;
    Window root;
    int x, y;				/* dummy variables */
    unsigned int width, height, bw, depth;  /* dummy variables */

    if (XGetGeometry (dpy, w, &root, &x, &y, &width, &height,
		      &bw, &depth) == False) {
	return None;
    }
    for (i = 0; i < ScreenCount (dpy); i++) {	/* find root from list */
	if (root == RootWindow (dpy, i)) {
	    return ScreenOfDisplay (dpy, i);
	}
    }
    return NULL;
}


#if (MSKCNT > 4)
/*
 * This is a macro if MSKCNT <= 4
 */
_XANYSET(src)
    long	*src;
{
    int i;

    for (i=0; i<MSKCNT; i++)
	if (src[ i ])
	    return (1);
    return (0);
}
#endif


#ifdef CRAY
/*
 * Cray UniCOS does not have readv and writev so we emulate
 */
#include <sys/socket.h>

int _XReadV (fd, iov, iovcnt)
int fd;
struct iovec *iov;
int iovcnt;
{
	struct msghdr hdr;

	hdr.msg_iov = iov;
	hdr.msg_iovlen = iovcnt;
	hdr.msg_accrights = 0;
	hdr.msg_accrightslen = 0;
	hdr.msg_name = 0;
	hdr.msg_namelen = 0;

	return (recvmsg (fd, &hdr, 0));
}

int _XWriteV (fd, iov, iovcnt)
int fd;
struct iovec *iov;
int iovcnt;
{
	struct msghdr hdr;

	hdr.msg_iov = iov;
	hdr.msg_iovlen = iovcnt;
	hdr.msg_accrights = 0;
	hdr.msg_accrightslen = 0;
	hdr.msg_name = 0;
	hdr.msg_namelen = 0;

	return (sendmsg (fd, &hdr, 0));
}

#endif /* CRAY */

#if (defined(SYSV) && defined(SYSV386) && !defined(STREAMSCONN)) || defined(WIN32)
/*
 * SYSV/386 and WIN32 do not have readv so we emulate
 */
#ifndef WIN32
#include <sys/uio.h>
#endif

int _XReadV (fd, iov, iovcnt)
int fd;
struct iovec *iov;
int iovcnt;
{
    int i, len, total;
    char *base;

    ESET(0);
    for (i=0, total=0;  i<iovcnt;  i++, iov++) {
	len = iov->iov_len;
	base = iov->iov_base;
	while (len > 0) {
	    register int nbytes;
	    nbytes = ReadFromServer(fd, base, len);
	    if (nbytes < 0 && total == 0)  return -1;
	    if (nbytes <= 0)  return total;
	    ESET(0);
	    len   -= nbytes;
	    total += nbytes;
	    base  += nbytes;
	}
    }
    return total;
}

#endif /* SYSV && SYSV386 && !STREAMSCONN || WIN32 */

#ifdef WIN32

int _XWriteV (fd, iov, iovcnt)
    int fd;
    struct iovec *iov;
    int iovcnt;
{
    int i, len, total;
    char *base;

    ESET(0);
    for (i=0, total=0;  i<iovcnt;  i++, iov++) {
	len = iov->iov_len;
	base = iov->iov_base;
	while (len > 0) {
	    register int nbytes;
	    nbytes = WriteToServer(fd, base, len);
	    if (nbytes < 0 && total == 0)  return -1;
	    if (nbytes <= 0)  return total;
	    ESET(0);
	    len   -= nbytes;
	    total += nbytes;
	    base  += nbytes;
	}
    }
    return total;
}
#endif /* WIN32 */

#ifdef STREAMSCONN
/*
 * Copyright 1988, 1989 AT&T, Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of AT&T not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  AT&T makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * AT&T DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL AT&T
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 iovec.c (C source file)
	Acc: 575557389 Mon Mar 28 08:03:09 1988
	Mod: 575557397 Mon Mar 28 08:03:17 1988
	Sta: 575557397 Mon Mar 28 08:03:17 1988
	Owner: 2011
	Group: 1985
	Permissions: 664
*/
/*
	START USER STAMP AREA
*/
/*
	END USER STAMP AREA
*/


extern char _XsTypeofStream[];
extern Xstream _XsStream[];

#define MAX_WORKAREA 4096
static char workarea[MAX_WORKAREA];



int
_XReadV (fd, v, n)
    int		fd;
    struct iovec *v;
    int		n;
{
	int i, rc, len, size = 0;
	char * buf = workarea;
	char * p;

	if (n <= 0 || n > 16)
	{
		ESET(EINVAL);
		return (-1);
	}
	for (i = 0; i < n; ++i)
	{
		if ((len = v[i].iov_len) < 0 || v[i].iov_base == NULL)
		{
			ESET(EINVAL);
			return (-1);
		}
		size += len;
	}
	if ((size > MAX_WORKAREA) && ((buf = malloc (size)) == NULL))
	{
		ESET(EINVAL);
		return (-1);
	}
	if((rc = (*_XsStream[_XsTypeOfStream[fd]].ReadFromStream)(fd, buf, size,
							     BUFFERING))> 0)
	{
		for (i = 0, p = buf; i < n; ++i)
		{
			memcpy (v[i].iov_base, p, len = v[i].iov_len);
			p += len;
		}
	}
	if (size > MAX_WORKAREA)
		free (buf);

	return (rc);
}

int
_XWriteV (fd, v, n)
    int fd;
    struct iovec *v;
    int n;
{
	int i, rc, len, size = 0;
	char * buf = workarea;
	char * p;

	if (n <= 0 || n > 16)
	{
		ESET(EINVAL);
		return (-1);
	}
	for (i = 0; i < n; ++i)
	{
		if ((len = v[i].iov_len) < 0 || v[i].iov_base == NULL)
		{
			ESET(EINVAL);
			return (-1);
		}
		size += len;
	}

	if ((size > MAX_WORKAREA) && ((buf = malloc (size)) == NULL))
	{
		ESET(EINVAL);
		return (-1);
	}
	for (i = 0, p = buf; i < n; ++i)
	{
		memcpy (p, v[i].iov_base, len = v[i].iov_len);
		p += len;
	}
	rc = (*_XsStream[_XsTypeOfStream[fd]].WriteToStream)(fd, buf, size);

	if (size > MAX_WORKAREA)
		free (buf);

	return (rc);
}



#endif /* STREAMSCONN */
