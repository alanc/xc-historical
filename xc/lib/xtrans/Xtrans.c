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

/*
 * The transport table contains a definition for every transport (protocol)
 * family. All operations that can be made on the transport go through this
 * table.
 *
 * New transports can be added by adding an entry in this table.
 */

static
Xtransport *Xtransports[] = {
#if defined(STREAMSCONN)
    &TRANS(TLIINETFuncs),
    &TRANS(TLITCPFuncs),
    &TRANS(TLITLIFuncs),
#endif /* STREAMSCONN */
#if defined(UNIXCONN)
    &TRANS(SocketUNIXFuncs),
#if !defined(LOCALCONN)
    &TRANS(SocketLocalFuncs),
#endif /* !LOCALCONN */
#endif /* UNIXCONN */
#if defined(TCPCONN)
    &TRANS(SocketINETFuncs),
    &TRANS(SocketTCPFuncs),
#endif /* TCPCONN */
#if defined(DNETCONN)
    &TRANS(DNETFuncs),
#endif /* DNETCONN */
#if defined(LOCALCONN)
    &TRANS(LocalFuncs),
    &TRANS(PTSFuncs),
    &TRANS(NAMEDFuncs),
    &TRANS(ISCFuncs),
    &TRANS(SCOFuncs),
#endif /* LOCALCONN */
};

#define NUMTRANS	(sizeof(Xtransports)/sizeof(Xtransport *))


#ifdef WIN32
#define ioctl ioctlsocket
#endif



/*
 * These are a few utility function used by the public interface functions.
 */

void
TRANS(FreeConnInfo) (ciptr)

XtransConnInfo ciptr;

{
    PRMSG (3,"TRANS(FreeConnInfo) (%x)\n", ciptr, 0, 0);

    if (ciptr->addr)
	free (ciptr->addr);

    if (ciptr->peeraddr)
	free (ciptr->peeraddr);

    free ((char *) ciptr);
}


#define PROTOBUFSIZE	20

static Xtransport *
TRANS(SelectTransport) (protocol)

char *protocol;

{
    char 	protobuf[PROTOBUFSIZE];
    int		i;

    PRMSG (3,"TRANS(SelectTransport) (%s)\n", protocol, 0, 0);

    /*
     * Force Protocol to be lowercase as a way of doing
     * a case insensitive match.
     */

    strncpy (protobuf, protocol, PROTOBUFSIZE);

    for (i = 0; i < PROTOBUFSIZE && protobuf[i] != '\0'; i++)
	if (isupper (protobuf[i]))
	    protobuf[i] = tolower (protobuf[i]);

    /* Look at all of the configured protocols */

    for (i = 0; i < NUMTRANS; i++)
    {
	if (!strcmp (protobuf, Xtransports[i]->TransName))
	    return Xtransports[i];
    }

    return NULL;
}

#ifndef TEST_t
static
#endif /* TEST_t */
int
TRANS(ParseAddress) (address, protocol, host, port)

char	*address;
char	**protocol;
char	**host;
char	**port;

