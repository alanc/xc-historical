/*
 * The transport table contains a definition for every transport (protocol)
 * family. All operations that can be made on the transport go through this
 * table.
 *
 * New transports can be added by adding an entry in this table.
 */

static
Xtransport	*Xtransports[] = {
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

/*
 * Connection Info table. This array keeps track of information unique to each
 * connection, such as the transport type and the address.
 * The transport index is stored in the table using
 * the fd as the index.
 */

static
XtransConnInfo **TRANS(conninfo) = NULL; /* Gets initialized at run time */

#define GetConnectionInfo(fd)		TRANS(conninfo)[fd]

#ifdef WIN32
#define ESET(val) WSASetLastError(val)
#define ioctl ioctlsocket
#else
#define ESET(val) errno = val
#endif



/*
 * These a few utility function used by the public interface functions.
 */
static void
TRANS(SetConnInfo)(ciptr, trans)
XtransConnInfo	*ciptr;
Xtransport	*trans;
{
int	fd;

PRMSG(3,"TRANS(SetConnInfo)(%x,%x)\n",ciptr, trans, 0);

if( TRANS(conninfo) == NULL )
	{
	/* First call: allocate array */

	TRANS(conninfo)=(XtransConnInfo **)calloc(100,sizeof(XtransConnInfo *));
				/* XXXSTU This should be determined in a
				   POSIX manner based on how many fds are
				   configured. Similar thing was done in Xt. */
	}

fd=ciptr->fd;
ciptr->transptr=trans;

TRANS(conninfo)[fd]=ciptr;
}

static void
TRANS(FreeConnInfo)(ciptr)
XtransConnInfo	*ciptr;
{
int	fd;

PRMSG(3,"TRANS(FreeConnInfo)(%x)\n",ciptr, 0, 0);

fd=ciptr->fd;

if(ciptr->addr)
	free(ciptr->addr);

if(ciptr->peeraddr)
	free(ciptr->peeraddr);

free(ciptr);

TRANS(conninfo)[fd]=NULL;
}

static Xtransport *
TRANS(SelectTransport)(protocol)
char	*protocol;
{
int	i;
#define PROTOBUFSIZE	20
char	protobuf[PROTOBUFSIZE];

PRMSG(3,"TRANS(SelectTransport)(%s)\n",protocol, 0, 0);

/* Force Protocol to be lowercase as a way of doing a case insensitive match */

strncpy(protobuf,protocol,PROTOBUFSIZE);

for(i=0;i<PROTOBUFSIZE&&protobuf[i]!='\0';i++)
	protobuf[i]=tolower(protobuf[i]);

/* Look at all of the configured protocols */

for(i=0;i<NUMTRANS;i++)
	{
	if( !strcmp(protobuf,Xtransports[i]->TransName) )
		return Xtransports[i];
	}
return NULL;
}

#ifndef TEST
static
#endif /* TEST */
int
TRANS(ParseAddress)(address, protocol, host, port)
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
char	*mybuf,*tmpptr;
char	*_protocol,*_host,*_port;

PRMSG(3,"TRANS(ParseAddress)(%s)\n",address, 0, 0);

/* Copy the string so it can be changed */

tmpptr=mybuf=(char *) malloc (strlen (address) + 1);
strcpy (mybuf, address);

/* Parse the string to get each component */

/* Get the protocol part */

_protocol=mybuf;

if( (mybuf=strpbrk(mybuf,"/:")) == NULL )
	{
	/* adress is in a bad format */
	*protocol=NULL;
	*host=NULL;
	*port=NULL;
	free(tmpptr);
	return 0;
	}

if( *mybuf == ':' )
	{
	/*
	 * If there is a hostname, then assume inet, otherwise
	 * it must be local.
	 */
	if( mybuf == tmpptr )
		{
		/* There is neither a protocol or host specified */
		_protocol="local";
		}
	else
		{
		/* Ther is a hostname specified */
		_protocol="inet";
		mybuf = tmpptr;	/* reset to the begining of the host ptr */
		}
	}
else
	{
	/* *mybuf == '/' */

	*mybuf++='\0'; /* put a null at the end of the protocol */

	if( strlen(_protocol) == 0 )
		{
		/*
		 * If there is a hostname, then assume inet, otherwise
		 * it must be local.
		 */
		if( *mybuf != ':' )
			_protocol="inet";
		else
			_protocol="local";
		}
	}

/* Get the host part */

_host=mybuf;

if( (mybuf=strchr(mybuf,':')) == NULL )
	{
	*protocol=NULL;
	*host=NULL;
	*port=NULL;
	free(tmpptr);
	return 0;
	}

*mybuf++='\0';

if( strlen(_host) == 0 )
	{
	_host="local";
	}

/* Check for DECnet */

if( *mybuf == ':' )
	{
	_protocol="decnet";
	mybuf++;
	}

/* The rest is the port */

get_port:

_port=mybuf;

/*
 * Now that we have all of the components, allocate new
 * string space for them.
 */

if( (*protocol=(char *) malloc(strlen (_protocol) + 1)) == NULL )
	{
	/* Malloc failed */
	*port=NULL;
	*host=NULL;
	*protocol=NULL;
	free(tmpptr);
	return 0;
	}
else
        strcpy (*protocol, _protocol);

if( (*host=(char *) malloc (strlen (_host) + 1)) == NULL )
	{
	/* Malloc failed */
	*port=NULL;
	*host=NULL;
	free(*protocol);
	*protocol=NULL;
	free(tmpptr);
	return 0;
	}
else
        strcpy (*host, _host);

if( (*port=(char *) malloc (strlen (_port) + 1)) == NULL )
	{
	/* Malloc failed */
	*port=NULL;
	free(*host);
	*host=NULL;
	free(*protocol);
	*protocol=NULL;
	free(tmpptr);
	return 0;
	}
else
        strcpy (*port, _port);

return 1;
}


