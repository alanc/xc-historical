/* $XConsortium: connection.c,v 1.193 95/04/06 16:10:29 mor Exp dpw $ */
/***********************************************************

Copyright (c) 1987, 1989  X Consortium

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


Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
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
/*****************************************************************
 *  Stuff to create connections --- OS dependent
 *
 *      EstablishNewConnections, CreateWellKnownSockets, ResetWellKnownSockets,
 *      CloseDownConnection, CheckConnections, AddEnabledDevice,
 *	RemoveEnabledDevice, OnlyListToOneClient,
 *      ListenToAllClients,
 *
 *      (WaitForSomething is in its own file)
 *
 *      In this implementation, a client socket table is not kept.
 *      Instead, what would be the index into the table is just the
 *      file descriptor of the socket.  This won't work for if the
 *      socket ids aren't small nums (0 - 2^8)
 *
 *****************************************************************/

#include "X.h"
#include "Xproto.h"
#include <X11/Xtrans.h>
#include <errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif
#include <sys/socket.h>

#include <signal.h>

#ifdef hpux
#include <sys/utsname.h>
#include <sys/ioctl.h>
#endif

#ifdef AIXV3
#include <sys/ioctl.h>
#endif

#if defined(TCPCONN) || defined(STREAMSCONN)
# include <netinet/in.h>
# ifndef hpux
#  ifdef apollo
#   ifndef NO_TCP_H
#    include <netinet/tcp.h>
#   endif
#  else
#   include <netinet/tcp.h>
#  endif
# endif
#endif

#include <stdio.h>
#include <sys/uio.h>
#include "misc.h"		/* for typedef of pointer */
#include <X11/Xpoll.h>
#include "osdep.h"
#include "opaque.h"
#include "dixstruct.h"

#ifdef LBX
#ifndef X_NOT_POSIX
#include <unistd.h>
#else
extern int read(), writev();
#endif
#endif /* LBX */

#ifdef X_NOT_POSIX
#define Pid_t int
#else
#define Pid_t pid_t
#endif

#ifdef DNETCONN
#include <netdnet/dn.h>
#endif /* DNETCONN */

extern char *display;		/* The display number */
int lastfdesc;			/* maximum file descriptor */

fd_set WellKnownConnections;	/* Listener mask */
fd_set EnabledDevices;		/* mask for input devices that are on */
fd_set AllSockets;		/* select on this */
fd_set AllClients;		/* available clients */
fd_set LastSelectMask;		/* mask returned from last select call */
fd_set ClientsWithInput;	/* clients with FULL requests in buffer */
fd_set ClientsWriteBlocked;	/* clients who cannot receive output */
fd_set OutputPending;		/* clients with reply/event data ready to go */
int MaxClients = MAXSOCKS;
long NConnBitArrays = howmany(XFD_SETSIZE, NFDBITS);
Bool NewOutputPending;		/* not yet attempted to write some new output */
Bool AnyClientsWriteBlocked;	/* true if some client blocked on write */

Bool RunFromSmartParent;	/* send SIGUSR1 to parent process */
Bool PartialNetwork;		/* continue even if unable to bind all addrs */
static Pid_t ParentProcess;

static Bool debug_conns = FALSE;

fd_set IgnoredClientsWithInput;
static fd_set GrabImperviousClients;
static fd_set SavedAllClients;
static fd_set SavedAllSockets;
static fd_set SavedClientsWithInput;
int GrabInProgress = 0;

int ConnectionTranslation[MAXSOCKS];
#ifdef LBX
int ConnectionOutputTranslation[MAXSOCKS];
#endif

XtransConnInfo 	*ListenTransConns = NULL;
int	       	*ListenTransFds = NULL;
int		ListenTransCount;

extern int auditTrailLevel;

static void ErrorConnMax(
#if NeedFunctionPrototypes
XtransConnInfo /* trans_conn */
#endif
);

#ifndef LBX
static
#endif
void CloseDownFileDescriptor(
#if NeedFunctionPrototypes
#ifdef LBX
    ClientPtr	client
#else
    register OsCommPtr /*oc*/
#endif
#endif
);

static XtransConnInfo
lookup_trans_conn (fd)

int fd;

