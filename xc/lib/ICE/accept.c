/* $XConsortium: accept.c,v 1.12 93/11/18 11:36:18 mor Exp $ */
/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
******************************************************************************/

#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICElibint.h>

#include <stdio.h>
#include <sys/types.h>
#ifndef WIN32

#include <sys/socket.h>

#ifdef UNIXCONN
#include <sys/un.h>
# ifndef ICE_UNIX_DIR
#   define ICE_UNIX_DIR "/tmp/.ICE-unix"
# endif
static int _IceUnixDomainConnection = -1;
#endif

#ifdef TCPCONN
# include <sys/param.h>
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

#ifdef DNETCONN
#include <netdnet/dn.h>
#endif

#else

#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#include <X11/Xw32defs.h>
#undef close
#define close closesocket

#endif

#define MAX_LISTEN_CONNECTIONS 3  	/* TCP, DECnet, and Unix Domain */

#ifdef UNIXCONN
static int open_unix_socket ();
#endif
#ifdef TCPCONN
static int open_tcp_socket ();
#endif
#ifdef DNETCONN
static int open_dnet_socket ();
#endif


static int   _listen_count = 0;
static int   _listen_descrips[MAX_LISTEN_CONNECTIONS];
static char *_listen_network_ids[MAX_LISTEN_CONNECTIONS];



Status
IceListenForConnections (countRet, descripsRet, networkIdsRet,
    errorLength, errorStringRet)

int  *countRet;
int  **descripsRet;
char **networkIdsRet;
int  errorLength;
char *errorStringRet;

{
    int  networkIdsLen = 0;
    int  fd;

#ifdef UNIXCONN
    char *unix_networkId = NULL;
    int  unix_networkId_len = 0;
#endif
#ifdef TCPCONN
    char *tcp_networkId = NULL;
    int  tcp_networkId_len = 0;
#endif
#ifdef DNETCONN
    char *dnet_networkId = NULL;
    int  dnet_networkId_len = 0;
#endif

    *countRet = 0;
    *networkIdsRet = NULL;
    if (errorStringRet && errorLength > 0)
	*errorStringRet = '\0';

    /*
     * For each network interface available, open a socket for
     * listening to new connections.
     */

    *descripsRet = (int *) malloc (MAX_LISTEN_CONNECTIONS * sizeof (int));

#ifdef UNIXCONN
    if ((fd = open_unix_socket (&unix_networkId)) != -1)
    {
	(*descripsRet)[*countRet] = _IceUnixDomainConnection = fd;
        unix_networkId_len = strlen (unix_networkId);
	networkIdsLen += (unix_networkId_len + 1);

	_listen_descrips[*countRet] = fd;
	_listen_network_ids[*countRet] = (char *) malloc (
	    unix_networkId_len + 1);
	strcpy (_listen_network_ids[*countRet], unix_networkId);

	(*countRet)++;
    }
    else
    {
	strncpy (errorStringRet,
	    "Cannot establish unix listening socket", errorLength);
    }
#endif
#ifdef TCPCONN
    if ((fd = open_tcp_socket (&tcp_networkId)) != -1)
    {
	(*descripsRet)[*countRet] = fd;
        tcp_networkId_len = strlen (tcp_networkId);
	networkIdsLen += (tcp_networkId_len + 1);

	_listen_descrips[*countRet] = fd;
	_listen_network_ids[*countRet] = (char *) malloc (
	    tcp_networkId_len + 1);
	strcpy (_listen_network_ids[*countRet], tcp_networkId);

	(*countRet)++;
    }
    else
    {
	strncpy (errorStringRet,
	    "Cannot establish tcp listening socket", errorLength);
    }
#endif
#ifdef DNETCONN
    if ((fd = open_dnet_socket (&dnet_networkId)) != -1)
    {
	(*descripsRet)[*countRet] = fd;
        dnet_networkId_len = strlen (dnet_networkId);
	networkIdsLen += (dnet_networkId_len + 1);

	_listen_descrips[*countRet] = fd;
	_listen_network_ids[*countRet] = (char *) malloc (
	    dnet_networkId_len + 1);
	strcpy (_listen_network_ids[*countRet], dnet_networkId);

	(*countRet)++;
    }
    else
    {
	strncpy (errorStringRet,
	    "Cannot establish dnet listening socket", errorLength);
    }
#endif

    _listen_count = *countRet;

    if (*countRet == 0)
    {
        strncpy (errorStringRet,
	    "Cannot establish any listening sockets", errorLength);
	return (0);
    }
    else
    {
	/*
	 * Now build the string containing the list of network ids
	 * the client can connect to.
	 */

	*networkIdsRet = (char *) malloc (networkIdsLen + 1);
	(*networkIdsRet)[0] = '\0';

#ifdef UNIXCONN
	if (unix_networkId)
	{
	    strcat (*networkIdsRet, unix_networkId);
	    strcat (*networkIdsRet, ",");
	    free (unix_networkId);
	}
#endif
#ifdef TCPCONN
	if (tcp_networkId)
	{
	    strcat (*networkIdsRet, tcp_networkId);
	    strcat (*networkIdsRet, ",");
	    free (tcp_networkId);
	}
#endif
#ifdef DNETCONN
	if (dnet_networkId)
	{
	    strcat (*networkIdsRet, dnet_networkId);
	    strcat (*networkIdsRet, ",");
	    free (dnet_networkId);
	}
#endif

	(*networkIdsRet)[strlen (*networkIdsRet) - 1] = '\0';
	
	return (1);
    }
}