/*
 * TRANS(Open) does all of the real work opening a connection. The only
 * funny part about this is the type parameter which is used to decide which
 * type of open to perform.
 */

static int
TRANS(Open)(type, address)
int	type;
char	*address;
{
char *protocol=NULL, *host=NULL, *port=NULL;
XtransConnInfo	*ciptr=NULL;
Xtransport	*thistrans;

PRMSG(2,"TRANS(Open)(%d,%s)\n",type,address,0);

/* Parse the Address */

if( TRANS(ParseAddress)(address,&protocol,&host,&port) == 0 )
	{
	PRMSG(1,"TRANS(Open): Unable to Parse address %s\n", address, 0,0 );
	return -1;
	}

/* Determine the transport type */

if( (thistrans=TRANS(SelectTransport)(protocol)) == NULL )
	{
	PRMSG(1,"TRANS(Open): Unable to find transport for %s\n", protocol,0,0);
	free(protocol);
	free(host);
	free(port);
	return -1;
	}

/* Open the transport */

switch( type )
	{
	case XTRANS_OPEN_COTS_CLIENT:
		ciptr=thistrans->OpenCOTSClient(thistrans,protocol,host,port);
		break;
	case XTRANS_OPEN_COTS_SERVER:
		ciptr=thistrans->OpenCOTSServer(thistrans,protocol,host,port);
		break;
	case XTRANS_OPEN_CLTS_CLIENT:
		ciptr=thistrans->OpenCLTSClient(thistrans,protocol,host,port);
		break;
	case XTRANS_OPEN_CLTS_SERVER:
		ciptr=thistrans->OpenCLTSServer(thistrans,protocol,host,port);
		break;
	default:
	PRMSG(1,"TRANS(Open): Unknown Open type %d\n", type, 0,0 );
	}

if( ciptr == NULL )
	{
	PRMSG(1,"TRANS(Open): transport open failed for %s/%s:%s\n",
						protocol, host, port );
	free(protocol);
	free(host);
	free(port);
	return -1;
	}

TRANS(SetConnInfo)(ciptr,thistrans);

free(protocol);
free(host);
free(port);

return ciptr->fd;
}

#define	PASSTHRUCALL(func,args)	\
Xtransport	*thistrans;	\
XtransConnInfo	*ciptr;		\
				\
ciptr=GetConnectionInfo(fd);	\
thistrans=ciptr->transptr;	\
				\
return thistrans->func args

/*
 * These are the public interfaces to this Transport interface.
 * These are the only functions that should have knowledge of the transport
 * table.
 */

int
TRANS(OpenCOTSClient)(address)
char	*address;
{
PRMSG(2,"TRANS(OpenCOTSClient)(%s)\n", address, 0,0);
return TRANS(Open)(XTRANS_OPEN_COTS_CLIENT, address);
}