{
    if (ListenTransFds)
    {
	int i;
	for (i = 0; i < ListenTransCount; i++)
	    if (ListenTransFds[i] == fd)
		return ListenTransConns[i];
    }

    return (NULL);
}

#ifdef XDMCP
void XdmcpOpenDisplay(), XdmcpInit(), XdmcpReset(), XdmcpCloseDisplay();
#endif

#ifdef LBX
extern int  StandardReadRequestFromClient();
extern int  StandardWriteToClient ();
extern int  UncompressWriteToClient ();
extern unsigned long  StandardRequestLength ();
extern int  StandardFlushClient ();
#endif


/*****************
 * CreateWellKnownSockets
 *    At initialization, create the sockets to listen on for new clients.
 *****************/

void
CreateWellKnownSockets()
{
    int		request, i;
    int		partial;
    char 	port[20];

    FD_ZERO(&AllSockets);
    FD_ZERO(&AllClients);
    FD_ZERO(&LastSelectMask);
    FD_ZERO(&ClientsWithInput);

    for (i=0; i<MAXSOCKS; i++) ConnectionTranslation[i] = 0;
#ifdef LBX
    for (i=0; i<MAXSOCKS; i++) ConnectionOutputTranslation[i] = 0;
#endif
#ifdef XNO_SYSCONF      /* should only be on FreeBSD 1.x and NetBSD 0.x */
#undef _SC_OPEN_MAX
#endif
#ifdef _SC_OPEN_MAX
    lastfdesc = sysconf(_SC_OPEN_MAX) - 1;
#else
#ifdef hpux
    lastfdesc = _NFILE - 1;
#else
    lastfdesc = getdtablesize() - 1;
#endif
#endif

    if (lastfdesc > MAXSOCKS)
    {
	lastfdesc = MAXSOCKS;
	if (debug_conns)
	    ErrorF( "GOT TO END OF SOCKETS %d\n", MAXSOCKS);
    }

    FD_ZERO (&WellKnownConnections);

    sprintf (port, "%d", atoi (display));

    if ((_XSERVTransMakeAllCOTSServerListeners (port, &partial,
	&ListenTransCount, &ListenTransConns) >= 0) &&
	(ListenTransCount >= 1))
    {
	if (!PartialNetwork && partial)
	{
	    FatalError ("Failed to establish all listening sockets");
	}
	else
	{
	    ListenTransFds = (int *) malloc (ListenTransCount * sizeof (int));

	    for (i = 0; i < ListenTransCount; i++)
	    {
		int fd = _XSERVTransGetConnectionNumber (ListenTransConns[i]);
		
		ListenTransFds[i] = fd;
		FD_SET (fd, &WellKnownConnections);

		if (!_XSERVTransIsLocal (ListenTransConns[i]))
		{
		    DefineSelf (fd);
		}
	    }
	}
    }

    if (!XFD_ANYSET (&WellKnownConnections))
        FatalError ("Cannot establish any listening sockets - Make sure an X server isn't already running");

    OsSignal (SIGPIPE, SIG_IGN);
    OsSignal (SIGHUP, AutoResetServer);
    OsSignal (SIGINT, GiveUp);
    OsSignal (SIGTERM, GiveUp);
    XFD_COPYSET (&WellKnownConnections, &AllSockets);
    ResetHosts(display);
    /*
     * Magic:  If SIGUSR1 was set to SIG_IGN when
     * the server started, assume that either
     *
     *  a- The parent process is ignoring SIGUSR1
     *
     * or
     *
     *  b- The parent process is expecting a SIGUSR1
     *     when the server is ready to accept connections
     *
     * In the first case, the signal will be harmless,
     * in the second case, the signal will be quite
     * useful
     */
    if (OsSignal (SIGUSR1, SIG_IGN) == SIG_IGN)
	RunFromSmartParent = TRUE;
    ParentProcess = getppid ();
    if (RunFromSmartParent) {
	if (ParentProcess > 0) {
	    kill (ParentProcess, SIGUSR1);
	}
    }
#ifdef XDMCP
    XdmcpInit ();
#endif
}

