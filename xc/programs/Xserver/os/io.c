/***********************************************************
Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/* $XConsortium: io.c,v 1.50 89/01/03 08:31:00 rws Exp $ */
/*****************************************************************
 * i/o functions
 *
 *   WriteToClient, ReadRequestFromClient
 *   InsertFakeRequest, ResetCurrentRequest
 *
 *****************************************************************/

#include <stdio.h>
#include "Xos.h"
#include "Xmd.h"
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "X.h"
#include "Xproto.h"
#include "os.h"
#include "osdep.h"
#include "opaque.h"
#include "dixstruct.h"
#include "misc.h"

extern long ClientsWithInput[];
extern long ClientsWriteBlocked[];
extern long OutputPending[];
extern long OutputBufferSize;
extern int ConnectionTranslation[];
extern Bool NewOutputPending;
extern Bool AnyClientsWriteBlocked;
static Bool CriticalOutputPending;
static int timesThisConnection = 0;

extern int errno;

#define request_length(req, cli) ((cli->swapped ? \
	lswaps((req)->length) : (req)->length) << 2)
#define MAX_TIMES_PER         10

/*****************************************************************
 * ReadRequestFromClient
 *    Returns one request from client.  If the client misbehaves,
 *    returns NULL.  The dispatcher closes down all misbehaving clients.  
 *
 *        client:  index into bit array returned from WaitForSomething() 
 *
 *        status: status is set to
 *            > 0 the number of bytes in the request if the read is sucessful 
 *            = 0 if action would block (entire request not ready)
 *            < 0 indicates an error (probably client died)
 *
 *        oldbuf:
 *            To facilitate buffer management (e.g. on multi-processor
 *            systems), the diX layer must tell the OS layer when it is 
 *            done with a request, so the parameter oldbuf is a pointer 
 *            to a request that diX is finished with.  In the 
 *            sample implementation, which is single threaded,
 *            oldbuf is ignored.  We assume that when diX calls
 *            ReadRequestFromClient(), the previous buffer is finished with.
 *
 *    The returned string returned must be contiguous so that it can be
 *    cast in the dispatcher to the correct request type.  Because requests
 *    are variable length, ReadRequestFromClient() must look at the first 4
 *    bytes of a request to determine the length (the request length is
 *    always the 3rd byte in the request).  
 *
 *    Note: in order to make the server scheduler (WaitForSomething())
 *    "fair", the ClientsWithInput mask is used.  This mask tells which
 *    clients have FULL requests left in their buffers.  Clients with
 *    partial requests require a read.  Basically, client buffers
 *    are drained before select() is called again.  But, we can't keep
 *    reading from a client that is sending buckets of data (or has
 *    a partial request) because others clients need to be scheduled.
 *****************************************************************/

ConnectionInput inputBuffers[MAXSOCKS];    /* buffers for clients */

#define YieldControl()				\
        { isItTimeToYield = TRUE;		\
	  timesThisConnection = 0; }
#define YieldControlNoInput()			\
        { YieldControl();			\
	  BITCLEAR(ClientsWithInput, fd); }
#define YieldControlAndReturnNull()		\
        { YieldControlNoInput();		\
	  return((char *) NULL ); }

