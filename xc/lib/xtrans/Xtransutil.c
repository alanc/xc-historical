/*
 * These are some utility functions created for convenience or to provide
 * an interface that is similar to an existing interface. These are built
 * only using the Transport Independant API, and have no knowledge of
 * the internal implementation.
 */

#ifdef X11

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
#if defined(TCPCONN)
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
	len = SIZEOF_in_addr;
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
#if defined(CRAY) && defined(OLDTCP)
	    memcpy(addrp,&saddr.sin_addr,len);
#else
	    memcpy(addrp,&saddr.sin_addr.s_addr,len);
#endif
	}
	break;
    }
#endif /* TCPCONN */
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
#endif /* DNETCONN */
#if defined(UNIXCONN)
    case AF_UNIX:
    {
	*familyp=FamilyLocal;
	break;
    }
#endif /* UNIXCONN */
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

#endif /* X11 */

#ifdef ICE

char *
TRANS(GetMyNetworkId) (family, addrlen, addr)

int		family;
int		addrlen;
Xtransaddr	*addr;

{
    char	hostnamebuf[256];
    char 	*networkId = NULL;


    if (TRANS(GetHostname) (hostnamebuf, sizeof (hostnamebuf)) < 0)
    {
	return (NULL);
    }

    switch (family)
    {
#ifdef UNIXCONN
    case AF_UNIX:
    {
	struct sockaddr_un *saddr = (struct sockaddr_un *) addr;
	networkId = (char *) malloc (
	    8 + strlen (hostnamebuf) + strlen (saddr->sun_path));
	sprintf (networkId, "local/%s:%s", hostnamebuf, saddr->sun_path);
	break;
    }
#endif
#ifdef TCPCONN
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
#endif
#ifdef DNETCONN
    case AF_DECnet:
    {
	struct sockaddr_dn *saddr = (struct sockaddr_dn *) addr;

	networkId = (char *) malloc (
	    13 + strlen (hostnamebuf) + saddr->sdn_objnamel);
	sprintf (networkId, "decnet/%s::%s",
	    hostnamebuf, saddr->sdn_objname);
	break;
    }
#endif
    default:
	break;
    }

    return (networkId);
}


#include <arpa/nameser.h>
#include <resolv.h>

static ns_inaddrtohostname ();

char *
TRANS(GetPeerNetworkId) (family, peer_addrlen, peer_addr)

int		family;
int		peer_addrlen;
Xtransaddr	*peer_addr;

{
    char	*hostname;
    char	*networkId = NULL;
    char	addr[256], prefix[10];

    addr[0] = '\0';

    switch (family)
    {
    case AF_UNSPEC:
#ifdef UNIXCONN
    case AF_UNIX:
#endif
    {
	char hostnamebuf[256];

	strcpy (prefix, "local/");
	if (TRANS(GetHostname) (hostnamebuf, sizeof (hostnamebuf)) == 0)
	    strcpy (addr, hostnamebuf);
	break;
    }
#ifdef TCPCONN
    case AF_INET:
    {
	struct sockaddr_in *saddr = (struct sockaddr_in *) peer_addr;
	strcpy (prefix, "tcp/");
	ns_inaddrtohostname (&saddr->sin_addr, addr, sizeof (addr));
	break;
    }
#endif
#ifdef DNETCONN
    case AF_DECnet:
    {
	struct sockaddr_dn *saddr = (struct sockaddr_dn *) peer_addr;
	strcpy (prefix, "decnet/");
	sprintf (addr, "%s", dnet_ntoa (&saddr->sdn_add));
	break;
    }
#endif
    default:
	return (NULL);
    }


    hostname = (char *) malloc (strlen (prefix) + strlen (addr) + 1);
    strcpy (hostname, prefix);
    strcat (hostname, addr);

    return (hostname);
}

/*
 * Get a host name from an Internet address.
 * Thanks to Keith Packard for this code.
 */

#define DATA_LEN 4096

struct ns_answer {
	u_short	type;
	u_short	class;
	u_short	len;
	u_char	data[DATA_LEN];
};

static
hostorderheader (h)
	HEADER	*h;
{
	h->id = ntohs(h->id);
	h->qdcount = ntohs(h->qdcount);
	h->ancount = ntohs(h->ancount);
	h->nscount = ntohs(h->nscount);
	h->arcount = ntohs(h->arcount);
}

static
do_query (type, class, string, answers, max)
	int			type, class;
	char			*string;
	struct ns_answer	*answers;
	int			max;
{
	int		i;
	int		msglen, anslen;
	u_char		msg[PACKETSZ];
	u_char		ans[PACKETSZ];
	HEADER		*h;
	u_char		*a, *eom;
	int		len;

	msglen = res_mkquery (QUERY, string, class, type,
			   (char *) 0, 0, (char *) 0,
			   msg, sizeof msg);
	anslen = res_send(msg, msglen, ans, sizeof ans);
	if (anslen < sizeof (HEADER))
		return 0;
	h = (HEADER *) ans;
	hostorderheader(h);
	a = ans + sizeof (HEADER);
	eom = ans + anslen;
	/* skip query stuff */
	if (h->qdcount)
	{
		for (i = 0; i < h->qdcount; i++)
		{
			len = dn_skipname(a, eom);
			if (len < 0)
				return 0;
			a += len;
			a += 2;	/* type */
			a += 2;	/* class */
		}
	}
	for (i = 0; i < h->ancount && i < max; i++)
	{
		len = dn_skipname (a, eom);
		if (len < 0)
			return 0;
		a += len;
		GETSHORT (answers[i].type, a);	/* type */
		GETSHORT (answers[i].class, a);	/* class */
		a += 4;				/* ttl */
		GETSHORT (len, a);		/* dlen */
		if (len > DATA_LEN)
			len = DATA_LEN;
		bcopy (a, answers[i].data, len);
		answers[i].len = len;
	}
	return i;
}

static
ns_inaddrtohostname (addr,name,max)
	struct in_addr	*addr;
	char		*name;
	int		max;
{
	struct ns_answer	answer;
	char			addrstring[256];
	u_char			*a;
	int			ret;
	int			l;

	a = (u_char *) addr;
	sprintf (addrstring, "%d.%d.%d.%d.IN-ADDR.ARPA",
	    a[3], a[2], a[1], a[0]);
	ret = do_query (T_PTR, C_IN, addrstring, &answer, 1);
	if (ret <= 0)
		return 0;
	if (answer.type != T_PTR || answer.class != C_IN ||
	    answer.len > max)
		return 0;
	a = (u_char *) answer.data;
	while (l = *a++) {
		while (l--)
			*name++ = (char) *a++;
		if (*a)
			*name++ = '.';
	}
	*name = '\0';
	return 1;
}

#endif /* ICE */