void
ResetWellKnownSockets ()
{
    int i;

    ResetOsBuffers();

    for (i = 0; i < ListenTransCount; i++)
    {
	int status = _XSERVTransResetListener (ListenTransConns[i]);

	if (status != TRANS_RESET_NOOP)
	{
	    if (status == TRANS_RESET_FAILURE)
	    {
		/*
		 * ListenTransConns[i] freed by xtrans.
		 * Remove it from out list.
		 */

		FD_CLR (ListenTransFds[i], &WellKnownConnections);
		ListenTransFds[i] = ListenTransFds[ListenTransCount - 1];
		ListenTransConns[i] = ListenTransConns[ListenTransCount - 1];
		ListenTransCount -= 1;
		i -= 1;
	    }
	    else if (status == TRANS_RESET_NEW_FD)
	    {
		/*
		 * A new file descriptor was allocated (the old one was closed)
		 */

		int newfd = _XSERVTransGetConnectionNumber (ListenTransConns[i]);

		FD_CLR (ListenTransFds[i], &WellKnownConnections);
		ListenTransFds[i] = newfd;
		FD_SET(newfd, &WellKnownConnections);
	    }
	}
    }

    ResetAuthorization ();
    ResetHosts(display);
    /*
     * See above in CreateWellKnownSockets about SIGUSR1
     */
    if (RunFromSmartParent) {
	if (ParentProcess > 0) {
	    kill (ParentProcess, SIGUSR1);
	}
    }
    /*
     * restart XDMCP
     */
#ifdef XDMCP
    XdmcpReset ();
#endif
}

static void
AuthAudit (client, letin, saddr, len, proto_n, auth_proto)
    int client;
    Bool letin;
    struct sockaddr *saddr;
    int len;
    unsigned short proto_n;
    char *auth_proto;
{
    char addr[128];

    if (!len)
        strcpy(addr, "local host");
    else
	switch (saddr->sa_family)
	{
	case AF_UNSPEC:
#if defined(UNIXCONN) || defined(LOCALCONN)
	case AF_UNIX:
#endif
	    strcpy(addr, "local host");
	    break;
#if defined(TCPCONN) || defined(STREAMSCONN)
	case AF_INET:
	    sprintf(addr, "IP %s port %d",
		    inet_ntoa(((struct sockaddr_in *) saddr)->sin_addr),
		    ((struct sockaddr_in *) saddr)->sin_port);
	    break;
#endif
#ifdef DNETCONN
	case AF_DECnet:
	    sprintf(addr, "DN %s",
		    dnet_ntoa(&((struct sockaddr_dn *) saddr)->sdn_add));
	    break;
#endif
	default:
	    strcpy(addr, "unknown address");
	}
    if (letin)
	AuditF("client %d connected from %s\n", client, addr);
    else
	AuditF("client %d rejected from %s\n", client, addr);
    if (proto_n)
	AuditF("  Auth name: %.*s\n", proto_n, auth_proto);
}

/*****************************************************************
 * ClientAuthorized
 *
 *    Sent by the client at connection setup:
 *                typedef struct _xConnClientPrefix {
 *                   CARD8	byteOrder;
 *                   BYTE	pad;
 *                   CARD16	majorVersion, minorVersion;
 *                   CARD16	nbytesAuthProto;    
 *                   CARD16	nbytesAuthString;   
 *                 } xConnClientPrefix;
 *
 *     	It is hoped that eventually one protocol will be agreed upon.  In the
 *        mean time, a server that implements a different protocol than the
 *        client expects, or a server that only implements the host-based
 *        mechanism, will simply ignore this information.
 *
 *****************************************************************/