/*ARGSUSED*/
char *
ReadRequestFromClient(client, status, oldbuf)
    ClientPtr client;
    int *status;          /* read at least n from client */
    char *oldbuf;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int fd = oc->fd;
    int result, gotnow, needed;
    register ConnectionInput *pBuff;
    register xReq *request;

    /* ignore oldbuf, just assume we're done with prev. buffer */

    pBuff = &inputBuffers[fd];
    pBuff->bufptr += pBuff->lenLastReq;

    request = (xReq *)pBuff->bufptr;
    gotnow = pBuff->bufcnt + pBuff->buffer - pBuff->bufptr;
    if ((gotnow < sizeof(xReq)) ||
	(gotnow < (needed = request_length(request, client))))
    {
	pBuff->lenLastReq = 0;
	if ((gotnow < sizeof(xReq)) || (needed == 0))
	   needed = sizeof(xReq);
	else if (needed > MAXBUFSIZE)
	{
	    *status = -1;
	    YieldControlAndReturnNull();
	}
	if ((gotnow == 0) ||
	    ((pBuff->bufptr - pBuff->buffer + needed) > pBuff->size))
	{
	    if ((gotnow > 0) && (pBuff->bufptr != pBuff->buffer))
		bcopy(pBuff->bufptr, pBuff->buffer, gotnow);
	    if (needed > pBuff->size)
	    {
		pBuff->size = needed;
		pBuff->buffer = (char *)xrealloc(pBuff->buffer, pBuff->size);
	    }
	    pBuff->bufptr = pBuff->buffer;
	    pBuff->bufcnt = gotnow;
	}
	result = read(fd, pBuff->buffer + pBuff->bufcnt, 
		      pBuff->size - pBuff->bufcnt); 
	if (result <= 0)
	{
	    if ((result < 0) && (errno == EWOULDBLOCK))
		*status = 0;
	    else
		*status = -1;
	    YieldControlAndReturnNull();
	}
	pBuff->bufcnt += result;
	gotnow += result;
	/* free up some space after huge requests */
	if ((pBuff->size > BUFWATERMARK) &&
	    (pBuff->bufcnt < BUFSIZE) && (needed < BUFSIZE))
	{
	    pBuff->size = BUFSIZE;
	    pBuff->buffer = (char *)xrealloc(pBuff->buffer, pBuff->size);
	    pBuff->bufptr = pBuff->buffer + pBuff->bufcnt - gotnow;
	}
	request = (xReq *)pBuff->bufptr;
	if ((gotnow < sizeof(xReq)) ||
	    (gotnow < (needed = request_length(request, client))))
	{
	    *status = 0;
	    YieldControlAndReturnNull();
	}
    }
    *status = needed;
    pBuff->lenLastReq = needed;

    /*
     *  Check to see if client has at least one whole request in the
     *  buffer.  If there is only a partial request, treat like buffer
     *  is empty so that select() will be called again and other clients
     *  can get into the queue.   
     */

    if (gotnow >= needed + sizeof(xReq)) 
    {
	request = (xReq *)(pBuff->bufptr + needed);
        if (gotnow >= needed + request_length(request, client))
	    BITSET(ClientsWithInput, fd);
        else
	    YieldControlNoInput();
    }
    else
	YieldControlNoInput();
    if (++timesThisConnection >= MAX_TIMES_PER)
	YieldControl();

    return(pBuff->bufptr);
}

/*****************************************************************
 * InsertFakeRequest
 *    Splice a consed up (possibly partial) request in as the next request.
 *
 **********************/

InsertFakeRequest(client, data, count)
    ClientPtr client;
    char *data;
    int count;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int fd = oc->fd;
    register ConnectionInput *pBuff;
    register xReq *request;
    int gotnow, moveup;

    pBuff = &inputBuffers[fd];
    pBuff->lenLastReq = 0;
    gotnow = pBuff->bufcnt + pBuff->buffer - pBuff->bufptr;
    if ((gotnow + count) > pBuff->size)
    {
	pBuff->size = gotnow + count;
	pBuff->buffer = (char *)xrealloc(pBuff->buffer, pBuff->size);
	pBuff->bufptr = pBuff->buffer + pBuff->bufcnt - gotnow;
    }
    moveup = count - (pBuff->bufptr - pBuff->buffer);
    if (moveup > 0)
    {
	if (gotnow > 0)
	    bcopy(pBuff->bufptr, pBuff->bufptr + moveup, gotnow);
	pBuff->bufptr += moveup;
	pBuff->bufcnt += moveup;
    }
    bcopy(data, pBuff->bufptr - count, count);
    pBuff->bufptr -= count;
    request = (xReq *)pBuff->bufptr;
    gotnow += count;
    if ((gotnow >= sizeof(xReq)) &&
	(gotnow >= request_length(request, client)))
	BITSET(ClientsWithInput, fd);
    else
	YieldControlNoInput();
}

/*****************************************************************
 * ResetRequestFromClient
 *    Reset to reexecute the current request, and yield.
 *
 **********************/

