/* $XConsortium: XRecordTk.c,v 1.1 94/01/29 17:46:18 rws Exp $ */
/***************************************************************************
 * Copyright 1994 Network Computing Devices; 
 * Portions Copyright 1988 by Digital Equipment Corporation
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices or Digital 
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 **************************************************************************/ 
#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xlibint.h>
#include <X11/Xtrans.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <errno.h>

#include "recordstr.h" 
#include "record.h" 

#define MAXline		78

unsigned short 
IByte(buf)
    unsigned char buf[];
{
    return (buf[0]);
}

DumpItem(name, fd, buf, n)
char           *name;
int		fd; 
unsigned char  *buf;
long            n;
{
    static char     Leader[10 + 1];

    if (n == 0)
	return;

    fprintf(stderr, "%s%20s (fd %d): ", Leader, name, fd); 

    DumpIntBuffer(buf, n);
    fprintf(stdout, "\n");
}

DumpIntBuffer(buf, n)
unsigned char  *buf;
long            n;
{
    short           i;
    short           column;
    char            h[6] /* one hex or octal character */ ;
 
    column = 27 + 8;
    for (i = 0; i < n; i++)
    {
	/* get the int representations */
        (void) sprintf(h, "%d", (0xff & buf[i]));

	/* check if these characters will fit on this line */
	if ((column + strlen(h) + 1) > MAXline)
	{
	    /* line will be too long -- print it */
	    fprintf(stdout, "\n");
	    column = 0;
	}
	fprintf(stdout, "%s ", h);
	column += 3;
    }
}

static void
XNCDDispatch(reply)
    xRecordEnableConfigReply *reply;
{
     char *type;
     type = (reply->direction ? "FromServer" : "FromClient");

     fprintf(stderr, "%s  ", type); 
     if(reply->direction)
     {
        if(reply->data.u.event.u.u.type > X_Reply) 
    	    fprintf(stderr, "Event: %d:%d (%d, %d) Client: 0x%lx seq: 0x%lx\n", 
	    	reply->data.u.event.u.u.type, reply->data.u.event.u.u.detail,
	    	reply->data.u.event.u.keyButtonPointer.rootX,
	    	reply->data.u.event.u.keyButtonPointer.rootY,
	    	reply->id_base, reply->client_seq);
        else if(reply->data.u.event.u.u.type == X_Error)
     	    fprintf(stderr, "Error: %d:%d (%d, %d) Client: 0x%lx seq: 0x%lx\n", 
	    	reply->data.u.event.u.u.type, reply->data.u.event.u.u.detail,
	    	reply->data.u.event.u.keyButtonPointer.rootX,
	    	reply->data.u.event.u.keyButtonPointer.rootY,
	    	reply->id_base, reply->client_seq); 
		  
     } 
     else 
     {
     	fprintf(stderr, "Request: %d  Client: 0x%lx seq: 0x%lx\n", 
	    reply->data.u.req.reqType, reply->id_base, reply->client_seq);
     } 
}

static int
XNCDParseConn(dpy)
    Display	*dpy;
{
    int			xfd = ConnectionNumber(dpy); 
    BYTE		buf[BUFSIZE];
    int			nbytes = SIZEOF(xRecordEnableConfigReply);
    register int	len; 
    BytesReadable_t	numread; 
    register xRecordEnableConfigReply *reply;   

    if(_X11TransBytesReadable(dpy->trans_conn, &numread) < 0)
    {
        perror ("Error while reading input");
	fprintf(stderr, "IO error on  \"%s\"\n", 
  	DisplayString(dpy) ); 
        return -1;   
    } 
#ifdef VERBOSE
    DumpItem("XRECORD data", xfd, buf, numread);
#endif

    if(numread)
    {
 
        len = numread; 
        if(len < nbytes)
	    len = nbytes;
        if(len > BUFSIZE)
	    len = BUFSIZE;
        len = (len/nbytes) * nbytes; 

        _XRead(dpy, buf, (long) len); 

        /* 
        ** No events or errors should come across the data connection.
        ** The control connection is used for event loops, resource 
	** allocation, etc.
        */

        for(reply = (xRecordEnableConfigReply *)buf; len > 0;)
        {
            XNCDDispatch( (xRecordEnableConfigReply *)reply); 
	    reply++;
	    len -= nbytes;
#ifdef VERBOSE 
            fprintf(stderr, "len=%d\n", len); 
#endif
        }
        return; 
     }
     return;  
}

int 
XRecordEnableLoop(dpy, done)
    Display *dpy;
    Bool *done;
{
    int status = True;

    if(done)
    {
        while (1) 
        {      
            (void)XNCDParseConn(dpy);             
        } 
    }
    else
    {
        status = False;
    }
   return(status);
}