char * 
ClientAuthorized(client, proto_n, auth_proto, string_n, auth_string)
    ClientPtr client;
    char *auth_proto, *auth_string;
    unsigned int proto_n, string_n;
{
    register OsCommPtr 	priv;
    Xtransaddr		*from = NULL;
    int 		family;
    int			fromlen;
    XID	 		auth_id;
    char	 	*reason = NULL;

    auth_id = CheckAuthorization (proto_n, auth_proto,
				  string_n, auth_string, client, &reason);

    priv = (OsCommPtr)client->osPrivate;
    if (auth_id == (XID) ~0L)
    {
	if (_XSERVTransGetPeerAddr (priv->trans_conn,
	    &family, &fromlen, &from) != -1)
	{
	    if (InvalidHost ((struct sockaddr *) from, fromlen))
		AuthAudit(client->index, FALSE,
		    (struct sockaddr *) from, fromlen, proto_n, auth_proto);
	    else
	    {
		auth_id = (XID) 0;
		if (auditTrailLevel > 1)
		    AuthAudit(client->index, TRUE,
			(struct sockaddr *) from, fromlen,
			proto_n, auth_proto);
	    }

	    free ((char *) from);
	}

	if (auth_id == (XID) ~0L)
	    if (reason)
		return reason;
	    else
		return "Client is not authorized to connect to Server";
    }
    else if (auditTrailLevel > 1)
    {
	if (_XSERVTransGetPeerAddr (priv->trans_conn,
	    &family, &fromlen, &from) != -1)
	{
	    AuthAudit(client->index, TRUE, (struct sockaddr *) from, fromlen,
		      proto_n, auth_proto);

	    free ((char *) from);
	}
    }

    priv->auth_id = auth_id;
    priv->conn_time = 0;

#ifdef XDMCP
    /* indicate to Xdmcp protocol that we've opened new client */
    XdmcpOpenDisplay(priv->fd);
#endif /* XDMCP */
    /* At this point, if the client is authorized to change the access control
     * list, we should getpeername() information, and add the client to
     * the selfhosts list.  It's not really the host machine, but the
     * true purpose of the selfhosts list is to see who may change the
     * access control list.
     */
    return((char *)NULL);
}

#ifdef LBX

int
ClientConnectionNumber (client)
    ClientPtr	client;
{
    OsCommPtr oc = (OsCommPtr) client->osPrivate;

    return oc->fd;
}

AvailableClientInput (client)
    ClientPtr	client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;

    if (FD_ISSET(oc->fd, &AllSockets))
	FD_SET(oc->fd, &ClientsWithInput);
}

ClientPtr
AllocNewConnection (trans_conn, fd, Read, Writev, Close)
    XtransConnInfo trans_conn;
    int	    fd;
    int	    (*Read)();
    int	    (*Writev)();
    void    (*Close)();
{
    OsCommPtr	oc;
    ClientPtr	client;
    
    if (fd >= lastfdesc)
	return NullClient;
    oc = (OsCommPtr)xalloc(sizeof(OsCommRec));
    if (!oc)
	return NullClient;
    oc->trans_conn = trans_conn;
    oc->fd = fd;
    oc->input = (ConnectionInputPtr)NULL;
    oc->output = (ConnectionOutputPtr)NULL;
    oc->conn_time = GetTimeInMillis();
    oc->Read = Read;
    oc->Writev = Writev;
    oc->Close = Close;
    oc->flushClient = StandardFlushClient;
    oc->ofirst = (ConnectionOutputPtr) NULL;
    if (!(client = NextAvailableClient((pointer)oc)))
    {
	xfree (oc);
	return NullClient;
    }
    if (!ConnectionTranslation[fd])
    {
	ConnectionTranslation[fd] = client->index;
	ConnectionOutputTranslation[fd] = client->index;
	if (GrabInProgress)
	{
	    FD_SET(fd, &SavedAllClients);
	    FD_SET(fd, &SavedAllSockets);
	}
	else
	{
	    FD_SET(fd, &AllClients);
	    FD_SET(fd, &AllSockets);
	}
    }
    client->public.readRequest = StandardReadRequestFromClient;
    client->public.writeToClient = StandardWriteToClient;
    client->public.uncompressedWriteToClient = UncompressWriteToClient;
    client->public.requestLength = StandardRequestLength;
    return client;
}

ClientPtr
AllocPiggybackConnection (client, Read, Writev, Close)
    ClientPtr client;
    int	    (*Read)();
    int	    (*Writev)();
    void    (*Close)();
{
    OsCommPtr oc = (OsCommPtr) client->osPrivate;
    return AllocNewConnection(oc->trans_conn, oc->fd, Read, Writev, Close);
}