void
IceSetAuthenticationData (numEntries, entries)

int			numEntries;
IceAuthDataEntry	*entries;

{
    int i;

    if (_IceAuthDataEntries)
    {
	for (i = 0; i < _IceAuthDataEntryCount; i++)
	{
	    free (_IceAuthDataEntries[i].protocol_name);
	    free (_IceAuthDataEntries[i].protocol_data);
	    free (_IceAuthDataEntries[i].address_list);
	    free (_IceAuthDataEntries[i].auth_name);
	    free (_IceAuthDataEntries[i].auth_data);
	}

	free ((char *) _IceAuthDataEntries);
    }

    _IceAuthDataEntryCount = numEntries;

    _IceAuthDataEntries = (IceAuthDataEntry *) malloc (
	numEntries * sizeof (IceAuthDataEntry));

    for (i = 0; i < numEntries; i++)
    {
	_IceAuthDataEntries[i].protocol_name_length =
	    entries[i].protocol_name_length;
	_IceAuthDataEntries[i].protocol_data_length =
	    entries[i].protocol_data_length;
	_IceAuthDataEntries[i].address_list_length =
            entries[i].address_list_length;
	_IceAuthDataEntries[i].auth_name_length =
            entries[i].auth_name_length;
	_IceAuthDataEntries[i].auth_data_length =
            entries[i].auth_data_length;

	_IceAuthDataEntries[i].protocol_name = (char *) malloc (
	    entries[i].protocol_name_length);
	_IceAuthDataEntries[i].protocol_data = (char *) malloc (
	    entries[i].protocol_data_length);
	_IceAuthDataEntries[i].address_list = (char *) malloc (
            entries[i].address_list_length);
	_IceAuthDataEntries[i].auth_name = (char *) malloc (
            entries[i].auth_name_length);
	_IceAuthDataEntries[i].auth_data = (char *) malloc (
            entries[i].auth_data_length);

	memcpy (_IceAuthDataEntries[i].protocol_name,
	    entries[i].protocol_name, entries[i].protocol_name_length);
	memcpy (_IceAuthDataEntries[i].protocol_data,
	    entries[i].protocol_data, entries[i].protocol_data_length);
	memcpy (_IceAuthDataEntries[i].address_list,
            entries[i].address_list, entries[i].address_list_length);
	memcpy (_IceAuthDataEntries[i].auth_name,
            entries[i].auth_name, entries[i].auth_name_length);
	memcpy (_IceAuthDataEntries[i].auth_data,
            entries[i].auth_data, entries[i].auth_data_length);
    }
}



void
IceSetHostBasedAuthProc (proc, clientData)

IceHostBasedAuthProc	proc;
IcePointer		clientData;

{
    _IceHostBasedAuthProc = proc;
    _IceHostBasedAuthProcClientData = clientData;
}



IceConn
IceAcceptConnection (fd)

int fd;