{
    /*
     * Address is a string formatted as "protocol/host:port". If the protocol
     * part is missing, then assume INET. If the protocol part and host part
     * are missing, then assume local. If a "::" is found then assume DNET.
     */

    char	*mybuf, *tmpptr;
    char	*_protocol, *_host, *_port;
    char	hostnamebuf[256];

    PRMSG (3,"TRANS(ParseAddress) (%s)\n", address, 0, 0);

    /* Copy the string so it can be changed */

    tmpptr = mybuf = (char *) malloc (strlen (address) + 1);
    strcpy (mybuf, address);

    /* Parse the string to get each component */
    
    /* Get the protocol part */

    _protocol = mybuf;

    if ((mybuf = strpbrk (mybuf,"/:")) == NULL)
    {
	/* adress is in a bad format */
	*protocol = NULL;
	*host = NULL;
	*port = NULL;
	free (tmpptr);
	return 0;
    }

    if (*mybuf == ':')
    {
	/*
	 * If there is a hostname, then assume inet, otherwise
	 * it must be local.
	 */
	if (mybuf == tmpptr)
	{
	    /* There is neither a protocol or host specified */
	    _protocol = "local";
	}
	else
	{
	    /* Ther is a hostname specified */
	    _protocol = "inet";
	    mybuf = tmpptr;	/* reset to the begining of the host ptr */
	}
    }
    else
    {
	/* *mybuf == '/' */

	*mybuf ++= '\0'; /* put a null at the end of the protocol */

	if (strlen(_protocol) == 0)
	{
	    /*
	     * If there is a hostname, then assume inet, otherwise
	     * it must be local.
	     */
	    if (*mybuf != ':')
		_protocol = "inet";
	    else
		_protocol = "local";
	}
    }

    /* Get the host part */

    _host = mybuf;

    if ((mybuf = strchr (mybuf,':')) == NULL)
    {
	*protocol = NULL;
	*host = NULL;
	*port = NULL;
	free (tmpptr);
	return 0;
    }

    *mybuf ++= '\0';

    if (strlen(_host) == 0)
    {
#if defined(UNIXCONN) || defined(LOCALCONN)
	_host = "local";
#else
	TRANS(GetHostname) (hostnamebuf, sizeof (hostnamebuf));
	_host = hostnamebuf;
#endif
    }

    /* Check for DECnet */

    if (*mybuf == ':')
    {
	_protocol = "decnet";
	mybuf++;
    }

    /* The rest is the port */

get_port:

    _port = mybuf;

    /*
     * Now that we have all of the components, allocate new
     * string space for them.
     */

    if ((*protocol = (char *) malloc(strlen (_protocol) + 1)) == NULL)
    {
	/* Malloc failed */
	*port = NULL;
	*host = NULL;
	*protocol = NULL;
	free (tmpptr);
	return 0;
    }
    else
        strcpy (*protocol, _protocol);

    if ((*host = (char *) malloc (strlen (_host) + 1)) == NULL)
    {
	/* Malloc failed */
	*port = NULL;
	*host = NULL;
	free (*protocol);
	*protocol = NULL;
	free (tmpptr);
	return 0;
	}
    else
        strcpy (*host, _host);

    if ((*port = (char *) malloc (strlen (_port) + 1)) == NULL)
    {
	/* Malloc failed */
	*port = NULL;
	free (*host);
	*host = NULL;
	free (*protocol);
	*protocol = NULL;
	free (tmpptr);
	return 0;
    }
    else
        strcpy (*port, _port);

    free (tmpptr);

    return 1;
}


/*
 * TRANS(Open) does all of the real work opening a connection. The only
 * funny part about this is the type parameter which is used to decide which
 * type of open to perform.
 */

static XtransConnInfo
TRANS(Open) (type, address)

int	type;
char	*address;