void
SwitchConnectionFuncs (client, Read, Writev, Close)
    ClientPtr	client;
    int		(*Read)();
    int		(*Writev)();
    void	(*Close)();
{
    OsCommPtr	oc = (OsCommPtr) client->osPrivate;

    oc->Read = Read;
    oc->Writev = Writev;
    oc->Close = Close;
    oc->conn_time = 0;
}

void
StartOutputCompression(client, CompressOn, CompressOff)
    ClientPtr	client;
    void	(*CompressOn)();
    void	(*CompressOff)();
{
    OsCommPtr	oc = (OsCommPtr) client->osPrivate;
    extern int	LbxFlushClient();

    oc->compressOn = CompressOn;
    oc->compressOff = CompressOff;
    oc->flushClient = LbxFlushClient;
}
#endif

/*****************
 * EstablishNewConnections
 *    If anyone is waiting on listened sockets, accept them.
 *    Returns a mask with indices of new clients.  Updates AllClients
 *    and AllSockets.
 *****************/

/*ARGSUSED*/
Bool
EstablishNewConnections(clientUnused, closure)
    ClientPtr clientUnused;
    pointer closure;
{
    fd_mask readyconnections;     /* mask of listeners that are ready */
    int curconn;                  /* fd of listener that's ready */
    register int newconn;         /* fd of new client */
    CARD32 connect_time;
    register int i;
    register ClientPtr client;
    register OsCommPtr oc;
    fd_set tmask;
#ifdef LBX
    extern int  writev(), close();
#endif

    XFD_ANDSET (&tmask, (fd_set*)closure, &WellKnownConnections);
    readyconnections = tmask.fds_bits[0];
    if (!readyconnections)
	return TRUE;
    connect_time = GetTimeInMillis();
    /* kill off stragglers */
    for (i=1; i<currentMaxClients; i++)
    {
	if (client = clients[i])
	{
	    oc = (OsCommPtr)(client->osPrivate);
	    if (oc && (oc->conn_time != 0) &&
		(connect_time - oc->conn_time) >= TimeOutValue)
		CloseDownClient(client);     
	}
    }
    while (readyconnections) 
    {
	XtransConnInfo trans_conn, new_trans_conn;
	int status;

	curconn = ffs (readyconnections) - 1;
	readyconnections &= ~(1 << curconn);

	if ((trans_conn = lookup_trans_conn (curconn)) == NULL)
	    continue;

	if ((new_trans_conn = _XSERVTransAccept (trans_conn, &status)) == NULL)
	    continue;

	newconn = _XSERVTransGetConnectionNumber (new_trans_conn);

	_XSERVTransSetOption(new_trans_conn, TRANS_NONBLOCKING, 1);

#ifdef LBX
	client = AllocNewConnection (new_trans_conn, newconn,
		read, writev, CloseDownFileDescriptor);
	if (!client)
	{
	    ErrorConnMax(new_trans_conn);
	    _XSERVTransClose(new_trans_conn);
	    continue;
	}
#else
	oc = (OsCommPtr)xalloc(sizeof(OsCommRec));
	if (!oc)
	{
	    ErrorConnMax(new_trans_conn);
	    _XSERVTransClose(new_trans_conn);
	    continue;
	}
	if (GrabInProgress)
	{
	    FD_SET(newconn, &SavedAllClients);
	    FD_SET(newconn, &SavedAllSockets);
	}
	else
	{
	    FD_SET(newconn, &AllClients);
	    FD_SET(newconn, &AllSockets);
	}
	oc->fd = newconn;
	oc->trans_conn = new_trans_conn;
	oc->input = (ConnectionInputPtr)NULL;
	oc->output = (ConnectionOutputPtr)NULL;
	oc->conn_time = connect_time;
	if ((newconn < lastfdesc) &&
	    (client = NextAvailableClient((pointer)oc)))
	{
	    ConnectionTranslation[newconn] = client->index;
	}
	else
	{
	    ErrorConnMax(new_trans_conn);
	    CloseDownFileDescriptor(oc);
	}
#endif	/* LBX */
    }
    return TRUE;
}

#define NOROOM "Maximum number of clients reached"

/************
 *   ErrorConnMax
 *     Fail a connection due to lack of client or file descriptor space
 ************/

