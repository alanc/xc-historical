/* $XConsortium: Xtransutil.c,v 1.12 94/02/17 15:25:23 mor Exp $ */

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

/*
 * These are some utility functions created for convenience or to provide
 * an interface that is similar to an existing interface. These are built
 * only using the Transport Independant API, and have no knowledge of
 * the internal implementation.
 */

#ifdef X11_t

/*
 * These values come from X.h and Xauth.h, and MUST match them. Some
 * of these values are also defined by the ChangeHost protocol message.
 */

#define FamilyInternet		0
#define FamilyDECnet		1
#define FamilyChaos		2
#define FamilyLocalHost		252
#define FamilyKrb5Principal	253
#define FamilyNetname		254
#define FamilyLocal		256
#define FamilyWild		65535

/*
 * TRANS(ConvertAddress) converts a sockaddr based address to an
 * X authorization based address. Some of this is defined as part of
 * the ChangeHost protocol. The rest is just doen in a consistent manner.
 */

int
TRANS(ConvertAddress)(familyp,addrlenp,addrp)
int	*familyp;
int	*addrlenp;
Xtransaddr	*addrp;
{

    PRMSG(2,"TRANS(ConvertAddress)(%d,%d,%x)\n",*familyp,*addrlenp,addrp);

    switch( *familyp )
    {
#if defined(TCPCONN) || defined(STREAMSCONN)
    case AF_INET:
    {
	/*
	 * Check for the BSD hack localhost address 127.0.0.1.
	 * In this case, we are really FamilyLocal.
	 */

	struct sockaddr_in saddr;
#ifdef CRAY
#ifdef OLDTCP
	int len = sizeof(saddr.sin_addr);
#else
	int len = SIZEOF_in_addr;
#endif /* OLDTCP */
	char *cp = (char *) &saddr.sin_addr;
#else /* else not CRAY */
	int len = sizeof(saddr.sin_addr.s_addr);
	char *cp = (char *) &saddr.sin_addr.s_addr;
#endif /* CRAY */

	memcpy (&saddr, addrp, sizeof (struct sockaddr_in));

	if ((len == 4) && (cp[0] == 127) && (cp[1] == 0) &&
	    (cp[2] == 0) && (cp[3] == 1))
	{
	    *familyp=FamilyLocal;
	}
	else
	{
	    *familyp=FamilyInternet;
	    *addrlenp=len;
	    memcpy(addrp,&saddr.sin_addr,len);
	}
	break;
    }
#endif /* defined(TCPCONN) || defined(STREAMSCONN) */

#if defined(DNETCONN)
    case AF_DECnet:
    {
	struct sockaddr_dn saddr;

	memcpy (&saddr, addrp, sizeof (struct sockaddr_dn));

	*familyp=FamilyDECnet;
	*addrlenp=sizeof(struct dn_naddr);
	memcpy(addrp,&saddr.sdn_add,*addrlenp);

	break;
    }
#endif /* defined(DNETCONN) */

#if defined(UNIXCONN) || defined(LOCALCONN)
    case AF_UNIX:
    {
	*familyp=FamilyLocal;
	break;
    }
#endif /* defined(UNIXCONN) || defined(LOCALCONN) */

    default:
	PRMSG(1,"TRANS(ConvertFamily) Unknown family type %d\n",
	      *familyp, 0,0 );
	return -1;
    }


    if (*familyp == FamilyLocal)
    {
	/*
	 * In the case of a local connection, we need to get the
	 * host name for authentication.
	 */
	
	char hostnamebuf[256];
	int len = TRANS(GetHostname) (hostnamebuf, sizeof hostnamebuf);

	if (len > 0) {
	    if (addrp && *addrlenp < (len + 1))
	    {
		free ((char *) addrp);
		addrp = NULL;
	    }
	    if (!addrp)
		addrp = (Xtransaddr *) malloc (len + 1);
	    if (addrp) {
		strcpy ((char *) addrp, hostnamebuf);
		*addrlenp = len;
	    } else {
		*addrlenp = 0;
	    }
	}
	else
	{
	    if (addrp)
		free ((char *) addrp);
	    addrp = NULL;
	    *addrlenp = 0;
	}
    }

    return 0;
}

#endif /* X11_t */

#ifdef ICE_t

#include <signal.h>

char *
TRANS(GetMyNetworkId) (family, addrlen, addr)

int		family;
int		addrlen;
Xtransaddr	*addr;