ResetCurrentRequest(client)
    ClientPtr client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int fd = oc->fd;
    register ConnectionInput *pBuff;
    register xReq *request;
    int gotnow;

    pBuff = &inputBuffers[fd];
    pBuff->lenLastReq = 0;
    request = (xReq *)pBuff->bufptr;
    gotnow = pBuff->bufcnt + pBuff->buffer - pBuff->bufptr;
    if ((gotnow >= sizeof(xReq)) &&
	(gotnow >= request_length(request, client)))
    {
	BITSET(ClientsWithInput, fd);
	YieldControl();
    }
    else
	YieldControlNoInput();
}

    /* lookup table for adding padding bytes to data that is read from
    	or written to the X socket.  */
static int padlength[4] = {0, 3, 2, 1};

 /********************
 * FlushClient()
 *    If the client isn't keeping up with us, then we try to continue
 *    buffering the data and set the apropriate bit in ClientsWritable
 *    (which is used by WaitFor in the select).  If the connection yields
 *    a permanent error, or we can't allocate any more space, we then
 *    close the connection.
 *
 **********************/

int
FlushClient(who, oc, extraBuf, extraCount)
    ClientPtr who;
    OsCommPtr oc;
    char *extraBuf;
    int extraCount; /* do not modify... returned below */
{
    int connection = oc->fd,
    	total, n, i, notWritten, written,
	iovCnt = 0;
    struct iovec iov[3];
    char padBuffer[3];

    total = 0;
    if (oc->count)
    {
	total += iov[iovCnt].iov_len = oc->count;
	iov[iovCnt++].iov_base = (caddr_t)oc->buf;
        /* Notice that padding isn't needed for oc->buf since
           it is alreay padded by WriteToClient */
    }
    if (extraCount)
    {
	total += iov[iovCnt].iov_len = extraCount;
	iov[iovCnt++].iov_base = extraBuf;
	if (extraCount & 3)
	{
	    total += iov[iovCnt].iov_len = padlength[extraCount & 3];
	    iov[iovCnt++].iov_base = padBuffer;
	}
    }

    notWritten = total;
    while ((n = writev (connection, iov, iovCnt)) != notWritten)
    {
#ifdef hpux
	if (n == -1 && errno == EMSGSIZE)
	    n = swWritev (connection, iov, 2);
#endif
        if (n > 0) 
        {
	    notWritten -= n;
	    for (i = 0; i < iovCnt; i++)
            {
		if (n > iov[i].iov_len)
		{
		    n -= iov[i].iov_len;
		    iov[i].iov_len = 0;
		}
		else
		{
		    iov[i].iov_len -= n;
		    iov[i].iov_base += n;
		    break;
		}
	    }
	    continue;
	}
	else if (errno != EWOULDBLOCK)
        {
#ifdef notdef
	    if (errno != EBADF)
		ErrorF("Closing connection %d because write failed\n",
			connection);
		/* this close will cause the select in WaitForSomething
		   to return that the connection is dead, so we can actually
		   clean up after the client.  We can't clean up here,
		   because the we're in the middle of doing something
		   and will probably screw up some data strucutres */
#endif
	    close(connection);
            MarkClientException(who);
	    return(-1);
	}

	/* If we've arrived here, then the client is stuffed to the gills
	   and not ready to accept more.  Make a note of it and buffer
	   the rest. */
	BITSET(ClientsWriteBlocked, connection);
	AnyClientsWriteBlocked = TRUE;

	written = total - notWritten;
	if (written < oc->count)
	{
	    if (written > 0)
	    {
		oc->count -= written;
		bcopy((char *)oc->buf + written, (char *)oc->buf, oc->count);
		written = 0;
	    }
	}
	else
	{
	    written -= oc->count;
	    oc->count = 0;
	}

	if (notWritten > oc->bufsize)
	{
	    /* allocate at least enough to contain it plus one
	       OutputBufferSize */
	    oc->bufsize = notWritten + OutputBufferSize;
	    oc->buf = (unsigned char *)xrealloc(oc->buf, oc->bufsize);
	    if (oc->buf == NULL)
	    {
	outOfMem:
#ifdef notdef
		ErrorF("Closing connection %d because out of memory\n",
			connection);
		/* this close will cause the select in WaitForSomething
		   to return that the connection is dead, so we can actually
		   clean up after the client.  We can't clean up here,
		   because the we're in the middle of doing something
		   and will probably screw up some data strucutres */
#endif
		close(connection);
		MarkClientException(who);
		oc->count = 0;
		oc->bufsize = 0;
		return(-1);
	    }
	}

	/* If the amount written extended into the padBuffer, then the
	   difference "extraCount - written" may be less than 0 */
	if ((n = extraCount - written) > 0)
	    bcopy (extraBuf + written, (char *)oc->buf + oc->count, n);

	oc->count = notWritten; /* this will include the pad */

	return extraCount; /* return only the amount explicitly requested */
    }

    /* everything was flushed out */
    oc->count = 0;
    if (oc->bufsize > OutputBufferSize)
    {
	oc->bufsize = OutputBufferSize;
	oc->buf = (unsigned char *)xrealloc(oc->buf, OutputBufferSize);
	if (oc->buf == NULL) /* nearly impossible */
	    goto outOfMem;
    }
    return extraCount; /* return only the amount explicitly requested */
}

 /********************
 * FlushAllOutput()
 *    Flush all clients with output.  However, if some client still
 *    has input in the queue (more requests), then don't flush.  This
 *    will prevent the output queue from being flushed every time around
 *    the round robin queue.  Now, some say that it SHOULD be flushed
 *    every time around, but...
 *
 **********************/

