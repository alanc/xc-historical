/* $XConsortium$ */

/* Copyright (c) 1993, 1994 NCR Corporation - Dayton, Ohio, USA
 * Copyright 1993, 1994 by the Massachusetts Institute of Technology
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name NCR or M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCR and M.I.T. make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCR DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NCR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#include <sys/ioctl.h>

#if defined(X11_t)
#define DNETOBJ X$X
#endif
#if defined(XIM_t)
#define DNETOBJ IMSERVER$
#endif
#if defined(FS_t) || defined(FONT_t)
#define DNETOBJ X$FONT
#endif
#if defined(ICE_t)
#define DNETOBJ X$ICE
#endif
#if defined(TEST_t)
#define DNETOBJ X$TEST
#endif


/*
 * This is the DNET implementation of the X Transport service layer
 */

static int
is_numeric (str)

char *str;

{
    int i;

    for (i = 0; i < (int) strlen (str); i++)
	if (!isdigit (str[i]))
	    return (0);

    return (1);
}


/*
 * This function gets the local address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */

static int
TRANS(DNETGetAddr) (ciptr)

XtransConnInfo ciptr;

{
    sockaddr_dn	sockname;
    int		namelen = sizeof(sockname);

    PRMSG (3,"TRANS(DNETGetAddr) (%x)\n", ciptr, 0, 0);

    if (getsockname (ciptr->fd, (struct sockaddr *) &sockname, &namelen) < 0)
    {
	PRMSG (1,"TRANS(DNETGetAddr): getsockname() failed: %d\n",
	      EGET(), 0, 0);
	return -1;
    }


    /*
     * Everything looks good: fill in the XtransConnInfo structure.
     */

    if ((ciptr->addr = (char *) malloc (namelen)) == NULL)
    {
        PRMSG (1, "TRANS(DNETGetAddr): Can't allocate space for the addr\n",
	       0, 0, 0);
        return -1;
    }

    ciptr->family = sockname.sdn_family;
    ciptr->addrlen = namelen;
    memcpy (ciptr->addr, &sockname, ciptr->addrlen);

    return 0;
}


/*
 * This function gets the remote address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */

static int
TRANS(DNETGetPeerAddr) (ciptr)

XtransConnInfo ciptr;

{
    sockaddr_dn	sockname;
    int		namelen = sizeof(sockname);

    PRMSG (3,"TRANS(DNETGetPeerAddr) (%x)\n", ciptr, 0, 0);

    if (getpeername (ciptr->fd, (struct sockaddr *) &sockname, &namelen) < 0)
    {
	PRMSG (1,"TRANS(DNETGetPeerAddr): getpeername() failed: %d\n",
	      EGET(), 0, 0);
	return -1;
    }

    /*
     * Everything looks good: fill in the XtransConnInfo structure.
     */

    if ((ciptr->peeraddr = (char *) malloc (namelen)) == NULL)
    {
        PRMSG (1,
	      "TRANS(DNETGetPeerAddr): Can't allocate space for the addr\n",
	      0, 0, 0);
        return -1;
    }

    ciptr->peeraddrlen = namelen;
    memcpy (ciptr->peeraddr, &sockname, ciptr->peeraddrlen);

    return 0;
}


static XtransConnInfo
TRANS(DNETOpenCOTSClient) (thistrans, protocol, host, port)

Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;

{
    XtransConnInfo	ciptr;

    PRMSG (2,"TRANS(DNETOpenCOTSClient) (%s,%s,%s)\n", protocol, host, port);

    if ((ciptr = (XtransConnInfo) calloc (
	1, sizeof(struct _XtransConnInfo))) == NULL)
    {
	PRMSG (1, "TRANS(DNETOpenCOTSClient): malloc failed\n", 0, 0, 0);
	return NULL;
    }

    /* nothing else to do here */

    return ciptr;
}


static XtransConnInfo
TRANS(DNETOpenCOTSServer) (thistrans, protocol, host, port)

Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;

{
    XtransConnInfo	ciptr;

    PRMSG (2,"TRANS(DNETOpenCOTSServer) (%s,%s,%s)\n", protocol, host, port);

    if ((ciptr = (XtransConnInfo) calloc (
	1, sizeof(struct _XtransConnInfo))) == NULL)
    {
	PRMSG (1, "TRANS(DNETOpenCOTSServer): malloc failed\n", 0, 0, 0);
	return NULL;
    }

    if ((ciptr->fd = socket (AF_DECnet, SOCK_STREAM, 0)) < 0)
    {
	free ((char *) ciptr);
	return NULL;
    }

    return (ciptr);
}