static void
ErrorConnMax(trans_conn)
XtransConnInfo trans_conn;
{
    register int fd = _XSERVTransGetConnectionNumber (trans_conn);
    xConnSetupPrefix csp;
    char pad[3];
    struct iovec iov[3];
    char byteOrder = 0;
    int whichbyte = 1;
    struct timeval waittime;
    fd_set mask;

    /* if these seems like a lot of trouble to go to, it probably is */
    waittime.tv_sec = BOTIMEOUT / MILLI_PER_SECOND;
    waittime.tv_usec = (BOTIMEOUT % MILLI_PER_SECOND) *
		       (1000000 / MILLI_PER_SECOND);
    FD_ZERO(&mask);
    FD_SET(fd, &mask);
    (void)Select(fd + 1, &mask, NULL, NULL, &waittime);
    /* try to read the byte-order of the connection */
    (void)_XSERVTransRead(trans_conn, &byteOrder, 1);
    if ((byteOrder == 'l') || (byteOrder == 'B'))
    {
	csp.success = xFalse;
	csp.lengthReason = sizeof(NOROOM) - 1;
	csp.length = (sizeof(NOROOM) + 2) >> 2;
	csp.majorVersion = X_PROTOCOL;
	csp.minorVersion = X_PROTOCOL_REVISION;
	if (((*(char *) &whichbyte) && (byteOrder == 'B')) ||
	    (!(*(char *) &whichbyte) && (byteOrder == 'l')))
	{
	    swaps(&csp.majorVersion, whichbyte);
	    swaps(&csp.minorVersion, whichbyte);
	    swaps(&csp.length, whichbyte);
	}
	iov[0].iov_len = sz_xConnSetupPrefix;
	iov[0].iov_base = (char *) &csp;
	iov[1].iov_len = csp.lengthReason;
	iov[1].iov_base = NOROOM;
	iov[2].iov_len = (4 - (csp.lengthReason & 3)) & 3;
	iov[2].iov_base = pad;
	(void)_XSERVTransWritev(trans_conn, iov, 3);
    }
}

/************
 *   CloseDownFileDescriptor:
 *     Remove this file descriptor and it's I/O buffers, etc.
 ************/

#ifdef LBX
void
CloseDownFileDescriptor(client)
    ClientPtr	client;
#else
static void
CloseDownFileDescriptor(oc)
    register OsCommPtr oc;
#endif
{
#ifdef LBX
    register OsCommPtr oc = (OsCommPtr) client->osPrivate;
#endif
    int connection = oc->fd;

    if (oc->trans_conn)
	_XSERVTransClose(oc->trans_conn);
#ifdef LBX
    ConnectionTranslation[connection] = 0;
    ConnectionOutputTranslation[connection] = 0;
#else
    FreeOsBuffers(oc);
#endif
    FD_CLR(connection, &AllSockets);
    FD_CLR(connection, &AllClients);
    FD_CLR(connection, &ClientsWithInput);
    FD_CLR(connection, &GrabImperviousClients);
    if (GrabInProgress)
    {
	FD_CLR(connection, &SavedAllSockets);
	FD_CLR(connection, &SavedAllClients);
	FD_CLR(connection, &SavedClientsWithInput);
    }
    FD_CLR(connection, &ClientsWriteBlocked);
    if (!XFD_ANYSET(&ClientsWriteBlocked))
    	AnyClientsWriteBlocked = FALSE;
    FD_CLR(connection, &OutputPending);
#ifndef LBX
    xfree(oc);
#endif
}

/*****************
 * CheckConections
 *    Some connection has died, go find which one and shut it down 
 *    The file descriptor has been closed, but is still in AllClients.
 *    If would truly be wonderful if select() would put the bogus
 *    file descriptors in the exception mask, but nooooo.  So we have
 *    to check each and every socket individually.
 *****************/

void
CheckConnections()
{
    fd_mask		mask;
    fd_set		tmask; 
    register int	curclient, curoff;
    int			i;
    struct timeval	notime;
    int r;

    notime.tv_sec = 0;
    notime.tv_usec = 0;

    for (i=0; i<howmany(XFD_SETSIZE, NFDBITS); i++)
    {
	mask = AllClients.fds_bits[i];
        while (mask)
    	{
	    curoff = ffs (mask) - 1;
 	    curclient = curoff + (i << 5);
            FD_ZERO(&tmask);
            FD_SET(curclient, &tmask);
            r = Select (curclient + 1, &tmask, NULL, NULL, &notime);
            if (r < 0)
		CloseDownClient(clients[ConnectionTranslation[curclient]]);
	    mask &= ~(1 << curoff);
	}
    }	
}


