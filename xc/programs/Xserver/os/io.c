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
/* $XConsortium: io.c,v 1.54 89/03/14 15:46:14 rws Exp $ */
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

extern void MarkClientException();
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
    register OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int fd = oc->fd;
    int result, gotnow, needed;
    register xReq *request;

    /* ignore oldbuf, just assume we're done with prev. buffer */

    oc->input.bufptr += oc->input.lenLastReq;

    request = (xReq *)oc->input.bufptr;
    gotnow = oc->input.bufcnt + oc->input.buffer - oc->input.bufptr;
    if ((gotnow < sizeof(xReq)) ||
	(gotnow < (needed = request_length(request, client))))
    {
	oc->input.lenLastReq = 0;
	if ((gotnow < sizeof(xReq)) || (needed == 0))
	   needed = sizeof(xReq);
	else if (needed > MAXBUFSIZE)
	{
	    *status = -1;
	    YieldControlAndReturnNull();
	}
	if ((gotnow == 0) ||
	    ((oc->input.bufptr - oc->input.buffer + needed) > oc->input.size))
	{
	    if ((gotnow > 0) && (oc->input.bufptr != oc->input.buffer))
		bcopy(oc->input.bufptr, oc->input.buffer, gotnow);
	    if (needed > oc->input.size)
	    {
		char *ibuf;

		ibuf = (char *)xrealloc(oc->input.buffer, needed);
		if (!ibuf)
		{
		    *status = -1;
		    YieldControlAndReturnNull();
		}
		oc->input.size = needed;
		oc->input.buffer = ibuf;
	    }
	    oc->input.bufptr = oc->input.buffer;
	    oc->input.bufcnt = gotnow;
	}
	result = read(fd, oc->input.buffer + oc->input.bufcnt, 
		      oc->input.size - oc->input.bufcnt); 
	if (result <= 0)
	{
	    if ((result < 0) && (errno == EWOULDBLOCK))
		*status = 0;
	    else
		*status = -1;
	    YieldControlAndReturnNull();
	}
	oc->input.bufcnt += result;
	gotnow += result;
	/* free up some space after huge requests */
	if ((oc->input.size > BUFWATERMARK) &&
	    (oc->input.bufcnt < BUFSIZE) && (needed < BUFSIZE))
	{
	    char *ibuf;

	    ibuf = (char *)xrealloc(oc->input.buffer, BUFSIZE);
	    if (ibuf)
	    {
		oc->input.size = BUFSIZE;
		oc->input.buffer = ibuf;
		oc->input.bufptr = ibuf + oc->input.bufcnt - gotnow;
	    }
	}
	request = (xReq *)oc->input.bufptr;
	if ((gotnow < sizeof(xReq)) ||
	    (gotnow < (needed = request_length(request, client))))
	{
	    *status = 0;
	    YieldControlAndReturnNull();
	}
    }
    *status = needed;
    oc->input.lenLastReq = needed;

    /*
     *  Check to see if client has at least one whole request in the
     *  buffer.  If there is only a partial request, treat like buffer
     *  is empty so that select() will be called again and other clients
     *  can get into the queue.   
     */

    if (gotnow >= needed + sizeof(xReq)) 
    {
	request = (xReq *)(oc->input.bufptr + needed);
        if (gotnow >= needed + request_length(request, client))
	    BITSET(ClientsWithInput, fd);
        else
	    YieldControlNoInput();
    }
    else
	YieldControlNoInput();
    if (++timesThisConnection >= MAX_TIMES_PER)
	YieldControl();

    return(oc->input.bufptr);
}

/*****************************************************************
 * InsertFakeRequest
 *    Splice a consed up (possibly partial) request in as the next request.
 *
 **********************/