static XtransConnInfo
TRANS(DNETOpenCLTSClient) (thistrans, protocol, host, port)

Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;

{
    XtransConnInfo	ciptr;

    PRMSG (2,"TRANS(DNETOpenCLTSClient) (%s,%s,%s)\n", protocol, host, port);

    if ((ciptr = (XtransConnInfo) calloc (
	1, sizeof (struct _XtransConnInfo))) == NULL)
    {
	PRMSG (1, "TRANS(DNETOpenCLTSClient): malloc failed\n", 0, 0, 0);
	return NULL;
    }

    /* nothing else to do here */

    return ciptr;
}


static XtransConnInfo
TRANS(DNETOpenCLTSServer) (thistrans, protocol, host, port)

Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;

{
    /* NEED TO IMPLEMENT */

    PRMSG (2,"TRANS(DNETOpenCLTSServer) (%s,%s,%s)\n", protocol, host, port);
    return NULL;
}


static int
TRANS(DNETSetOption) (ciptr, option, arg)

XtransConnInfo	ciptr;
int		option;
int		arg;

{
    PRMSG (2,"TRANS(DNETSetOption) (%d,%d,%d)\n", ciptr->fd, option, arg);

    return -1;
}


static int
TRANS(DNETCreateListener) (ciptr, port)

XtransConnInfo	ciptr;
char		*port;

{
    struct sockaddr_dn  dnsock;
    int			fd = ciptr->fd;

    PRMSG (3, "TRANS(DNETCreateListener) (%x,%d)\n", ciptr, fd, 0);

    bzero ((char *) &dnsock, sizeof (dnsock));
    dnsock.sdn_family = AF_DECnet;

    if (port && *port )
	sprintf (dnsock.sdn_objname, "%s%s", DNETOBJ, port);
    else
#ifdef X11_t
	return -1;
#else
	sprintf (dnsock.sdn_objname, "%s%d", DNETOBJ, getpid ());
#endif

    dnsock.sdn_objnamel = strlen (dnsock.sdn_objname);

    if (bind (fd, (struct sockaddr *) &dnsock, sizeof (dnsock)))
    {
	close (fd);
	return -1;
    }

    if (listen (fd, 5))
    {
	close (fd);
	return (-1);
    }


    /* Set a flag to indicate that this connection is a listener */

    ciptr->flags = 1;

    return 0;
}


static XtransConnInfo
TRANS(DNETAccept) (ciptr)

XtransConnInfo	ciptr;

{
    XtransConnInfo	newciptr;
    sockaddr_dn		sockname;
    int			namelen = sizeof(sockname);

    PRMSG (2, "TRANS(DNETAccept) (%x,%d)\n", ciptr, ciptr->fd, 0);

    if ((newciptr = (XtransConnInfo) calloc(
	1, sizeof (struct _XtransConnInfo))) == NULL)
    {
	PRMSG (1, "TRANS(DNETAccept): malloc failed\n", 0, 0, 0);
	return NULL;
    }

    if((newciptr->fd = accept (ciptr->fd,
	(struct sockaddr *) &sockname, &namelen)) < 0)
    {
	PRMSG (1, "TRANS(DNETAccept): accept() failed\n", 0, 0, 0);

	free (newciptr);
	return NULL;
    }

    /*
     * Get this address again because the transport may give a more 
     * specific address now that a connection is established.
     */

    if (TRANS(DNETGetAddr) (newciptr) < 0)
    {
	PRMSG(1,
	"TRANS(DNETAccept): TRANS(DNETGetAddr)() failed:\n", 0, 0, 0);
	close (newciptr->fd);
	free (newciptr);
        return NULL;
    }

    if (TRANS(DNETGetPeerAddr) (newciptr) < 0)
    {
	PRMSG(1,
	"TRANS(DNETAccept): TRANS(DNETGetPeerAddr)() failed:\n", 0, 0, 0);

	close (newciptr->fd);
	if (newciptr->addr) free (newciptr->addr);
	free (newciptr);
        return NULL;
    }

    return newciptr;
}


#define OBJBUFSIZE 64

static int
TRANS(DNETConnect) (ciptr, host, port)

XtransConnInfo	ciptr;
char		*host;
char		*port;