{
    IceConn    		iceConn;
    int        		newconn;
    iceByteOrderMsg 	*pMsg;
    int   		endian, i;

    /*
     * Accept the connection.
     */

    if ((newconn = accept (fd, (struct sockaddr *) NULL, (int *) NULL)) < 0)
    {
	return (NULL);
    }


    /*
     * Create an ICE object for this connection.
     */

    if ((iceConn = (IceConn) malloc (sizeof (struct _IceConn))) == NULL)
    {
	close (newconn);
	return (NULL);
    }

    iceConn->waiting_for_byteorder = True;
    iceConn->connection_status = IceConnectPending;
    iceConn->my_ice_version_index = 0;

    iceConn->fd = newconn;
    iceConn->sequence = 0;

    for (i = 0; i < _listen_count; i++)
	if (_listen_descrips[i] == fd)
	    break;

    if (i == _listen_count)
    {
	/* This should never happen, but just in case */

	close (newconn);
	free ((char *) iceConn);
	return (NULL);
    }
    else
    {
	iceConn->connection_string = (char *) malloc (
	    strlen (_listen_network_ids[i]) + 1);

	strcpy (iceConn->connection_string, _listen_network_ids[i]);
    }

    iceConn->vendor = NULL;
    iceConn->release = NULL;

    if ((iceConn->inbuf = iceConn->inbufptr =
	(char *) malloc (ICE_INBUFSIZE)) != NULL)
    {
	iceConn->inbufmax = iceConn->inbuf + ICE_INBUFSIZE;
    }
    else
    {
	close (newconn);
	free ((char *) iceConn);
	return (NULL);
    }

    if ((iceConn->outbuf = iceConn->outbufptr =
	(char *) malloc (ICE_OUTBUFSIZE)) != NULL)
    {
	iceConn->outbufmax = iceConn->outbuf + ICE_OUTBUFSIZE;
    }
    else
    {
	close (newconn);
	free (iceConn->inbuf);
	free ((char *) iceConn);
	return (NULL);
    }

    iceConn->scratch = NULL;
    iceConn->scratch_size = 0;

    if (fd == _IceUnixDomainConnection)
	iceConn->iceConn_type = ICE_CONN_FROM_LOCAL_ACCEPT;
    else
	iceConn->iceConn_type = ICE_CONN_FROM_ACCEPT;

    iceConn->open_ref_count = 1;
    iceConn->proto_ref_count = 0;

    iceConn->skip_want_to_close = False;
    iceConn->want_to_close = False;

    iceConn->saved_reply_waits = NULL;
    iceConn->ping_waits = NULL;

    iceConn->process_msg_info = NULL;

    iceConn->connect_to_you = NULL;
    iceConn->protosetup_to_you = NULL;

    iceConn->connect_to_me = NULL;
    iceConn->protosetup_to_me = NULL;


    /*
     * Send our byte order.
     */

    IceGetHeader (iceConn, 0, ICE_ByteOrder,
	SIZEOF (iceByteOrderMsg), iceByteOrderMsg, pMsg);

    endian = 1;
    if (*(char *) &endian)
	pMsg->byteOrder = IceLittleEndian;
    else
	pMsg->byteOrder = IceBigEndian;

    IceFlush (iceConn);


    if (_IceWatchProcs)
    {
	/*
	 * Notify the watch procedures that an iceConn was opened.
	 */

	_IceConnectionOpened (iceConn);
    }

    return (iceConn);
}



/* ------------------------------------------------------------------------- *
 *                            local routines                                 *
 * ------------------------------------------------------------------------- */

/*
 * Open a tcp socket for listening
 */

#ifdef TCPCONN

static int
open_tcp_socket (networkIdRet)

char **networkIdRet;

{
    struct sockaddr_in 	insock;
    int			fd;
    int			addrlen;
    int 		retry = 20;
    char		hostnamebuf[256];
    char		portnumbuf[10];
#ifdef WIN32
    static WSADATA wsadata;
#endif

#ifdef WIN32
    if (!wsadata.wVersion && WSAStartup(MAKEWORD(1,1), &wsadata))
	return -1;
#endif

    *networkIdRet = NULL;

    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) 
	return (-1);

#ifdef SO_REUSEADDR
    {
	int one = 1;
	setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (int));
    }
#endif

    (void) bzero ((char *) &insock, sizeof (insock));

#ifdef BSD44SOCKETS
    insock.sin_len = sizeof (insock);
#endif

    insock.sin_family = AF_INET;
    insock.sin_port = htons (0);
    insock.sin_addr.s_addr = htonl (INADDR_ANY);

    while (bind (fd, (struct sockaddr *) &insock, sizeof (insock)))
    {
	if (--retry == 0)
	{
	    close (fd);
	    return (-1);
	}

#ifdef SO_REUSEADDR
	sleep (1);
#else
	sleep (10);
#endif
    }

