#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#ifdef DNETCONN
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif
#include "xauth.h"
#include <X11/X.h>

Bool nameserver_timedout = False;


/*
 * get_hostname - Given an internet address, return a name (CHARON.MIT.EDU)
 * or a string representing the address (18.58.0.13) if the name cannot
 * be found.  Stolen from xhost.
 */

static jmp_buf env;
static nameserver_lost()
{
  nameserver_timedout = True;
  longjmp (env, -1);
}


char *get_hostname (auth)
    Xauth *auth;
{
    struct hostent *hp = NULL;
    int nameserver_lost();
    char *inet_ntoa();
#ifdef DNETCONN
    struct nodeent *np;
    static char nodeaddr[16];
#endif /* DNETCONN */

    if (auth->family == FamilyInternet) {
	/* gethostbyaddr can take a LONG time if the host does not exist.
	   Assume that if it does not respond in NAMESERVER_TIMEOUT seconds
	   that something is wrong and do not make the user wait.
	   gethostbyaddr will continue after a signal, so we have to
	   jump out of it. 
	   */
	nameserver_timedout = False;
	signal (SIGALRM, nameserver_lost);
	alarm (4);
	if (setjmp(env) == 0) {
	    hp = gethostbyaddr (auth->address, auth->address_length, AF_INET);
	}
	alarm (0);
	if (hp)
	  return (hp->h_name);
	else
	  return (inet_ntoa(*((struct in_addr *)(auth->address))));
    }
#ifdef DNETCONN
    if (auth->family == FamilyDECnet) {
	if (np = getnodebyaddr(auth->address, auth->address_length,
			       AF_DECnet)) {
	    sprintf(nodeaddr, "%s:", np->n_name);
	} else {
	    sprintf(nodeaddr, "%s:", dnet_htoa(auth->address));
	}
	return(nodeaddr);
    }
#endif

    return (NULL);
}

/*
 * cribbed from lib/X/XConnDis.c
 */
static Bool get_inet_address (name, resultp)
    char *name;
    unsigned long *resultp;		/* return */
{
    unsigned long hostinetaddr = inet_addr (name);
    struct hostent *host_ptr;
    struct sockaddr_in inaddr;		/* dummy variable for size calcs */

    if (hostinetaddr == -1) {		/* oh, gross.... */
	if ((host_ptr = gethostbyname (name)) == NULL) {
	    /* No such host! */
	    errno = EINVAL;
	    return False;
	}
	/* Check the address type for an internet host. */
	if (host_ptr->h_addrtype != AF_INET) {
	    /* Not an Internet host! */
	    errno = EPROTOTYPE;
	    return False;
	}
 
	bcopy((char *)host_ptr->h_addr, (char *)&hostinetaddr,
	      sizeof(inaddr.sin_addr));
    }
    *resultp = hostinetaddr;
    return True;
}

#ifdef DNETCONN
static Bool get_dnet_address (name, resultp)
    char *name;
    struct dn_anaddr *resultp;
{
    struct dn_naddr *dnaddrp, dnaddr;
    struct nodeent *np;

    if (dnaddrp = dnet_addr (name)) {	/* stolen from xhost */
	dnaddr = *dnaddrp;
    } else {
	if ((np = getnodebyname (name)) == NULL) return False;
	dnaddr.a_len = np->n_length;
	bcopy (np->n_addr, dnaddr.a_addr, np->n_length);
    }
    *resultp = dnaddr;
    return True;
}
#endif

char *get_address_info (family, fulldpyname, prefix, host, lenp)
    int family;
    char *fulldpyname;
    int prefix;
    char *host;
    int *lenp;
{
    char *retval = NULL;
    int len;
    char *src;
    unsigned long hostinetaddr;
    struct sockaddr_in inaddr;		/* dummy variable for size calcs */
#ifdef DNETCONN
    struct dn_naddr dnaddr;
#endif

    /*
     * based on the family, set the pointer src to the start of the address
     * information to be copied and set len to the number of bytes.
     */
    switch (family) {
      case FamilyLocal:			/* hostname/unix:0 */
	src = fulldpyname;
	len = prefix;
	break;
      case FamilyInternet:		/* host:0 */
	if (!get_inet_address (host, &hostinetaddr)) return NULL;
	src = (char *) &hostinetaddr;
	len = (sizeof inaddr.sin_addr);
	break;
      case FamilyDECnet:		/* host::0 */
#ifdef DNETCONN
	if (!get_dnet_address (host, &dnaddr)) return NULL;
	src = (char *) &dnaddr;
	len = (sizeof dnaddr);
	break;
#else
	/* fall through since we don't have code for it */
#endif
      default:
	src = NULL;
	len = 0;
    }

    /*
     * if source was provided, allocate space and copy it
     */
    if (len == 0 || !src) return NULL;

    retval = malloc (len);
    if (retval) {
	bcopy (src, retval, len);
	*lenp = len;
    }
    return retval;
}