{
    char objname[OBJBUFSIZE];

    extern int dnet_conn();
    
    PRMSG (2,"TRANS(DNETConnect) (%d,%s,%s)\n", ciptr->fd, host, port);

#ifdef X11_t
    /*
     * X has a well known port, that is transport dependent. It is easier
     * to handle it here, than try and come up with a transport independent
     * representation that can be passed in and resolved the usual way.
     *
     * The port that is passed here is really a string containing the idisplay
     * from ConnectDisplay().
     */

    if (is_numeric (port))
    {
	short tmpport = (short) atoi (port);

	sprintf (objname, "X$X%d", tmpport);
    }
    else
#endif
	strncpy(objname, port, OBJBUFSIZE);


    /*
     * Do the connect
     */

    if (!host) host = "0";

    if ((ciptr->fd = dnet_conn (host, objname, SOCK_STREAM, 0, 0, 0, 0)) < 0)
    {
	return TRANS_CONNECT_FAILED;
    }


    /*
     * Sync up the address fields of ciptr.
     */

    if (TRANS(DNETGetAddr) (ciptr) < 0)
    {
	PRMSG (1,
	      "TRANS(DNETConnect): TRANS(DNETGetAddr) () failed:\n", 0, 0, 0);
	return TRANS_CONNECT_FAILED;
    }

    if (TRANS(DNETGetPeerAddr) (ciptr) < 0)
    {
	PRMSG (1,
	      "TRANS(DNETConnect): TRANS(DNETGetPeerAddr) () failed:\n",
	      0, 0, 0);
	return TRANS_CONNECT_FAILED;
    }

    return 0;
}


static int
TRANS(DNETBytesReadable) (ciptr, pend)

XtransConnInfo	ciptr;
BytesReadable_t	*pend;

{
    PRMSG (2,"TRANS(DNETBytesReadable) (%x,%d,%x)\n", ciptr, ciptr->fd, pend);

    return ioctl(ciptr->fd, FIONREAD, (char *)pend);
}


static int
TRANS(DNETRead) (ciptr, buf, size)

XtransConnInfo	ciptr;
char		*buf;
int		size;

{
    PRMSG (2,"TRANS(DNETRead) (%d,%x,%d)\n", ciptr->fd, buf, size);

    return read (ciptr->fd, buf, size);
}


static int
TRANS(DNETWrite) (ciptr, buf, size)

XtransConnInfo	ciptr;
char		*buf;
int		size;

{
    PRMSG (2,"TRANS(DNETWrite) (%d,%x,%d)\n", ciptr->fd, buf, size);

    return write (ciptr->fd, buf, size);
}


static int
TRANS(DNETReadv) (ciptr, buf, size)

XtransConnInfo	ciptr;
struct iovec	*buf;
int		size;

{
    PRMSG (2,"TRANS(DNETReadv) (%d,%x,%d)\n", ciptr->fd, buf, size);

    return READV (ciptr, buf, size);
}


static int
TRANS(DNETWritev) (ciptr, buf, size)

XtransConnInfo	ciptr;
struct iovec	*buf;
int		size;

{
    PRMSG (2,"TRANS(DNETWritev) (%d,%x,%d)\n", ciptr->fd, buf, size);

    return WRITEV (ciptr, buf, size);
}


static int
TRANS(DNETDisconnect) (ciptr)

XtransConnInfo	ciptr;

{
    PRMSG (2,"TRANS(DNETDisconnect) (%x,%d)\n", ciptr, ciptr->fd, 0);

    return shutdown (ciptr->fd, 2); /* disallow further sends and receives */
}


static int
TRANS(DNETClose) (ciptr)

XtransConnInfo	ciptr;

{
    PRMSG (2,"TRANS(DNETClose) (%x,%d)\n", ciptr, ciptr->fd, 0);

    return close (ciptr->fd);
}


static int
TRANS(DNETNameToAddr) (ciptr)

XtransConnInfo	ciptr;

{
    return -1;
}


static int
TRANS(DNETAddrToName) (ciptr)

XtransConnInfo	ciptr;

{
    return -1;
}


Xtransport	TRANS(DNETFuncs) = {
    /* DNET Interface */
    "decnet",
    0,
    TRANS(DNETOpenCOTSClient),
    TRANS(DNETOpenCOTSServer),
    TRANS(DNETOpenCLTSClient),
    TRANS(DNETOpenCLTSServer),
    TRANS(DNETSetOption),
    TRANS(DNETCreateListener),
    NULL,		       			/* ResetListener */
    TRANS(DNETAccept),
    TRANS(DNETConnect),
    TRANS(DNETBytesReadable),
    TRANS(DNETRead),
    TRANS(DNETWrite),
    TRANS(DNETReadv),
    TRANS(DNETWritev),
    TRANS(DNETDisconnect),
    TRANS(DNETClose),
    TRANS(DNETNameToAddr),
    TRANS(DNETAddrToName),
};
