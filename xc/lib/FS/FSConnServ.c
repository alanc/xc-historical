/* $XConsortium: FSConnServ.c,v 1.23 93/09/22 21:52:24 rws Exp $ */

/*
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include	<stdio.h>
#include	"FSlibint.h"
#ifdef NCD
#include	<fcntl.h>
#endif
#ifdef WIN32
#define ECHECK(err) (WSAGetLastError() == err)
#else
#define ECHECK(err) (errno == err)
#endif

/*
 * Attempts to connect to server, given server name. Returns transport
 * connection object or NULL if connection fails.
 */

#define FS_CONNECTION_RETRIES 5

XtransConnInfo
_FSConnectServer(server_name)
    char       *server_name;
{
    XtransConnInfo trans_conn;		/* transport connection object */
    int retry, connect_stat;
    int  madeConnection = 0;

    /*
     * Open the network connection.
     */

    for (retry = FS_CONNECTION_RETRIES; retry >= 0; retry--)
    {
	if ((trans_conn = _FSTransOpenCOTSClient(server_name)) == NULL)
	{
	    break;
	}

	if ((connect_stat = _FSTransConnect(trans_conn,server_name)) < 0)
	{
	    _FSTransClose(trans_conn);

	    if (connect_stat == TRANS_TRY_CONNECT_AGAIN)
	    {
		sleep(1);
		continue;
	    }
	    else
		break;
	}
	else
	{
	    madeConnection = 1;
	    break;
	}
    }

    if (!madeConnection)
	return (NULL);


    /*
     * set it non-blocking.  This is so we can read data when blocked for
     * writing in the library.
     */

    _FSTransSetOption(trans_conn, TRANS_NONBLOCKING, 1);

    return (trans_conn);
}

/*
 * Disconnect from server.
 */

int
_FSDisconnectServer(trans_conn)
    XtransConnInfo	trans_conn;

{
    (void) _FSTransClose(trans_conn);
}

#undef NULL
#define NULL ((char *) 0)
/*
 * This is an OS dependent routine which:
 * 1) returns as soon as the connection can be written on....
 * 2) if the connection can be read, must enqueue events and handle errors,
 * until the connection is writable.
 */
_FSWaitForWritable(svr)
    FSServer     *svr;
{
    FdSet	r_mask;
    FdSet	w_mask;
    int         nfound;

    CLEARBITS(r_mask);
    CLEARBITS(w_mask);

    while (1) {
	BITSET(r_mask, svr->fd);
	BITSET(w_mask, svr->fd);

	do {
#ifdef WIN32
	    nfound = select (0, &r_mask, &w_mask, NULL, NULL);
#else
	    nfound = select(svr->fd + 1, r_mask, w_mask, NULL, NULL);
#endif
	    if (nfound < 0 && !ECHECK(EINTR))
		(*_FSIOErrorFunction) (svr);
	} while (nfound <= 0);

	if (_FSANYSET(r_mask)) {
	    char        buf[BUFSIZE];
	    BytesReadable_t pend_not_register;
	    register BytesReadable_t pend;
	    register fsEvent *ev;

	    /* find out how much data can be read */
	    if (_FSTransBytesReadable(svr->trans_conn, &pend_not_register) < 0)
		(*_FSIOErrorFunction) (svr);
	    pend = pend_not_register;

	    /*
	     * must read at least one fsEvent; if none is pending, then we'll
	     * just block waiting for it
	     */
	    if (pend < SIZEOF(fsEvent))
		pend = SIZEOF(fsEvent);

	    /* but we won't read more than the max buffer size */
	    if (pend > BUFSIZE)
		pend = BUFSIZE;

	    /* round down to an integral number of FSReps */
	    pend = (pend / SIZEOF(fsEvent)) * SIZEOF(fsEvent);

	    _FSRead(svr, buf, pend);

	    /* no space between comma and type or else macro will die */
	    STARTITERATE(ev, fsEvent, buf, (pend > 0),
			 (pend -= SIZEOF(fsEvent))) {
		if (ev->type == FS_Error)
		    _FSError(svr, (fsError *) ev);
		else		/* it's an event packet; enqueue it */
		    _FSEnq(svr, ev);
	    }
	    ENDITERATE
	}
	if (_FSANYSET(w_mask))
	    return;
    }
}


_FSWaitForReadable(svr)
    FSServer     *svr;
{
    FdSet	r_mask;
    int         result;

    CLEARBITS(r_mask);
    do {
	BITSET(r_mask, svr->fd);
#ifdef WIN32
	result = select (0, &r_mask, NULL, NULL, NULL);
#else
	result = select(svr->fd + 1, r_mask, NULL, NULL, NULL);
#endif
	if (result == -1 && !ECHECK(EINTR))
	    (*_FSIOErrorFunction) (svr);
    } while (result <= 0);
}

_FSSendClientPrefix(svr, client)
    FSServer     *svr;
    fsConnClientPrefix *client;
{
    struct iovec iovarray[5],
               *iov = iovarray;
    int         niov = 0;

#define add_to_iov(b,l) \
	  { iov->iov_base = (b); iov->iov_len = (l); iov++, niov++; }

    add_to_iov((caddr_t) client, SIZEOF(fsConnClientPrefix));

#undef add_to_iov

    (void) _FSTransWritev(svr->trans_conn, iovarray, niov);
    return;
}