void
FlushAllOutput()
{
    register int index, base, mask;
    OsCommPtr oc;
    register ClientPtr client;

    if (! NewOutputPending)
	return;

    /*
     * It may be that some client still has critical output pending,
     * but he is not yet ready to receive it anyway, so we will
     * simply wait for the select to tell us when he's ready to receive.
     */
    CriticalOutputPending = FALSE;
    NewOutputPending = FALSE;

    for (base = 0; base < mskcnt; base++)
    {
	mask = OutputPending[ base ];
	OutputPending[ base ] = 0;
	while (mask)
	{
	    index = ffs(mask) - 1;
	    mask &= ~lowbit(mask);
	    if ((index = ConnectionTranslation[(base << 5) + index]) == 0)
		continue;
	    client = clients[index];
	    if (client->clientGone)
		continue;
	    oc = (OsCommPtr)client->osPrivate;
	    if (GETBIT(ClientsWithInput, oc->fd))
	    {
		BITSET(OutputPending, oc->fd); /* set the bit again */
		NewOutputPending = TRUE;
	    }
	    else
		(void)FlushClient(client, oc, (char *)NULL, 0);
	}
    }

}

void
FlushIfCriticalOutputPending()
{
    if (CriticalOutputPending)
	FlushAllOutput();
}

void
SetCriticalOutputPending()
{
    CriticalOutputPending = TRUE;
}

/*****************
 * WriteToClient
 *    Copies buf into ClientPtr.buf if it fits (with padding), else
 *    flushes ClientPtr.buf and buf to client.  As of this writing,
 *    every use of WriteToClient is cast to void, and the result
 *    is ignored.  Potentially, this could be used by requests
 *    that are sending several chunks of data and want to break
 *    out of a loop on error.  Thus, we will leave the type of
 *    this routine as int.
 *****************/

int
WriteToClient (who, count, buf)
    ClientPtr who;
    char *buf;
    int count;
{
    OsCommPtr oc = (OsCommPtr)who->osPrivate;
    int padBytes;

    padBytes =  padlength[count & 3];

    if (oc->count + count + padBytes > oc->bufsize)
    {
	BITCLEAR(OutputPending, oc->fd);
	CriticalOutputPending = FALSE;
	NewOutputPending = FALSE;
	return FlushClient(who, oc, buf, count);
    }

    NewOutputPending = TRUE;
    BITSET(OutputPending, oc->fd);
    bcopy(buf, (char *)oc->buf + oc->count, count);
    oc->count += count + padBytes;
    
    return(count);
}