{
    char 		*protocol = NULL, *host = NULL, *port = NULL;
    XtransConnInfo	ciptr = NULL;
    Xtransport		*thistrans;

    PRMSG (2,"TRANS(Open) (%d,%s)\n", type, address, 0);

    /* Parse the Address */

    if (TRANS(ParseAddress) (address, &protocol, &host, &port) == 0)
    {
	PRMSG (1,"TRANS(Open): Unable to Parse address %s\n", address, 0, 0);
	return NULL;
    }

    /* Determine the transport type */

    if ((thistrans = TRANS(SelectTransport) (protocol)) == NULL)
    {
	PRMSG (1,"TRANS(Open): Unable to find transport for %s\n",
	       protocol, 0, 0);

	free (protocol);
	free (host);
	free (port);
	return NULL;
    }

    /* Open the transport */

    switch (type)
    {
    case XTRANS_OPEN_COTS_CLIENT:
	ciptr = thistrans->OpenCOTSClient(thistrans, protocol, host, port);
	break;
    case XTRANS_OPEN_COTS_SERVER:
	ciptr = thistrans->OpenCOTSServer(thistrans, protocol, host, port);
	break;
    case XTRANS_OPEN_CLTS_CLIENT:
	ciptr = thistrans->OpenCLTSClient(thistrans, protocol, host, port);
	break;
    case XTRANS_OPEN_CLTS_SERVER:
	ciptr = thistrans->OpenCLTSServer(thistrans, protocol, host, port);
	break;
    default:
	PRMSG (1,"TRANS(Open): Unknown Open type %d\n", type, 0, 0);
    }

    if (ciptr == NULL)
    {
	PRMSG (1,"TRANS(Open): transport open failed for %s/%s:%s\n",
	       protocol, host, port);
	free (protocol);
	free (host);
	free (port);
	return NULL;
    }

    ciptr->transptr = thistrans;

    free (protocol);
    free (host);
    free (port);

    return ciptr;
}


/*
 * These are the public interfaces to this Transport interface.
 * These are the only functions that should have knowledge of the transport
 * table.
 */

XtransConnInfo
TRANS(OpenCOTSClient) (address)

char	*address;

{
    PRMSG (2,"TRANS(OpenCOTSClient) (%s)\n", address, 0, 0);
    return TRANS(Open) (XTRANS_OPEN_COTS_CLIENT, address);
}

XtransConnInfo
TRANS(OpenCOTSServer) (address)

char	*address;

{
    PRMSG (2,"TRANS(OpenCOTSServer) (%s)\n", address, 0, 0);
    return TRANS(Open) (XTRANS_OPEN_COTS_SERVER, address);
}

XtransConnInfo
TRANS(OpenCLTSClient) (address)

char	*address;
{
    PRMSG (2,"TRANS(OpenCLTSClient) (%s)\n", address, 0, 0);
    return TRANS(Open) (XTRANS_OPEN_CLTS_CLIENT, address);
}

XtransConnInfo
TRANS(OpenCLTSServer) (address)

char	*address;

{
    PRMSG (2,"TRANS(OpenCLTSServer) (%s)\n", address, 0, 0);
    return TRANS(Open) (XTRANS_OPEN_CLTS_SERVER, address);
}

int
TRANS(SetOption) (ciptr, option, arg)

XtransConnInfo	ciptr;
int		option;
int		arg;

{
    int	fd = ciptr->fd;
    int	ret = 0;

    PRMSG (2,"TRANS(SetOption) (%d,%d,%d)\n", fd, option, arg);

    /*
     * For now, all transport type use the same stuff for setting options.
     * As long as this is true, we can put the common code here. Once a more
     * complicated transport such as shared memory or an OSI implementation
     * that uses the session and application libraries is implemented, this
     * code may have to move to a transport dependent function.
     *
     * ret = ciptr->transptr->SetOption (ciptr, option, arg);
     */

    switch (option)
    {
    case TRANS_NONBLOCKING:
	switch (arg)
	{
	case 0:
	    /* Set to blocking mode */
	    break;
	case 1: /* Set to non-blocking mode */
#if defined(O_NONBLOCK) && (!defined(ultrix) && !defined(hpux) && !defined(AIXV3) && !defined(uniosu))
	    ret = fcntl (fd, F_SETFL, O_NONBLOCK);
#else
#ifdef FIOSNBIO
	{
	    int arg;
	    arg = 1;
	    ret = ioctl (fd, FIOSNBIO, &arg);
	}
#else
#if (defined(AIXV3) || defined(uniosu) || defined(WIN32)) && defined(FIONBIO)
	{
	    int arg;
	    arg = 1;
	    ret = ioctl (fd, FIONBIO, &arg);
	}
#else
#ifdef FNDELAY
	    ret = fcntl (fd, F_SETFL, FNDELAY);
#else
	    ret = fcntl (fd, F_SETFL, O_NDELAY);
#endif
#endif /* AIXV3  || uniosu */
#endif /* FIOSNBIO */
#endif /* O_NONBLOCK */
	    break;
	default:
	    /* Unknown option */
	    break;
	}
	break;
    case TRANS_CLOSEONEXEC:
#ifdef F_SETFD
#ifdef FD_CLOEXEC
	ret = fcntl (fd, F_SETFD, FD_CLOEXEC);
#else
	ret = fcntl (fd, F_SETFD, 1);
#endif /* FD_CLOEXEC */
#endif /* F_SETFD */
	break;
    }
    
    return ret;
}

