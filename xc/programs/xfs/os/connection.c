/*
 * handles connections
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
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
 *
 * @(#)connection.c	4.1	5/2/91
 *
 */

#include	<stdio.h>
#include	<errno.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<signal.h>
#include	<fcntl.h>

#ifdef TCPCONN
#include	<netinet/in.h>
#include	<netinet/tcp.h>
#endif

#include	<X11/Xos.h>
#include	"FS.h"
#include	"FSproto.h"
#include	"clientstr.h"
#include	"osdep.h"
#include	"globals.h"
#include	"osstruct.h"

extern int  errno;

int         serverNum;		/* the server number */
int         lastfdesc;

long        WellKnownConnections;
long        AllSockets[mskcnt];
long        AllClients[mskcnt];
long        LastSelectMask[mskcnt];
long        ClientsWithInput[mskcnt];
long        ClientsWriteBlocked[mskcnt];
long        OutputPending[mskcnt];
extern long MaxClients;
long        OutputBufferSize = BUFSIZE;

Bool        NewOutputPending;
Bool        AnyClientsWriteBlocked;

int         ConnectionTranslation[MAXSOCKS];

extern ClientPtr NextAvailableClient();
extern int  AutoResetServer();
extern int  GiveUp();
extern int  ServerReconfig();
extern int  ServerCacheFlush();

extern void FreeOsBuffers();

static void error_conn_max();
static void close_fd();


