/*
 * $XConsortium: Flush.c,v 1.10 94/04/17 20:16:35 rws Exp $
 *
 * 
Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 * *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifdef WIN32
#define _WILLWINSOCK_
#endif
#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

#ifdef STREAMSCONN
#include <tiuser.h>
#else
#ifdef WIN32
#define BOOL wBOOL
#include <winsock.h>
#undef BOOL
#else
#include <sys/socket.h>
#endif
#endif

int
XdmcpFlush (fd, buffer, to, tolen)
    int		    fd;
    XdmcpBufferPtr  buffer;
    XdmcpNetaddr    to;
    int		    tolen;
{
    int result;

#ifdef STREAMSCONN
    struct t_unitdata dataunit;

    dataunit.addr.buf = to;
    dataunit.addr.len = tolen;
    dataunit.opt.len = 0;	/* default options */
    dataunit.udata.buf = (char *)buffer->data;
    dataunit.udata.len = buffer->pointer;
    result = t_sndudata(fd, &dataunit);
    if (result < 0)
	return FALSE;
#else
    result = sendto (fd, (char *)buffer->data, buffer->pointer, 0,
		     (struct sockaddr *)to, tolen);
    if (result != buffer->pointer)
	return FALSE;
#endif
    return TRUE;
}