int
TRANS(CreateListener) (ciptr, port)

XtransConnInfo	ciptr;
char		*port;

{
    return ciptr->transptr->CreateListener (ciptr, port);
}


int
TRANS(ResetListener) (ciptr)

XtransConnInfo	ciptr;

{
    if (ciptr->transptr->ResetListener)
	return ciptr->transptr->ResetListener (ciptr);
    else
	return TRANS_RESET_NOOP;
}


XtransConnInfo
TRANS(Accept) (ciptr)

XtransConnInfo	ciptr;

{
    XtransConnInfo	newciptr;

    PRMSG (2,"TRANS(Accept) (%d)\n", ciptr->fd, 0, 0);

    newciptr = ciptr->transptr->Accept (ciptr);

    if (newciptr)
	newciptr->transptr = ciptr->transptr;

    return newciptr;
}

int
TRANS(Connect) (ciptr, address)

XtransConnInfo	ciptr;
char		*address;

{
    char	*protocol;
    char	*host;
    char	*port;
    int		ret;

    PRMSG (2,"TRANS(Connect) (%d,%s)\n", ciptr->fd, address, 0);

    if (TRANS(ParseAddress) (address, &protocol, &host, &port) == 0)
    {
	PRMSG (1,"TRANS(Connect): Unable to Parse address %s\n",
	       address, 0, 0);
	return -1;
    }

    if (!port || !*port)
    {
	PRMSG (1,"TRANS(Connect): Missing port specification in %s\n",
	      address, 0, 0);
	if (protocol) free (protocol);
	if (host) free (host);
	return -1;
    }

    ret = ciptr->transptr->Connect (ciptr, host, port);

    if (protocol) free (protocol);
    if (host) free (host);
    if (port) free (port);
    
    return ret;
}

int
TRANS(BytesReadable) (ciptr, pend)

XtransConnInfo	ciptr;
BytesReadable_t	*pend;

{
    return ciptr->transptr->BytesReadable (ciptr, pend);
}

int
TRANS(Read) (ciptr, buf, size)

XtransConnInfo	ciptr;
char		*buf;
int		size;

{
    return ciptr->transptr->Read (ciptr, buf, size);
}

int
TRANS(Write) (ciptr, buf, size)

XtransConnInfo	ciptr;
char		*buf;
int		size;

{
    return ciptr->transptr->Write (ciptr, buf, size);
}

int
TRANS(Readv) (ciptr, buf, size)

XtransConnInfo	ciptr;
struct iovec	*buf;
int		size;
{
    return ciptr->transptr->Readv (ciptr, buf, size);
}

int
TRANS(Writev) (ciptr, buf, size)

XtransConnInfo	ciptr;
struct iovec	*buf;
int		size;

{
    return ciptr->transptr->Writev (ciptr, buf, size);
}

int
TRANS(Disconnect) (ciptr)

XtransConnInfo	ciptr;

{
    return ciptr->transptr->Disconnect (ciptr);
}

int
TRANS(Close) (ciptr)

XtransConnInfo	ciptr;

{
    int ret;

    PRMSG (2,"TRANS(Close) (%d)\n", ciptr->fd, 0, 0);

    ret = ciptr->transptr->Close (ciptr);

    TRANS(FreeConnInfo) (ciptr);

    return ret;
}