{
    char	hostnamebuf[256];
    char 	*networkId = NULL;


    if (gethostname (hostnamebuf, sizeof (hostnamebuf)) < 0)
    {
	return (NULL);
    }

    switch (family)
    {
#if defined(UNIXCONN) || defined(LOCALCONN)
    case AF_UNIX:
    {
	struct sockaddr_un *saddr = (struct sockaddr_un *) addr;
	networkId = (char *) malloc (
	    8 + strlen (hostnamebuf) + strlen (saddr->sun_path));
	sprintf (networkId, "local/%s:%s", hostnamebuf, saddr->sun_path);
	break;
    }
#endif /* defined(UNIXCONN) || defined(LOCALCONN) */

#if defined(TCPCONN) || defined(STREAMSCONN)
    case AF_INET:
    {
	struct sockaddr_in *saddr = (struct sockaddr_in *) addr;
	char portnumbuf[10];

	sprintf (portnumbuf, "%d", ntohs (saddr->sin_port));
	networkId = (char *) malloc (
	    6 + strlen (hostnamebuf) + strlen (portnumbuf));
	sprintf (networkId, "tcp/%s:%s", hostnamebuf, portnumbuf);
	break;
    }
#endif /* defined(TCPCONN) || defined(STREAMSCONN) */

#if defined(DNETCONN)
    case AF_DECnet:
    {
	struct sockaddr_dn *saddr = (struct sockaddr_dn *) addr;

	networkId = (char *) malloc (
	    13 + strlen (hostnamebuf) + saddr->sdn_objnamel);
	sprintf (networkId, "decnet/%s::%s",
	    hostnamebuf, saddr->sdn_objname);
	break;
    }
#endif /* defined(DNETCONN) */

    default:
	break;
    }

    return (networkId);
}

#include <setjmp.h>
static jmp_buf env;

#ifdef SIGALRM
static int nameserver_timedout = 0;

static 
#ifdef SIGNALRETURNSINT
int
#else
void
#endif
nameserver_lost(sig)
{
  nameserver_timedout = 1;
  longjmp (env, -1);
  /* NOTREACHED */
#ifdef SIGNALRETURNSINT
  return -1;				/* for picky compilers */
#endif
}
#endif /* SIGALARM */


char *
TRANS(GetPeerNetworkId) (family, peer_addrlen, peer_addr)

int		family;
int		peer_addrlen;
Xtransaddr	*peer_addr;

{
    char	*hostname;
    char	*networkId = NULL;
    char	addrbuf[256], prefix[10];
    char	*addr = NULL;

    switch (family)
    {
    case AF_UNSPEC:
#if defined(UNIXCONN) || defined(LOCALCONN)
    case AF_UNIX:
    {
	strcpy (prefix, "local/");
	if (gethostname (addrbuf, sizeof (addrbuf)) == 0)
	    addr = addrbuf;
	break;
    }
#endif /* defined(UNIXCONN) || defined(LOCALCONN) */

#if defined(TCPCONN) || defined(STREAMSCONN)
    case AF_INET:
    {
	struct sockaddr_in *saddr = (struct sockaddr_in *) peer_addr;
	struct hostent *hp = NULL;
#ifndef WIN32
 	char *inet_ntoa();
#endif

	strcpy (prefix, "tcp/");

#ifdef SIGALRM
	/*
	 * gethostbyaddr can take a LONG time if the host does not exist.
	 * Assume that if it does not respond in NAMESERVER_TIMEOUT seconds
	 * that something is wrong and do not make the user wait.
	 * gethostbyaddr will continue after a signal, so we have to
	 * jump out of it. 
	 */

	nameserver_timedout = 0;
	signal (SIGALRM, nameserver_lost);
	alarm (4);
	if (setjmp(env) == 0) {
#endif
	    hp = gethostbyaddr ((char *) &saddr->sin_addr,
		sizeof (saddr->sin_addr), AF_INET);
#ifdef SIGALRM
	}
	alarm (0);
#endif
	if (hp)
	  addr = hp->h_name;
	else
	  addr = inet_ntoa (saddr->sin_addr);
	break;
    }

#endif /* defined(TCPCONN) || defined(STREAMSCONN) */

#if defined(DNETCONN)
    case AF_DECnet:
    {
	struct sockaddr_dn *saddr = (struct sockaddr_dn *) peer_addr;
	struct nodeent *np;

	strcpy (prefix, "decnet/");

	if (np = getnodebyaddr(saddr->sdn_add.a_addr,
	    saddr->sdn_add.a_len, AF_DECnet)) {
	    sprintf(addrbuf, "%s:", np->n_name);
	} else {
	    sprintf(addrbuf, "%s:", dnet_htoa(&saddr->sdn_add));
	}
	addr = addrbuf;
	break;
    }
#endif /* defined(DNETCONN) */

    default:
	return (NULL);
    }


    hostname = (char *) malloc (strlen (prefix) + strlen (addr) + 1);
    strcpy (hostname, prefix);
    if (addr)
	strcat (hostname, addr);

    return (hostname);
}

#endif /* ICE_t */


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