int
TRANS(OpenCOTSServer)(address)
char	*address;
{
PRMSG(2,"TRANS(OpenCOTSServer)(%s)\n", address, 0,0);
return TRANS(Open)(XTRANS_OPEN_COTS_SERVER, address);
}

int
TRANS(OpenCLTSClient)(address)
char	*address;
{
PRMSG(2,"TRANS(OpenCLTSClient)(%s)\n", address, 0,0);
return TRANS(Open)(XTRANS_OPEN_CLTS_CLIENT, address);
}

int
TRANS(OpenCLTSServer)(address)
char	*address;
{
PRMSG(2,"TRANS(OpenCLTSServer)(%s)\n", address, 0,0);
return TRANS(Open)(XTRANS_OPEN_CLTS_SERVER, address);
}

void
TRANS(SetOption)(fd, option, arg)
int	fd;
int	option;
int	arg;
{
PRMSG(2,"TRANS(SetOption)(%d,%d,%d)\n", fd, option, arg);
/*
 * For now, all transport type use the same stuff for setting options.
 * As long as this is true, we can put the common code here. Once a more
 * complicated transport such as shared memory or an OSI implementation that
 * uses the session and application libraries is implemented, this code may
 * have to move to a transport dependent function.
 *
 * PASSTHRUCALL(SetOption,(ciptr, fd, option, arg));
 */
switch(option)
	{
	case TRANS_NONBLOCKING:
		switch( arg )
			{
			case 0:
				/* Set to blocking mode */
				break;
			case 1: /* Set to non-blocking mode */
#if defined(O_NONBLOCK) && (!defined(ultrix) && !defined(hpux) && !defined(AIXV3) && !defined(uniosu))
        			(void) fcntl (fd, F_SETFL, O_NONBLOCK);
#else
#ifdef FIOSNBIO
        			{
            				int arg;
            				arg = 1;
            				ioctl(fd, FIOSNBIO, &arg);
        			}
#else
#if (defined(AIXV3) || defined(uniosu) || defined(WIN32)) && defined(FIONBIO)
        			{
            				int arg;
            				arg = 1;
            				ioctl(fd, FIONBIO, &arg);
        			}
#else
					fcntl (fd, F_SETFL, FNDELAY);
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
		(void) fcntl (fd, F_SETFD, FD_CLOEXEC);
#else
		(void) fcntl (fd, F_SETFD, 1);
#endif /* FD_CLOEXEC */
#endif /* F_SETFD */
		break;
	case TRANS_COALESCENCE:
		break;
	}
}

int
TRANS(CreateListener)(fd, port)
int	fd;
char	*port;
{
PASSTHRUCALL(CreateListener,(ciptr, fd, port));
}

TRANS(Accept)(fd)
int	fd;
{
Xtransport	*thistrans;
XtransConnInfo	*ciptr;
XtransConnInfo	*newciptr;

PRMSG(2,"TRANS(Accept)(%d)\n", fd, 0,0);

ciptr=GetConnectionInfo(fd);
thistrans=ciptr->transptr;

newciptr=thistrans->Accept(ciptr,fd);

if( newciptr == NULL)
	return -1;

TRANS(SetConnInfo)(newciptr,thistrans);

return newciptr->fd;
}

TRANS(Connect)(fd, address)
int	fd;
char	*address;
{
Xtransport	*thistrans;
XtransConnInfo	*ciptr;
char	*protocol;
char	*host;
char	*port;
int	ret;

PRMSG(2,"TRANS(Connect)(%d,%s)\n", fd, address, 0);

ciptr=GetConnectionInfo(fd);
thistrans=ciptr->transptr;

if( TRANS(ParseAddress)(address,&protocol,&host,&port) == 0 )
	{
	PRMSG(1,"TRANS(Connect): Unable to Parse address %s\n", address, 0,0 );
	return -1;
	}

if( !port || !*port )
	{
	PRMSG(1,"TRANS(Connect): Missing port specification in %s\n",
							address, 0,0 );
	if(protocol) free(protocol);
	if(host) free(host);
	return -1;
	}

ret=thistrans->Connect(ciptr, fd, host, port);

if(protocol) free(protocol);
if(host) free(host);
if(port) free(port);

return ret;
}

TRANS(BytesReadable)(fd, pend)
int	fd;
BytesReadable_t	*pend;
{
PASSTHRUCALL(BytesReadable,(ciptr, fd, pend));
}

TRANS(Read)(fd, buf, size)
int	fd;
char	*buf;
int	size;
{
PASSTHRUCALL(Read,(ciptr, fd, buf, size));
}

TRANS(Write)(fd, buf, size)
int	fd;
char	*buf;
int	size;
{
PASSTHRUCALL(Write,(ciptr, fd, buf, size));
}

TRANS(Readv)(fd, buf, size)
int		fd;
struct iovec	*buf;
int		size;
{
PASSTHRUCALL(Readv,(ciptr, fd, buf, size));
}

TRANS(Writev)(fd, buf, size)
int		fd;
struct iovec	*buf;
int		size;
{
PASSTHRUCALL(Writev,(ciptr, fd, buf, size));
}

TRANS(Disconnect)(fd)
int	fd;
{
PASSTHRUCALL(Disconnect,(ciptr, fd));
}

TRANS(Close)(fd)
int	fd;
{
Xtransport	*thistrans;
XtransConnInfo	*ciptr;

PRMSG(2,"TRANS(Close)(%d)\n", fd, 0,0);

ciptr=GetConnectionInfo(fd);
thistrans=ciptr->transptr;

thistrans->Close(ciptr,fd);

TRANS(FreeConnInfo)(ciptr);
}

#ifdef not_yet
TRANS(NameToAddr)(int fd /*???what else???*/ )
{
}

TRANS(AddrToName)(int fd /*???what else???*/ )
{
}
#endif

void
TRANS(GetMyAddr)(fd, familyp, addrlenp, addrp)
int		fd;
int		*familyp;
int		*addrlenp;
Xtransaddr	**addrp;
{
Xtransaddr	*addr;
XtransConnInfo	*ciptr;

PRMSG(2,"TRANS(GetMyAddr)(%d)\n", fd, 0,0);

ciptr=GetConnectionInfo(fd);
*familyp=ciptr->family;
*addrlenp=ciptr->addrlen;
if( (addr=(Xtransaddr *)malloc(ciptr->addrlen)) == NULL )
	{
	PRMSG(1,"TRANS(GetMyAddr) malloc failed\n", 0,0,0 );
	return;
	}
memcpy(addr,ciptr->addr,ciptr->addrlen);
*addrp=addr;

return;
}

void
TRANS(GetPeerAddr)(fd, familyp, addrlenp, addrp )
int		fd;
int		*familyp;
int		*addrlenp;
Xtransaddr	**addrp;
{
Xtransaddr	*addr;
XtransConnInfo	*ciptr;

PRMSG(2,"TRANS(GetPeerAddr)(%d)\n", fd, 0,0);

ciptr=GetConnectionInfo(fd);
*familyp=ciptr->family;
*addrlenp=ciptr->peeraddrlen;
if( (addr=(Xtransaddr *)malloc(ciptr->peeraddrlen)) == NULL )
	{
	PRMSG(1,"TRANS(GetMyAddr) malloc failed\n", 0,0,0 );
	return;
	}
memcpy(addr,ciptr->peeraddr,ciptr->peeraddrlen);
*addrp=addr;

return;
}

/*
 * These functions are really utility functions, but they require knowledge
 * of the internal data structures, so they have to be part of the Transport
 * Independant API.
 */

void
TRANS(MakeAllCOTSServerListeners)(port, fds)
char	*port;
FdMask	*fds;
{
int	i, fd;
char	buffer[256]; /* ??? What size ?? */

PRMSG(2,"TRANS(MakeAllCOTSServerListeners)(%s,%x)\n",port,fds,0);

for(i=0;i<NUMTRANS;i++)
	{
	if(Xtransports[i]->flags&TRANS_ALIAS)
		continue;
	sprintf(buffer,"%s/:%s", Xtransports[i]->TransName, port );
	PRMSG(5,"TRANS(MakeAllCOTSServerListeners) opening %s\n", buffer,0,0 );
	if( (fd=TRANS(OpenCOTSServer(buffer))) < 0 )
		{
		PRMSG(1,
	"TRANS(MakeAllCOTSServerListeners) failed to open listener for %s\n",
					Xtransports[i]->TransName,0,0 );
		continue;
		}
	if( TRANS(CreateListener(fd,port)) < 0 )
		{
		PRMSG(1,
	"TRANS(MakeAllCOTSServerListeners) failed to create listener for %s\n",
					Xtransports[i]->TransName, 0,0 );
		continue;
		}
	PRMSG(5,
	"TRANS(MakeAllCOTSServerListeners) opened listener for %s, %d\n",
					Xtransports[i]->TransName, fd, 0 );
	*fds |= (((FdMask)1) << fd);
	}

return;
}

void
TRANS(MakeAllCLTSServerListeners)(port, fds)
char	*port;
FdMask	*fds;
{
int	i, fd;
char	buffer[256]; /* ??? What size ?? */

PRMSG(2,"TRANS(MakeAllCLTSServerListeners)(%s,%x)\n",port,fds,0);

for(i=0;i<NUMTRANS;i++)
	{
	if(Xtransports[i]->flags&TRANS_ALIAS)
		continue;
	sprintf(buffer,"%s/:%s", Xtransports[i]->TransName, port );
	PRMSG(5,"TRANS(MakeAllCLTSServerListeners) opening %s\n", buffer,0,0 );
	if( (fd=TRANS(OpenCLTSServer(buffer))) < 0 )
		{
		PRMSG(1,
	"TRANS(MakeAllCLTSServerListeners) failed to open listener for %s\n",
					Xtransports[i]->TransName,0,0 );
		continue;
		}
	if( TRANS(CreateListener(fd,port)) < 0 )
		{
		PRMSG(1,
	"TRANS(MakeAllCLTSServerListeners) failed to create listener for %s\n",
					Xtransports[i]->TransName, 0,0 );
		continue;
		}
	PRMSG(5,
	"TRANS(MakeAllCLTSServerListeners) opened listener for %s, %d\n",
					Xtransports[i]->TransName, fd, 0 );
	*fds |= (((FdMask)1) << fd);
	}

return;
}

/*
 * These routines are not part of the X Transport Interface, but they
 * may be used by it.
 */
#ifdef CRAY
/*
 * Cray UniCOS does not have readv and writev so we emulate
 */
#include <sys/socket.h> /* why is this needed?? */

int TRANS(ReadV) (fd, iov, iovcnt)
int fd;
struct iovec *iov;
int iovcnt;
{
	struct msghdr hdr;

	hdr.msg_iov = iov;
	hdr.msg_iovlen = iovcnt;
	hdr.msg_accrights = 0;
	hdr.msg_accrightslen = 0;
	hdr.msg_name = 0;
	hdr.msg_namelen = 0;

	return (recvmsg (fd, &hdr, 0));
}

int TRANS(WriteV) (fd, iov, iovcnt)
int fd;
struct iovec *iov;
int iovcnt;
{
	struct msghdr hdr;

	hdr.msg_iov = iov;
	hdr.msg_iovlen = iovcnt;
	hdr.msg_accrights = 0;
	hdr.msg_accrightslen = 0;
	hdr.msg_name = 0;
	hdr.msg_namelen = 0;

	return (sendmsg (fd, &hdr, 0));
}

#endif /* CRAY */

#if (defined(SYSV) && defined(SYSV386)) || defined(WIN32)
/*
 * SYSV/386 and WIN32 do not have readv so we emulate
 */

int TRANS(ReadV) (fd, iov, iovcnt)
int fd;
struct iovec *iov;
int iovcnt;
{
    int i, len, total;
    char *base;

    ESET(0);
    for (i=0, total=0;  i<iovcnt;  i++, iov++) {
	len = iov->iov_len;
	base = iov->iov_base;
	while (len > 0) {
	    register int nbytes;
	    nbytes = TRANS(Read)(fd, base, len);
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

#endif /* SYSV && SYSV386 || WIN32 */

#ifdef WIN32
/*
 * WIN32 does not have readv so we emulate
 */

int TRANS(WriteV) (fd, iov, iovcnt)
    int fd;
    struct iovec *iov;
    int iovcnt;
{
    int i, len, total;
    char *base;

    ESET(0);
    for (i=0, total=0;  i<iovcnt;  i++, iov++) {
	len = iov->iov_len;
	base = iov->iov_base;
	while (len > 0) {
	    register int nbytes;
	    nbytes = TRANS(Write)(fd, base, len);
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
#endif /* WIN32 */