#ifdef SO_DONTLINGER
    setsockopt (fd, SOL_SOCKET, SO_DONTLINGER, (char *) NULL, 0);
#else
#ifdef SO_LINGER
    {
    static int linger[2] = { 0, 0 };
    setsockopt (fd, SOL_SOCKET, SO_LINGER, (char *) linger, sizeof (linger));
    }
#endif
#endif

    if (listen (fd, 5))
    {
	close (fd);
	return (-1);
    }

    if (gethostname (hostnamebuf, sizeof (hostnamebuf)))
    {
	close (fd);
	return (-1);
    }

    addrlen = sizeof (insock);
    if (getsockname (fd, (struct sockaddr *) &insock, &addrlen))
    {
	close (fd);
	return (-1);
    }

    sprintf (portnumbuf, "%d", ntohs (insock.sin_port));
    *networkIdRet = (char *) malloc (
	6 + strlen (hostnamebuf) + strlen (portnumbuf));
    sprintf (*networkIdRet, "tcp/%s:%s", hostnamebuf, portnumbuf);

    return (fd);
}

#endif /* TCPCONN */



/*
 * Open a DECnet socket for listening
 */

#ifdef DNETCONN

static int
open_dnet_socket (networkIdRet)

char **networkIdRet;

{
    struct sockaddr_dn 	dnsock;
    int			fd;
    char		hostnamebuf[256];

    *networkIdRet = NULL;

    if ((fd = socket (AF_DECnet, SOCK_STREAM, 0)) < 0) 
	return (-1);

    bzero ((char *) &dnsock, sizeof (dnsock));
    dnsock.sdn_family = AF_DECnet;
    sprintf (dnsock.sdn_objname, "sm$%d", getpid ());
    dnsock.sdn_objnamel = strlen (dnsock.sdn_objname);

    if (bind (fd, (struct sockaddr *) &dnsock, sizeof (dnsock)))
    {
	close (fd);
	return (-1);
    }

    if (listen (fd, 5))
    {
	close (fd);
	return (-1);
    }

    if (gethostname (hostnamebuf, sizeof (hostnamebuf)))
    {
	close (fd);
	return (-1);
    }

    *networkIdRet = (char *) malloc (
	13 + strlen (hostnamebuf) +	dnsock.sdn_objnamel);
    sprintf (*networkIdRet, "decnet/%s::%s", hostnamebuf, dnsock.sdn_objname);

    return (fd);
}

#endif /* DNETCONN */



/*
 * Open a unix domain socket for listening
 */

#ifdef UNIXCONN

static int
open_unix_socket (networkIdRet)

char **networkIdRet;

{
    struct sockaddr_un	unsock;
    int 		fd;
    char		hostnamebuf[256];
    char		pidbuf[10];
    int 		oldUmask;

    bzero ((char *) &unsock, sizeof (unsock));

    unsock.sun_family = AF_UNIX;
    oldUmask = umask (0);

#ifdef ICE_UNIX_DIR
    if (!mkdir (ICE_UNIX_DIR, 0777))
	chmod (ICE_UNIX_DIR, 0777);
#endif

    strcpy (unsock.sun_path, ICE_UNIX_DIR);
    sprintf (pidbuf, "/%d", getpid ());
    strcat (unsock.sun_path, pidbuf);

#ifdef BSD44SOCKETS
    unsock.sun_len = strlen (unsock.sun_path);
#endif

    unlink (unsock.sun_path);

    if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) 
	return (-1);

#ifdef BSD44SOCKETS
    if (bind(fd, (struct sockaddr *) &unsock, SUN_LEN (&unsock)))
#else
    if (bind(fd, (struct sockaddr *) &unsock, strlen (unsock.sun_path) + 2))
#endif
    {
	close (fd);
	return (-1);
    }

    if (listen (fd, 5))
    {
	close (fd);
	return (-1);
    }

    if (gethostname (hostnamebuf, sizeof (hostnamebuf)))
    {
	close (fd);
	return (-1);
    }

    (void) umask (oldUmask);

    *networkIdRet = (char *) malloc (8 + strlen (hostnamebuf) +
	strlen (unsock.sun_path));
    sprintf (*networkIdRet, "local/%s:%s", hostnamebuf, unsock.sun_path);

    return (fd);
}

#endif /* UNIXCONN */