#ifdef not_yet
TRANS(NameToAddr) (XtransConnInfo ciptr /*???what else???*/)
{
}

TRANS(AddrToName) (XtransConnInfo ciptr /*???what else???*/)
{
}
#endif


int
TRANS(IsLocal) (ciptr)

XtransConnInfo	ciptr;

{
    return (ciptr->family == AF_UNIX);
}


int
TRANS(GetMyAddr) (ciptr, familyp, addrlenp, addrp)

XtransConnInfo	ciptr;
int		*familyp;
int		*addrlenp;
Xtransaddr	**addrp;

{
    PRMSG (2,"TRANS(GetMyAddr) (%d)\n", ciptr->fd, 0, 0);

    *familyp = ciptr->family;
    *addrlenp = ciptr->addrlen;

    if ((*addrp = (Xtransaddr *) malloc (ciptr->addrlen)) == NULL)
    {
	PRMSG (1,"TRANS(GetMyAddr) malloc failed\n", 0, 0, 0);
	return -1;
    }
    memcpy(*addrp, ciptr->addr, ciptr->addrlen);

    return 0;
}

int
TRANS(GetPeerAddr) (ciptr, familyp, addrlenp, addrp)

XtransConnInfo	ciptr;
int		*familyp;
int		*addrlenp;
Xtransaddr	**addrp;

{
    PRMSG (2,"TRANS(GetPeerAddr) (%d)\n", ciptr->fd, 0, 0);

    *familyp = ciptr->family;
    *addrlenp = ciptr->peeraddrlen;

    if ((*addrp = (Xtransaddr *) malloc (ciptr->peeraddrlen)) == NULL)
    {
	PRMSG (1,"TRANS(GetMyAddr) malloc failed\n", 0, 0, 0);
	return -1;
    }
    memcpy(*addrp, ciptr->peeraddr, ciptr->peeraddrlen);

    return 0;
}


int
TRANS(GetConnectionNumber) (ciptr)

XtransConnInfo	ciptr;

{
    return ciptr->fd;
}


/*
 * These functions are really utility functions, but they require knowledge
 * of the internal data structures, so they have to be part of the Transport
 * Independant API.
 */

static int
complete_network_count ()

{
    int count = 0;
    int found_local = 0;
    int i;

    /*
     * For a complete network, we only need one LOCALCONN transport to work
     */

    for (i = 0; i < NUMTRANS; i++)
    {
	if (Xtransports[i]->flags & TRANS_ALIAS)
	    continue;

	if (Xtransports[i]->flags & TRANS_LOCAL)
	    found_local = 1;
	else
	    count++;
    }

    return (count + found_local);
}


int
TRANS(MakeAllCOTSServerListeners) (port, partial, count_ret, ciptrs_ret)

char		*port;
int		*partial;
int		*count_ret;
XtransConnInfo 	**ciptrs_ret;