/*****************
 * CloseDownConnection
 *    Delete client from AllClients and free resources 
 *****************/

void
CloseDownConnection(client)
    ClientPtr client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;

    if (oc->output && oc->output->count)
	FlushClient(client, oc, (char *)NULL, 0);
#ifdef XDMCP
    XdmcpCloseDisplay(oc->fd);
#endif
#ifndef LBX
    CloseDownFileDescriptor(oc);
#else
    (*oc->Close) (client);
    FreeOsBuffers(oc);
    xfree(oc);
#endif
    client->osPrivate = (pointer)NULL;
    if (auditTrailLevel > 1)
	AuditF("client %d disconnected\n", client->index);
}


AddEnabledDevice(fd)
    int fd;
{
    FD_SET(fd, &EnabledDevices);
    FD_SET(fd, &AllSockets);
}


RemoveEnabledDevice(fd)
    int fd;
{
    FD_CLR(fd, &EnabledDevices);
    FD_CLR(fd, &AllSockets);
}

/*****************
 * OnlyListenToOneClient:
 *    Only accept requests from  one client.  Continue to handle new
 *    connections, but don't take any protocol requests from the new
 *    ones.  Note that if GrabInProgress is set, EstablishNewConnections
 *    needs to put new clients into SavedAllSockets and SavedAllClients.
 *    Note also that there is no timeout for this in the protocol.
 *    This routine is "undone" by ListenToAllClients()
 *****************/

OnlyListenToOneClient(client)
    ClientPtr client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    if (! GrabInProgress)
    {
	XFD_COPYSET(&ClientsWithInput, &SavedClientsWithInput);
	XFD_ANDSET(&ClientsWithInput,
		       &ClientsWithInput, &GrabImperviousClients);
	if (FD_ISSET(connection, &SavedClientsWithInput))
	{
	    FD_CLR(connection, &SavedClientsWithInput);
	    FD_SET(connection, &ClientsWithInput);
	}
	XFD_UNSET(&SavedClientsWithInput, &GrabImperviousClients);
	XFD_COPYSET(&AllSockets, &SavedAllSockets);
	XFD_COPYSET(&AllClients, &SavedAllClients);
	XFD_UNSET(&AllSockets, &AllClients);
	XFD_ANDSET(&AllClients, &AllClients, &GrabImperviousClients);
	FD_SET(connection, &AllClients);
	XFD_ORSET(&AllSockets, &AllSockets, &AllClients);
	GrabInProgress = client->index;
    }
}

/****************
 * ListenToAllClients:
 *    Undoes OnlyListentToOneClient()
 ****************/

ListenToAllClients()
{
    if (GrabInProgress)
    {
	XFD_ORSET(&AllSockets, &AllSockets, &SavedAllSockets);
	XFD_ORSET(&AllClients, &AllClients, &SavedAllClients);
	XFD_ORSET(&ClientsWithInput, &ClientsWithInput, &SavedClientsWithInput);
	GrabInProgress = 0;
    }	
}

/****************
 * IgnoreClient
 *    Removes one client from input masks.
 *    Must have cooresponding call to AttendClient.
 ****************/

IgnoreClient (client)
    ClientPtr	client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    if (!GrabInProgress || FD_ISSET(connection, &AllClients))
    {
    	if (FD_ISSET (connection, &ClientsWithInput))
	    FD_SET(connection, &IgnoredClientsWithInput);
    	else
	    FD_CLR(connection, &IgnoredClientsWithInput);
    	FD_CLR(connection, &ClientsWithInput);
    	FD_CLR(connection, &AllSockets);
    	FD_CLR(connection, &AllClients);
	FD_CLR(connection, &LastSelectMask);
    }
    else
    {
    	if (FD_ISSET (connection, &SavedClientsWithInput))
	    FD_SET(connection, &IgnoredClientsWithInput);
    	else
	    FD_CLR(connection, &IgnoredClientsWithInput);
	FD_CLR(connection, &SavedClientsWithInput);
	FD_CLR(connection, &SavedAllSockets);
	FD_CLR(connection, &SavedAllClients);
    }
    isItTimeToYield = TRUE;
}

