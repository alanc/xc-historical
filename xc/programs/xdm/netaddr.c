/*
 * xdm - X display manager
 *
 * $XConsortium$
 *
 * Copyright 1991 Massachusetts Institute of Technology
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
 */

/*
 * netaddr.c - Interpretation of XdmcpNetaddr object.
 */

#include "dm.h"

#include <X11/X.h>		/* FamilyInternet, etc. */

#ifdef XDMCP

#ifndef STREAMSCONN
#include <sys/socket.h>		/* struct sockaddr */
#endif

#ifdef UNIXCONN
#include <sys/un.h>		/* struct sockaddr_un */
#endif
#ifdef TCPCONN
#include <netinet/in.h>		/* struct sockaddr_in */
#endif
#ifdef DNETCONN
#include <netdnet/dn.h>		/* struct sockaddr_dn */
#endif

/* given an XdmcpNetaddr, returns the socket protocol family used,
   e.g., AF_INET */

int NetaddrFamily(netaddrp)
    XdmcpNetaddr *netaddrp;
{
#ifdef STREAMSCONN
    short family = *netaddrp;
    return family;
#else
    return ((struct sockaddr *)netaddrp)->sa_family;
#endif
}


/* given an XdmcpNetaddr, returns a pointer to the TCP/UDP port used
   and sets *lenp to the length of the address
   or 0 if not using TCP or UDP */

char * NetaddrPort(netaddrp, lenp)
    XdmcpNetaddr *netaddrp;
    int *lenp;			/* return */
{
#ifdef STREAMSCONN
    *lenp = 2;
    return netaddrp+2;
#else
    switch (NetaddrFamily(netaddrp))
    {
    case AF_INET:
	*lenp = 2;
	return (char *)&(((struct sockaddr_in *)netaddrp)->sin_port);
    default:
	*lenp = 0;
	return NULL;
    }
#endif
}


/* given an XdmcpNetaddr, returns a pointer to the network address
   and sets *lenp to the length of the address */

char * NetaddrAddress(netaddrp, lenp)
    XdmcpNetaddr *netaddrp;
    int *lenp;			/* return */
{
#ifdef STREAMSCONN
    return netaddrp+4;
#else
#ifdef UNIXCONN
    switch (NetaddrFamily(netaddrp)) {
    case AF_UNIX:
	*lenp = strlen(((struct sockaddr_un *)netaddrp)->sun_path);
        return (char *) (((struct sockaddr_un *)netaddrp)->sun_path);
#endif
#ifdef TCPCONN
    case AF_INET:
        *lenp = sizeof (struct in_addr);
        return (char *) &(((struct sockaddr_in *)netaddrp)->sin_addr);
#endif
#ifdef DNETCONN
    case AF_DECnet:
        *lenp = sizeof (struct dn_naddr);
        return (char *) &(((struct sockaddr_dn *)netaddrp)->sdn_add);
#endif
#ifdef AF_CHAOS
    case AF_CHAOS:
#endif
    default:
	*lenp = 0;
	return NULL;
    }
#endif /* STREAMSCONN else */
}


/* given an XdmcpNetaddr, sets *addr to the network address used and
   sets *len to the number of bytes in addr.
   Returns the X protocol family used, e.g., FamilyInternet */

int ConvertAddr (saddr, len, addr)
    XdmcpNetaddr *saddr;
    int *len;			/* return */
    char **addr;		/* return */
{
    int retval;

    if (len == NULL)
        return -1;
    *addr = NetaddrAddress(saddr, len);
    switch (NetaddrFamily(saddr))
    {
      case AF_UNSPEC:
#ifndef hpux
      case AF_UNIX:
#endif
        retval = FamilyLocal;
	break;
#ifdef TCPCONN
      case AF_INET:
        retval = FamilyInternet;
	break;
#endif
#ifdef DNETCONN
      case AF_DECnet:
        retval = FamilyDECnet;
	break;
#endif
#ifdef AF_CHAOS
    case AF_CHAOS:
	retval = FamilyChaos;
	break;
#endif
      default:
	retval = -1;
        break;
    }
    Debug ("ConvertAddr returning %d\n", retval);
    return retval;
}

#endif /* XDMCP */