{
    char		buffer[256]; /* ??? What size ?? */
    XtransConnInfo	ciptr, temp_ciptrs[NUMTRANS];
    int			i;

    PRMSG (2,"TRANS(MakeAllCOTSServerListeners) (%s,%x)\n",
	   port, ciptrs_ret, 0);

    *count_ret = 0;

    for (i = 0; i < NUMTRANS; i++)
    {
	if (Xtransports[i]->flags&TRANS_ALIAS)
	    continue;

	sprintf(buffer,"%s/:%s", Xtransports[i]->TransName, port);

	PRMSG (5,"TRANS(MakeAllCOTSServerListeners) opening %s\n",
	       buffer, 0, 0);

	if ((ciptr = TRANS(OpenCOTSServer(buffer))) == NULL)
	{
	    PRMSG (1,
	  "TRANS(MakeAllCOTSServerListeners) failed to open listener for %s\n",
		  Xtransports[i]->TransName, 0, 0);
	    continue;
	}

	if (TRANS(CreateListener (ciptr, port)) < 0)
	{
	    PRMSG (1,
	"TRANS(MakeAllCOTSServerListeners) failed to create listener for %s\n",
		  Xtransports[i]->TransName, 0, 0);
	    continue;
	}

	PRMSG (5,
	      "TRANS(MakeAllCOTSServerListeners) opened listener for %s, %d\n",
	      Xtransports[i]->TransName, ciptr->fd, 0);

	temp_ciptrs[*count_ret] = ciptr;
	(*count_ret)++;
    }

    *partial = (*count_ret < complete_network_count());

    PRMSG (5,
     "TRANS(MakeAllCLTSServerListeners) partial=%d, actual=%d, complete=%d \n",
	*partial, *count_ret, complete_network_count());

    if (*count_ret > 0)
    {
	if ((*ciptrs_ret = (XtransConnInfo *) malloc (
	    *count_ret * sizeof (XtransConnInfo))) == NULL)
	{
	    return -1;
	}

	for (i = 0; i < *count_ret; i++)
	{
	    (*ciptrs_ret)[i] = temp_ciptrs[i];
	}
    }
    else
	*ciptrs_ret = NULL;
 
    return 0;
}

int
TRANS(MakeAllCLTSServerListeners) (port, partial, count_ret, ciptrs_ret)

char		*port;
int		*partial;
int		*count_ret;
XtransConnInfo 	**ciptrs_ret;

{
    char		buffer[256]; /* ??? What size ?? */
    XtransConnInfo	ciptr, temp_ciptrs[NUMTRANS];
    int			i;

    PRMSG (2,"TRANS(MakeAllCLTSServerListeners) (%s,%x)\n",
	port, ciptrs_ret, 0);

    *count_ret = 0;

    for (i = 0; i < NUMTRANS; i++)
    {
	if (Xtransports[i]->flags&TRANS_ALIAS)
	    continue;

	sprintf(buffer,"%s/:%s", Xtransports[i]->TransName, port);

	PRMSG (5,"TRANS(MakeAllCLTSServerListeners) opening %s\n",
	    buffer, 0, 0);

	if ((ciptr = TRANS(OpenCLTSServer (buffer))) == NULL)
	{
	    PRMSG (1,
	"TRANS(MakeAllCLTSServerListeners) failed to open listener for %s\n",
		  Xtransports[i]->TransName, 0, 0);
	    continue;
	}

	if (TRANS(CreateListener (ciptr, port)) < 0)
	{
	    PRMSG (1,
	"TRANS(MakeAllCLTSServerListeners) failed to create listener for %s\n",
		  Xtransports[i]->TransName, 0, 0);
	    continue;
	}

	PRMSG (5,
	"TRANS(MakeAllCLTSServerListeners) opened listener for %s, %d\n",
	      Xtransports[i]->TransName, ciptr->fd, 0);
	temp_ciptrs[*count_ret] = ciptr;
	(*count_ret)++;
    }

    *partial = (*count_ret < complete_network_count());

    PRMSG (5,
     "TRANS(MakeAllCLTSServerListeners) partial=%d, actual=%d, complete=%d \n",
	*partial, *count_ret, complete_network_count());

    if (*count_ret > 0)
    {
	if ((*ciptrs_ret = (XtransConnInfo *) malloc (
	    *count_ret * sizeof (XtransConnInfo))) == NULL)
	{
	    return -1;
	}

	for (i = 0; i < *count_ret; i++)
	{
	    (*ciptrs_ret)[i] = temp_ciptrs[i];
	}
    }
    else
	*ciptrs_ret = NULL;
    
    return 0;
}



/*
 * These routines are not part of the X Transport Interface, but they
 * may be used by it.
 */

#ifdef CRAY