Bool
InsertFakeRequest(client, data, count)
    ClientPtr client;
    char *data;
    int count;
{
    register OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int fd = oc->fd;
    register xReq *request;
    int gotnow, moveup;

    oc->input.lenLastReq = 0;
    gotnow = oc->input.bufcnt + oc->input.buffer - oc->input.bufptr;
    if ((gotnow + count) > oc->input.size)
    {
	char *ibuf;

	ibuf = (char *)xrealloc(oc->input.buffer, gotnow + count);
	if (!ibuf)
	    return(FALSE);
	oc->input.size = gotnow + count;
	oc->input.buffer = ibuf;
	oc->input.bufptr = ibuf + oc->input.bufcnt - gotnow;
    }
    moveup = count - (oc->input.bufptr - oc->input.buffer);
    if (moveup > 0)
    {
	if (gotnow > 0)
	    bcopy(oc->input.bufptr, oc->input.bufptr + moveup, gotnow);
	oc->input.bufptr += moveup;
	oc->input.bufcnt += moveup;
    }
    bcopy(data, oc->input.bufptr - count, count);
    oc->input.bufptr -= count;
    request = (xReq *)oc->input.bufptr;
    gotnow += count;
    if ((gotnow >= sizeof(xReq)) &&
	(gotnow >= request_length(request, client)))
	BITSET(ClientsWithInput, fd);
    else
	YieldControlNoInput();
    return(TRUE);
}

/*****************************************************************
 * ResetRequestFromClient
 *    Reset to reexecute the current request, and yield.
 *
 **********************/

ResetCurrentRequest(client)
    ClientPtr client;
{
    register OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int fd = oc->fd;
    register xReq *request;
    int gotnow;

    oc->input.lenLastReq = 0;
    request = (xReq *)oc->input.bufptr;
    gotnow = oc->input.bufcnt + oc->input.buffer - oc->input.bufptr;
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
    if (oc->output.count)
    {
	total += iov[iovCnt].iov_len = oc->output.count;
	iov[iovCnt++].iov_base = (caddr_t)oc->output.buf;
        /* Notice that padding isn't needed for oc->output.buf since
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
	    close(connection);
            MarkClientException(who);
	    oc->output.count = 0;
	    return(-1);
	}

	/* If we've arrived here, then the client is stuffed to the gills
	   and not ready to accept more.  Make a note of it and buffer
	   the rest. */
	BITSET(ClientsWriteBlocked, connection);
	AnyClientsWriteBlocked = TRUE;

	written = total - notWritten;
	if (written < oc->output.count)
	{
	    if (written > 0)
	    {
		oc->output.count -= written;
		bcopy((char *)oc->output.buf + written,
		      (char *)oc->output.buf,
		      oc->output.count);
		written = 0;
	    }
	}
	else
	{
	    written -= oc->output.count;
	    oc->output.count = 0;
	}

	if (notWritten > oc->output.size)
	{
	    unsigned char *obuf;

	    obuf = (unsigned char *)xrealloc(oc->output.buf,
					     notWritten + OutputBufferSize);
	    if (!obuf)
	    {
		close(connection);
		MarkClientException(who);
		oc->output.count = 0;
		return(-1);
	    }
	    oc->output.size = notWritten + OutputBufferSize;
	    oc->output.buf = obuf;
	}

	/* If the amount written extended into the padBuffer, then the
	   difference "extraCount - written" may be less than 0 */
	if ((n = extraCount - written) > 0)
	    bcopy (extraBuf + written,
		   (char *)oc->output.buf + oc->output.count,
		   n);

	oc->output.count = notWritten; /* this will include the pad */

	return extraCount; /* return only the amount explicitly requested */
    }

    /* everything was flushed out */
    oc->output.count = 0;
    if (oc->output.size > OutputBufferSize)
    {
	unsigned char *obuf;

	obuf = (unsigned char *)xrealloc(oc->output.buf, OutputBufferSize);
	if (obuf)
	{
	    oc->output.size = OutputBufferSize;
	    oc->output.buf = obuf;
	}
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

    if (oc->output.count + count + padBytes > oc->output.size)
    {
	BITCLEAR(OutputPending, oc->fd);
	CriticalOutputPending = FALSE;
	NewOutputPending = FALSE;
	return FlushClient(who, oc, buf, count);
    }

    NewOutputPending = TRUE;
    BITSET(OutputPending, oc->fd);
    bcopy(buf, (char *)oc->output.buf + oc->output.count, count);
    oc->output.count += count + padBytes;
    
    return(count);
}

