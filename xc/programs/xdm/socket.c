/*
 * xdm - display manager daemon
 *
 * $XConsortium: socket.c,v 1.26 91/05/06 23:53:43 gildea Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * socket.c - Support for BSD sockets
 */

#include "dm.h"

#ifdef XDMCP

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>

extern int	socketFd;
extern int	chooserFd;

extern FD_TYPE	WellKnownSocketsMask;
extern int	WellKnownSocketsMax;

CreateWellKnownSockets ()
{
    struct sockaddr_in	sock_addr;
    char		*name, *localHostname();

    if (request_port == 0)
	    return;
    Debug ("creating socket %d\n", request_port);
    socketFd = socket (AF_INET, SOCK_DGRAM, 0);
    if (socketFd == -1) {
	LogError ("socket creation failed\n");
	return;
    }
    name = localHostname ();
    registerHostname (name, strlen (name));
    RegisterCloseOnFork (socketFd);
    /* zero out the entire structure; this avoids 4.4 incompatibilities */
    bzero ((char *) &sock_addr, sizeof (sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons ((short) request_port);
    sock_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (socketFd, &sock_addr, sizeof (sock_addr)) == -1)
    {
	LogError ("error binding socket address %d\n", request_port);
	close (socketFd);
	socketFd = -1;
	return;
    }
    WellKnownSocketsMax = socketFd;
    FD_SET (socketFd, &WellKnownSocketsMask);

    chooserFd = socket (AF_INET, SOCK_STREAM, 0);
    Debug ("Created chooser socket %d\n", chooserFd);
    if (chooserFd == -1)
    {
	LogError ("chooser socket creation failed\n");
	return;
    }
    listen (chooserFd, 5);
    if (chooserFd > WellKnownSocketsMax)
	WellKnownSocketsMax = chooserFd;
    FD_SET (chooserFd, &WellKnownSocketsMask);
}

GetChooserAddr (addr, lenp)
    struct sockaddr *addr;
    int		    *lenp;
{
    return getsockname (chooserFd, addr, lenp);
}

#endif /* XDMCP */