/*
 * Cray UniCOS does not have readv and writev so we emulate
 */

static int TRANS(ReadV) (ciptr, iov, iovcnt)

XtransConnInfo	ciptr;
struct iovec 	*iov;
int 		iovcnt;

{
    struct msghdr hdr;

    hdr.msg_iov = iov;
    hdr.msg_iovlen = iovcnt;
    hdr.msg_accrights = 0;
    hdr.msg_accrightslen = 0;
    hdr.msg_name = 0;
    hdr.msg_namelen = 0;

    return (recvmsg (ciptr->fd, &hdr, 0));
}

static int TRANS(WriteV) (ciptr, iov, iovcnt)

XtransConnInfo	ciptr;
struct iovec 	*iov;
int 		iovcnt;

{
    struct msghdr hdr;

    hdr.msg_iov = iov;
    hdr.msg_iovlen = iovcnt;
    hdr.msg_accrights = 0;
    hdr.msg_accrightslen = 0;
    hdr.msg_name = 0;
    hdr.msg_namelen = 0;

    return (sendmsg (ciptr->fd, &hdr, 0));
}

#endif /* CRAY */

#if (defined(SYSV) && defined(SYSV386)) || defined(WIN32) || defined(__sxg__)

/*
 * SYSV/386 and WIN32 do not have readv so we emulate
 */

static int TRANS(ReadV) (ciptr, iov, iovcnt)

XtransConnInfo	ciptr;
struct iovec 	*iov;
int 		iovcnt;

{
    int i, len, total;
    char *base;

    ESET(0);
    for (i = 0, total = 0;  i < iovcnt;  i++, iov++) {
	len = iov->iov_len;
	base = iov->iov_base;
	while (len > 0) {
	    register int nbytes;
	    nbytes = TRANS(Read) (ciptr, base, len);
	    if (nbytes < 0 && total == 0)  return -1;
	    if (nbytes <= 0)  return total;
	    ESET(0);
	    len   -= nbytes;
	    total += nbytes;
	    base  += nbytes;
	}
    }
    return total;
}

#endif /* SYSV && SYSV386 || WIN32 || __sxg__ */

#if defined(WIN32) || defined(__sxg__)

/*
 * WIN32 does not have readv so we emulate
 */

static int TRANS(WriteV) (ciptr, iov, iovcnt)

XtransConnInfo	ciptr;
struct iovec 	*iov;
int 		iovcnt;

{
    int i, len, total;
    char *base;

    ESET(0);
    for (i = 0, total = 0;  i < iovcnt;  i++, iov++) {
	len = iov->iov_len;
	base = iov->iov_base;
	while (len > 0) {
	    register int nbytes;
	    nbytes = TRANS(Write) (ciptr, base, len);
	    if (nbytes < 0 && total == 0)  return -1;
	    if (nbytes <= 0)  return total;
	    ESET(0);
	    len   -= nbytes;
	    total += nbytes;
	    base  += nbytes;
	}
    }
    return total;
}

#endif /* WIN32 || __sxg__ */


#if (defined(_POSIX_SOURCE) && !defined(AIXV3)) || defined(hpux) || defined(USG) || defined(SVR4)
#define NEED_UTSNAME
#include <sys/utsname.h>
#endif

/*
 * TRANS(GetHostname) - similar to gethostname but allows special processing.
 */

int TRANS(GetHostname) (buf, maxlen)

char *buf;
int maxlen;

{
    int len;

#ifdef NEED_UTSNAME
    struct utsname name;

    uname (&name);
    len = strlen (name.nodename);
    if (len >= maxlen) len = maxlen - 1;
    strncpy (buf, name.nodename, len);
    buf[len] = '\0';
#else
    buf[0] = '\0';
    (void) gethostname (buf, maxlen);
    buf [maxlen - 1] = '\0';
    len = strlen(buf);
#endif /* NEED_UTSNAME */
    return len;
}
