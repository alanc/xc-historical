/* $XConsortium: accept.c,v 1.13 93/11/22 16:29:18 mor Exp $ */
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

#define NUM_NETWORK_INTERFACES 3  	/* Unix Domain, TCP, and DECnet */

#ifdef UNIXCONN
static int open_unix_socket ();
#endif
#ifdef TCPCONN
static int open_tcp_socket ();
#endif
#ifdef DNETCONN
static int open_dnet_socket ();
#endif



Status
IceListenForConnections (countRet, listenObjsRet, errorLength, errorStringRet)

int		*countRet;
IceListenObj	**listenObjsRet;
int		errorLength;
char		*errorStringRet;

{
    struct _IceListenObj	listenObjs[NUM_NETWORK_INTERFACES];
    char			*networkId;
    int				fd, i, j;

    /*
     * For each network interface available, open a socket for
     * listening to new connections.
     */

    *countRet = 0;

#ifdef UNIXCONN
    if ((fd = open_unix_socket (&networkId)) != -1)
    {
	listenObjs[*countRet].fd = fd;
	listenObjs[*countRet].network_id = networkId;
	listenObjs[*countRet].unix_domain = True;

	(*countRet)++;
    }
#endif
#ifdef TCPCONN
    if ((fd = open_tcp_socket (&networkId)) != -1)
    {
	listenObjs[*countRet].fd = fd;
	listenObjs[*countRet].network_id = networkId;
	listenObjs[*countRet].unix_domain = False;

	(*countRet)++;
    }
#endif
#ifdef DNETCONN
    if ((fd = open_dnet_socket (&networkId)) != -1)
    {
	listenObjs[*countRet].fd = fd;
	listenObjs[*countRet].network_id = networkId;
	listenObjs[*countRet].unix_domain = False;

	(*countRet)++;
    }
#endif

    if (*countRet == 0)
    {
	*listenObjsRet = NULL;

        strncpy (errorStringRet,
	    "Cannot establish any listening sockets", errorLength);

	return (0);
    }
    else
    {
	*listenObjsRet = (IceListenObj *) malloc (
	    *countRet * sizeof (IceListenObj));

	if (*listenObjsRet == NULL)
	{
	    strncpy (errorStringRet, "Malloc failed", errorLength);

	    return (0);
	}
	else
	{
	    for (i = 0; i < *countRet; i++)
	    {
		(*listenObjsRet)[i] = (IceListenObj) malloc (
		    sizeof (struct _IceListenObj));

		if ((*listenObjsRet)[i] == NULL)
		{
		    strncpy (errorStringRet, "Malloc failed", errorLength);

		    for (j = 0; j < i; j++)
			free ((char *) (*listenObjsRet)[j]);

		    free ((char *) *listenObjsRet);

		    return (0);
		}
		else
		{
		    *((*listenObjsRet)[i]) = listenObjs[i];
		}
	    }

	    if (errorStringRet && errorLength > 0)
		*errorStringRet = '\0';

	    for (i = 0; i < *countRet; i++)
	    {
		(*listenObjsRet)[i]->auth_data_entry_count = 0;
		(*listenObjsRet)[i]->auth_data_entries = NULL;
		(*listenObjsRet)[i]->host_based_auth_proc = NULL;
		(*listenObjsRet)[i]->host_based_auth_proc_client_data = NULL;
	    }

	    return (1);
	}
    }
}



int
IceGetListenDescrip (listenObj)

IceListenObj listenObj;

{
    return (listenObj->fd);
}



char *
IceGetListenNetworkId (listenObj)

IceListenObj listenObj;

{
    char *networkId;

    networkId = (char *) malloc (strlen (listenObj->network_id) + 1);

    if (networkId)
	strcpy (networkId, listenObj->network_id);

    return (networkId);
}



char *
IceComposeNetworkIdList (count, listenObjs)

int		count;
IceListenObj	*listenObjs;

{
    char *list;
    int len = 0;
    int i;

    if (count < 1 || listenObjs == NULL)
	return (NULL);

    for (i = 0; i < count; i++)
	len += (strlen (listenObjs[i]->network_id) + 1);

    list = (char *) malloc (len);

    if (list == NULL)
	return (NULL);
    else
    {
	list[0] = '\0';
	for (i = 0; i < count - 1; i++)
	{
	    strcat (list, listenObjs[i]->network_id);
	    strcat (list, ",");
	}

	strcat (list, listenObjs[count - 1]->network_id);
	list[strlen (list)] = '\0';

	return (list);
    }
}



/* ------------------------------------------------------------------------- *
 *                            local routines                                 *
 * ------------------------------------------------------------------------- */

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