#ifdef	TCPCONN
static int
open_tcp_socket()
{
    struct sockaddr_in insock;
    int         request;
    int         retry;

#ifndef SO_DONTLINGER

#ifdef SO_LINGER
    static int  linger[2] = {0, 0};

#endif				/* SO_LINGER */

#endif				/* SO_DONTLINGER */

    if ((request = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	Error("Creating TCP socket");
	return -1;
    }

#ifdef SO_REUSEADDR

#ifdef hpux
    set_sockt_option(request, SO_REUSEADDR);
#else
    {
	int         one = 1;

	setsockopt(request, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    }
#endif

#endif				/* SO_REUSEADDR */

    bzero((char *) &insock, sizeof(insock));
    insock.sin_family = AF_INET;
    insock.sin_port = htons((unsigned short) (FS_TCP_PORT + serverNum));
    insock.sin_addr.s_addr = htonl(INADDR_ANY);
    retry = 20;
    while (bind(request, (struct sockaddr *) & insock, sizeof(insock))) {
	if (--retry == 0) {
	    Error("Binding TCP socket");
	    close(request);
	    return -1;
	}

#ifdef SO_REUSEADDR
	sleep(1);
#else
	sleep(10);
#endif				/* SO_REUSEADDR */
    }

#ifdef SO_DONTLINGER
    if (setsockopt(request, SOL_SOCKET, SO_DONTLINGER, (char *) NULL, 0))
	Error("Setting TCP SO_DONTLINGER");
#else

#ifdef SO_LINGER
    if (setsockopt(request, SOL_SOCKET, SO_LINGER,
		   (char *) linger, sizeof(linger)))
	Error("Setting TCP SO_LINGER");
#endif				/* SO_LINGER */

#endif				/* SO_DONTLINGER */

    if (listen(request, 5)) {
	Error("TCP listening");
	close(request);
	return -1;
    }
    return request;
}

#endif				/* TCPCONN */


/*
 * creates the sockets for listening to clients
 *
 * only called when server first started
 */
void
CreateSockets()
{
    int         request,
                i;

    CLEARBITS(AllSockets);
    CLEARBITS(AllClients);
    CLEARBITS(LastSelectMask);
    CLEARBITS(ClientsWithInput);

    for (i = 0; i < MAXSOCKS; i++)
	ConnectionTranslation[i] = 0;

    lastfdesc = getdtablesize() - 1;

    if (lastfdesc > MAXSOCKS) {
	lastfdesc = MAXSOCKS;
    }
    WellKnownConnections = 0;

#ifdef TCPCONN
    if ((request = open_tcp_socket()) != -1) {
	WellKnownConnections |= (1L << request);
    }
#endif				/* TCPCONN */

    if (WellKnownConnections == 0)
	FatalError("Cannot establish any listening sockets");

    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, AutoResetServer);
    signal(SIGINT, GiveUp);
    signal(SIGTERM, GiveUp);
    signal(SIGUSR1, ServerReconfig);
    signal(SIGUSR1, ServerCacheFlush);

    AllSockets[0] = WellKnownConnections;
}

/*
 * called when server cycles
 */
ResetSockets()
{
}

/*
 * accepts new connections
 */
void
MakeNewConnections()
{
    long        readyconnections;
    int         curconn;
    int         newconn;
    long        connect_time;
    int         i;
    ClientPtr   client;
    OsCommPtr   oc;
    char       *ibuf;
    unsigned char *obuf;

#ifdef TCP_NODELAY
    union {
	struct sockaddr sa;

#ifdef TCPCONN
	struct sockaddr_in in;
#endif				/* TCPCONN */
    }           from;
    int         fromlen;

#endif				/* TCP_NODELAY */

    readyconnections = (LastSelectMask[0] & WellKnownConnections);
    if (!readyconnections)
	return;
    connect_time = GetTimeInMillis();

#ifdef notyet	
/* XXX this seems a bit brutal for the FS environment */
/* this is probably the best place to spit out KeepAlive events instead */
    /* kill off stragglers */
    for (i = MINCLIENT; i < currentMaxClients; i++) {
	if ((client = clients[i]) != NullClient) {
	    oc = (OsCommPtr) client->osPrivate;
	    if (oc && (oc->conn_time != 0) &&
		    (connect_time - oc->conn_time) >= TimeOutValue)
		CloseDownClient(client);
	}
    }
#endif

    while (readyconnections) {
	curconn = ffs(readyconnections) - 1;
	readyconnections &= ~(1 << curconn);
	if ((newconn = accept(curconn, (struct sockaddr *) NULL,
			      (int *) NULL)) < 0)
	    continue;

#ifdef TCP_NODELAY
	fromlen = sizeof(from);
	if (!getpeername(newconn, &from.sa, &fromlen)) {
	    if (fromlen && (from.sa.sa_family == AF_INET)) {
		int         mi = 1;

		setsockopt(newconn, IPPROTO_TCP, TCP_NODELAY,
			   (char *) &mi, sizeof(int));
	    }
	}
#endif				/* TCP_NODELAY */

#ifdef hpux
#else
	fcntl(newconn, F_SETFL, FNDELAY);
#endif				/* hpux */

	oc = (OsCommPtr) fsalloc(sizeof(OsCommRec));
	ibuf = (char *) fsalloc(BUFSIZE);
	obuf = (unsigned char *) fsalloc(OutputBufferSize);
	if (!oc || !ibuf || !obuf) {
	    fsfree(oc);
	    error_conn_max(newconn);
	    close(newconn);
	    continue;
	}
	BITSET(AllClients, newconn);
	BITSET(AllSockets, newconn);
	oc->fd = newconn;
	oc->input = (ConnectionInputPtr) NULL;
	oc->output = (ConnectionOutputPtr) NULL;
	oc->conn_time = connect_time;

	if ((newconn < lastfdesc) &&
		(client = NextAvailableClient((pointer) oc))) {
	    ConnectionTranslation[newconn] = client->index;
	} else {
	    error_conn_max(newconn);
	    close_fd(oc);
	}
    }
}

#define	NOROOM	"Maximum number of clients reached"

static void
error_conn_max(fd)
    int         fd;
{
    fsConnSetup conn;
    char        pad[3];
    char        byteOrder = 0;
    int         whichbyte = 1;
    struct timeval waittime;
    long        mask[mskcnt];


    waittime.tv_usec = BOTIMEOUT / MILLI_PER_SECOND;
    waittime.tv_usec = (BOTIMEOUT % MILLI_PER_SECOND) *
	(1000000 / MILLI_PER_SECOND);
    CLEARBITS(mask);
    BITSET(mask, fd);
    (void) select(fd + 1, (int *) mask, (int *) NULL, (int *) NULL, &waittime);
    /* try to read the byteorder of the connection */
    (void) read(fd, &byteOrder, 1);
    if ((byteOrder == 'l') || (byteOrder == 'B')) {
	int         num_alts;
	AlternateServerPtr altservers,
	            as;
	int         i,
	            altlen = 0;

	num_alts = ListAlternateServers(&altservers);
	conn.status = AuthDenied;
	conn.major_version = FS_PROTOCOL;
	conn.minor_version = FS_PROTOCOL_MINOR;
	conn.num_alternates = num_alts;
	for (i = 0, as = altservers; i < num_alts; i++, as++) {
	    altlen += (2 + as->namelen + 3) >> 2;
	}
	conn.alternate_len = altlen;
	/* blow off the auth info */
	conn.auth_index = 0;
	conn.auth_len = 0;

	if (((*(char *) &whichbyte) && (byteOrder == 'B')) ||
		(!(*(char *) &whichbyte) && (byteOrder == 'l'))) {
	    swaps(&conn.status, whichbyte);
	    swaps(&conn.major_version, whichbyte);
	    swaps(&conn.minor_version, whichbyte);
	    swaps(&conn.alternate_len, whichbyte);
	}
	(void) write(fd, (char *) &conn, sizeof(fsConnSetup));
	/* dump alternates */
	for (i = 0, as = altservers; i < num_alts; i++, as++) {
	    (void) write(fd, (char *) as, 2);	/* XXX */
	    (void) write(fd, (char *) as->name,
			 as->namelen);
	}
	/* pad it */
	if (altlen & 3) {
	    (void) write(fd, (char *) pad, ((4 - (altlen & 3)) & 3));
	}
	if (num_alts)
	    fsfree((char *) altservers);
    }
}

static void
close_fd(oc)
    OsCommPtr   oc;
{
    int         fd = oc->fd;

    close(fd);
    FreeOsBuffers(oc);
    BITCLEAR(AllSockets, fd);
    BITCLEAR(AllClients, fd);
    BITCLEAR(ClientsWithInput, fd);
    BITCLEAR(ClientsWriteBlocked, fd);
    if (!ANYSET(ClientsWriteBlocked))
	AnyClientsWriteBlocked = FALSE;
    BITCLEAR(OutputPending, fd);
    fsfree(oc);
}

CheckConnections()
{
    long        mask[mskcnt];
    long        tmask[mskcnt];
    int         curclient;
    int         i;
    struct timeval notime;
    int         r;

    notime.tv_sec = 0;
    notime.tv_usec = 0;

    COPYBITS(AllClients, mask);
    for (i = 0; i < mskcnt; i++) {
	while (mask[i]) {
	    curclient = ffs(mask[i]) - 1 + (i << 5);
	    CLEARBITS(tmask);
	    BITSET(tmask, curclient);
	    r = select(curclient + 1, (int *) tmask, (int *) NULL,
		       (int *) NULL, &notime);
	    if (r < 0)
		CloseDownClient(clients[ConnectionTranslation[curclient]]);
	    BITCLEAR(mask, curclient);
	}
    }
}

CloseDownConnection(client)
    ClientPtr   client;
{
    OsCommPtr   oc = (OsCommPtr) client->osPrivate;

    if (oc->output && oc->output->count)
	FlushClient(client, oc, (char *) NULL, 0);
    ConnectionTranslation[oc->fd] = 0;
    close_fd(oc);
    client->osPrivate = (pointer) NULL;
}


/****************
 * IgnoreClient
 *    Removes one client from input masks.
 *    Must have cooresponding call to AttendClient.
 ****************/

static long IgnoredClientsWithInput[mskcnt];

IgnoreClient(client)
    ClientPtr   client;
{
    OsCommPtr   oc = (OsCommPtr) client->osPrivate;
    int         connection = oc->fd;

    if (GETBIT(ClientsWithInput, connection))
	BITSET(IgnoredClientsWithInput, connection);
    else
	BITCLEAR(IgnoredClientsWithInput, connection);
    BITCLEAR(ClientsWithInput, connection);
    BITCLEAR(AllSockets, connection);
    BITCLEAR(AllClients, connection);
    BITCLEAR(LastSelectMask, connection);
    isItTimeToYield = TRUE;
}

/****************
 * AttendClient
 *    Adds one client back into the input masks.
 ****************/

AttendClient(client)
    ClientPtr   client;
{
    OsCommPtr   oc = (OsCommPtr) client->osPrivate;
    int         connection = oc->fd;

    BITSET(AllClients, connection);
    BITSET(AllSockets, connection);
    BITSET(LastSelectMask, connection);
    if (GETBIT(IgnoredClientsWithInput, connection))
	BITSET(ClientsWithInput, connection);
}
