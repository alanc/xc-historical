#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#include <sys/ioctl.h>

/*
 * This is the DNET implementation of the X Transport service layer
 */

/*
 * This function gets the local address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */
static int
TRANS(DNETGetAddr)(ciptr)
XtransConnInfo ciptr;
{
Xtransaddr	sockname;
int		namelen=sizeof(sockname);

PRMSG(3,"TRANS(DNETGetAddr)(%x)\n", ciptr, 0,0 );

if( getsockname(ciptr->fd,(struct sockaddr *)&sockname,&namelen) < 0 )
	{
	PRMSG(1,"TRANS(DNETGetAddr): getsockname() failed: %d\n",
	    						EGET(),0,0);
	return -1;
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

if( (ciptr->addr=(char *)malloc(namelen)) == NULL )
        {
        PRMSG(1, "TRANS(DNETGetAddr): Can't allocate space for the addr\n",
									0,0,0);
        return -1;
        }

ciptr->family=sockname.family;
ciptr->addrlen=namelen;
memcpy(ciptr->addr,&sockname,ciptr->addrlen);

return 0;
}

/*
 * This function gets the remote address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */
static int
TRANS(DNETGetPeerAddr)(ciptr)
XtransConnInfo ciptr;
{
Xtransaddr	sockname;
int		namelen=sizeof(sockname);

PRMSG(3,"TRANS(DNETGetPeerAddr)(%x)\n", ciptr, 0,0 );

if( getpeername(ciptr->fd,(struct sockaddr *)&sockname,&namelen) < 0 )
	{
	PRMSG(1,"TRANS(DNETGetPeerAddr): getpeername() failed: %d\n",
						EGET(), 0,0 );
	return -1;
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

if( (ciptr->peeraddr=(char *)malloc(namelen)) == NULL )
        {
        PRMSG(1,
        "TRANS(DNETGetPeerAddr): Can't allocate space for the addr\n",
									0,0,0);
        return -1;
        }

ciptr->peeraddrlen=namelen;
memcpy(ciptr->peeraddr,&sockname,ciptr->peeraddrlen);

return 0;
}


static XtransConnInfo
TRANS(DNETOpenCOTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
XtransConnInfo	ciptr;

PRMSG(2,"TRANS(DNETOpenCOTSClient)(%s,%s,%s)\n", protocol, host, port );

if( (ciptr=(XtransConnInfo)calloc(1,sizeof(struct _XtransConnInfo))) == NULL )
	{
	PRMSG(1, "TRANS(DNETOpenCOTSClient): malloc failed\n", 0,0,0 );
	return NULL;
	}

/* nothing else to do here */

return ciptr;
}

static XtransConnInfo
TRANS(DNETOpenCOTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
PRMSG(2,"TRANS(DNETOpenCOTSServer)(%s,%s,%s)\n", protocol, host, port );
}

static XtransConnInfo
TRANS(DNETOpenCLTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
XtransConnInfo	ciptr;

PRMSG(2,"TRANS(DNETOpenCLTSClient)(%s,%s,%s)\n", protocol, host, port );

if( (ciptr=(XtransConnInfo)calloc(1,sizeof(struct _XtransConnInfo))) == NULL )
	{
	PRMSG(1, "TRANS(DNETOpenCLTSClient): malloc failed\n", 0,0,0 );
	return NULL;
	}

/* nothing else to do here */

return ciptr;
}

static XtransConnInfo
TRANS(DNETOpenCLTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
PRMSG(2,"TRANS(DNETOpenCLTSServer)(%s,%s,%s)\n", protocol, host, port );
return NULL;
}

static int
TRANS(DNETSetOption)(ciptr, option, arg)
XtransConnInfo	ciptr;
int		option;
int		arg;
{
PRMSG(2,"TRANS(DNETSetOption)(%d,%d,%d)\n", ciptr->fd, option, arg );

return -1;
}

static int
TRANS(DNETCreateListener)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
int	fd=ciptr->fd;

PRMSG(3, "TRANS(DNETCreateListener)(%x,%d)\n", ciptr, fd, 0 );
}

static XtransConnInfo
TRANS(DNETAccept)(ciptr)
XtransConnInfo	ciptr;
{
PRMSG(2, "TRANS(DNETAccept)(%x,%d)\n", ciptr, ciptr->fd, 0 );

return NULL;
}

static int
TRANS(DNETConnect)(ciptr, host, port)
XtransConnInfo	ciptr;
char		*host;
char		*port;
{
#define OBJBUFSIZE 64
char objname[OBJBUFSIZE];

extern int dnet_conn();

PRMSG(2,"TRANS(DNETConnect)(%d,%s,%s)\n", ciptr->fd, host, port );

#ifdef X11
/*
 * X has a well known port, that is transport dependent. It is easier
 * to handle it here, than try and come up with a transport independent
 * representation that can be passed in and resolved the usual way.
 *
 * The port that is passed here is really a string containing the idisplay
 * from ConnectDisplay().
 *
 * Xlib may be calling this for either X11 or IM. Assume that
 * if port < IP_RESERVE, then is is a display number. Otherwise, it is a
 * regular port number for IM.
 */

if (is_numeric (port))
{
    short tmpport = (short) atoi (port);

    if( tmpport < 1024 ) /* IP_RESERVED */
	sprintf (objname, "X$X%d", tmpport);
    else
	strncpy (objname, port, OBJBUFSIZE);
}
else
#endif
strncpy(objname, port, OBJBUFSIZE);

/*
 * Do the connect
 */

if (!host) host = "0";

if ((ciptr->fd = dnet_conn (host, objname, SOCK_STREAM, 0, 0, 0, 0)) < 0)
{
    return TRANS_CONNECT_FAILED;
}

/*
 * Sync up the address fields of ciptr.
 */

if( TRANS(DNETGetAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(DNETConnect): TRANS(DNETGetAddr)() failed:\n", 0,0,0 );
	return TRANS_CONNECT_FAILED;
	}

if( TRANS(DNETGetPeerAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(DNETConnect): TRANS(DNETGetPeerAddr)() failed:\n",
								0,0,0 );
	return TRANS_CONNECT_FAILED;
	}

return 0;
}

static int
TRANS(DNETBytesReadable)(ciptr, pend)
XtransConnInfo	ciptr;
BytesReadable_t	*pend;
{
PRMSG(2,"TRANS(DNETBytesReadable)(%x,%d,%x)\n", ciptr, ciptr->fd, pend );

return ioctl(ciptr->fd, FIONREAD, (char *)pend);
}

static int
TRANS(DNETRead)(ciptr, buf, size)
XtransConnInfo	ciptr;
char		*buf;
int		size;
{
PRMSG(2,"TRANS(DNETRead)(%d,%x,%d)\n", ciptr->fd, buf, size );

return read(ciptr->fd,buf,size);
}

static int
TRANS(DNETWrite)(ciptr, buf, size)
XtransConnInfo	ciptr;
char		*buf;
int		size;
{
PRMSG(2,"TRANS(DNETWrite)(%d,%x,%d)\n", ciptr->fd, buf, size );

return write(ciptr->fd,buf,size);
}

static int
TRANS(DNETReadv)(ciptr, buf, size)
XtransConnInfo	ciptr;
struct iovec	*buf;
int		size;
{
PRMSG(2,"TRANS(DNETReadv)(%d,%x,%d)\n", ciptr->fd, buf, size );

return READV(ciptr,buf,size);
}

static int
TRANS(DNETWritev)(ciptr, buf, size)
XtransConnInfo	ciptr;
struct iovec	*buf;
int		size;
{
PRMSG(2,"TRANS(DNETWritev)(%d,%x,%d)\n", ciptr->fd, buf, size );

return WRITEV(ciptr,buf,size);
}

static int
TRANS(DNETDisconnect)(ciptr)
XtransConnInfo	ciptr;
{
PRMSG(2,"TRANS(DNETDisconnect)(%x,%d)\n", ciptr, ciptr->fd, 0 );

return shutdown(ciptr->fd,2); /* disallow further sends and receives */
}

static int
TRANS(DNETClose)(ciptr)
XtransConnInfo	ciptr;
{
PRMSG(2,"TRANS(DNETClose)(%x,%d)\n", ciptr, ciptr->fd, 0 );

return close(ciptr->fd);
}

static int
TRANS(DNETNameToAddr)(ciptr)
XtransConnInfo	ciptr;
{
return -1;
}

static int
TRANS(DNETAddrToName)(ciptr)
XtransConnInfo	ciptr;
{
return -1;
}

Xtransport	TRANS(DNETFuncs) = {
	/* DNET Interface */
	"decnet",
	0,
	TRANS(DNETOpenCOTSClient),
	TRANS(DNETOpenCOTSServer),
	TRANS(DNETOpenCLTSClient),
	TRANS(DNETOpenCLTSServer),
	TRANS(DNETSetOption),
	TRANS(DNETCreateListener),
	TRANS(DNETAccept),
	TRANS(DNETConnect),
	TRANS(DNETBytesReadable),
	TRANS(DNETRead),
	TRANS(DNETWrite),
	TRANS(DNETReadv),
	TRANS(DNETWritev),
	TRANS(DNETDisconnect),
	TRANS(DNETClose),
	TRANS(DNETNameToAddr),
	TRANS(DNETAddrToName),
};