/****************
 * AttendClient
 *    Adds one client back into the input masks.
 ****************/

AttendClient (client)
    ClientPtr	client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    if (!GrabInProgress || GrabInProgress == client->index ||
	FD_ISSET(connection, &GrabImperviousClients))
    {
    	FD_SET(connection, &AllClients);
    	FD_SET(connection, &AllSockets);
	FD_SET(connection, &LastSelectMask);
    	if (FD_ISSET (connection, &IgnoredClientsWithInput))
	    FD_SET(connection, &ClientsWithInput);
    }
    else
    {
	FD_SET(connection, &SavedAllClients);
	FD_SET(connection, &SavedAllSockets);
	if (FD_ISSET(connection, &IgnoredClientsWithInput))
	    FD_SET(connection, &SavedClientsWithInput);
    }
}

/* make client impervious to grabs; assume only executing client calls this */

MakeClientGrabImpervious(client)
    ClientPtr client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    FD_SET(connection, &GrabImperviousClients);

    if (ServerGrabCallback)
    {
	ServerGrabInfoRec grabinfo;
	grabinfo.client = client;
	grabinfo.grabstate  = CLIENT_IMPERVIOUS;
	CallCallbacks(&ServerGrabCallback, &grabinfo);
    }
}

/* make client pervious to grabs; assume only executing client calls this */

MakeClientGrabPervious(client)
    ClientPtr client;
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    FD_CLR(connection, &GrabImperviousClients);
    if (GrabInProgress && (GrabInProgress != client->index))
    {
	if (FD_ISSET(connection, &ClientsWithInput))
	{
	    FD_SET(connection, &SavedClientsWithInput);
	    FD_CLR(connection, &ClientsWithInput);
	}
	FD_CLR(connection, &AllSockets);
	FD_CLR(connection, &AllClients);
	isItTimeToYield = TRUE;
    }

    if (ServerGrabCallback)
    {
	ServerGrabInfoRec grabinfo;
	grabinfo.client = client;
	grabinfo.grabstate  = CLIENT_PERVIOUS;
	CallCallbacks(&ServerGrabCallback, &grabinfo);
    }
}

#ifdef AIXV3

static fd_set pendingActiveClients;
static BOOL reallyGrabbed;

/****************
* DontListenToAnybody:
*   Don't listen to requests from any clients. Continue to handle new
*   connections, but don't take any protocol requests from anybody.
*   We have to take care if there is already a grab in progress, though.
*   Undone by PayAttentionToClientsAgain. We also have to be careful
*   not to accept any more input from the currently dispatched client.
*   we do this be telling dispatch it is time to yield.

*   We call this when the server loses access to the glass
*   (user hot-keys away).  This looks like a grab by the 
*   server itself, but gets a little tricky if there is already
*   a grab in progress.
******************/

void
DontListenToAnybody()
{
    if (!GrabInProgress)
    {
	XFD_COPYSET(&ClientsWithInput, &SavedClientsWithInput);
	XFD_COPYSET(&AllSockets, &SavedAllSockets);
	XFD_COPYSET(&AllClients, &SavedAllClients);
	GrabInProgress = TRUE;
	reallyGrabbed = FALSE;
    }
    else
    {
	XFD_COPYSET(&AllClients, &pendingActiveClients);
	reallyGrabbed = TRUE;
    }
    FD_ZERO(&ClientsWithInput);
    XFD_UNSET(&AllSockets, &AllClients);
    FD_ZERO(&AllClients);
    isItTimeToYield = TRUE;
}

void
PayAttentionToClientsAgain()
{
    if (reallyGrabbed)
    {
	XFD_ORSET(&AllSockets, &AllSockets, &pendingActiveClients);
	XFD_ORSET(&AllClients, &AllClients, &pendingActiveClients);
    }
    else
    {
	ListenToAllClients();
    }
    reallyGrabbed = FALSE;
}

#endif
