#include <sys/socket.h>
#include <sys/un.h>
#include <stropts.h>
#include <tiuser.h>

#include <netdir.h>
#include <netconfig.h>

/*
 * This is the TLI implementation of the X Transport service layer
 */

typedef struct _TLItrans2dev {
	char	*transname;
	char	*protofamily;
	char	*devcotsname;
	char	*devcltsname;
	int	family;
	} TLItrans2dev;

static TLItrans2dev TLItrans2devtab[] = {
	{"inet","inet","/dev/tcp","/dev/udp",AF_INET},
	{"tcp","inet","/dev/tcp","/dev/udp",AF_INET},
	{"tli","loopback","/dev/ticots","/dev/ticlts",AF_UNIX},
	};

#define NUMTLIFAMILIES (sizeof(TLItrans2devtab)/sizeof(TLItrans2dev))

/*
 * The local TLI connection, is a form of a local connection, so use a
 * sockaddr_un for the address so that it will be treated just like the other
 * local transports such as UNIX domain sockets, pts, and named.
 */

#if defined(X11)
#define TLINODENAME	"TLI:xserver"
#endif

#if defined(FS)
#define TLINODENAME	"TLI:fontserver"
#endif

#if defined(ICE)
#define TLINODENAME	"TLI:ICE"
#endif

#if defined(TEST)
#define TLINODENAME	"TLI:test"
#endif

/*
 * These are some utility function used by the real interface function below.
 */
static int
TRANS(TLISelectFamily)(family)
char	*family;
{
int     i;

PRMSG(3,"TRANS(TLISelectFamily)(%s)\n", family, 0,0 );

for(i=0;i<NUMTLIFAMILIES;i++)
        {
        if( !strcmp(family,TLItrans2devtab[i].transname) )
                return i;
        }
return -1;
}

/*
 * This function gets the local address of the transport and stores it in the
 * XtransConnInfo structure for the connection.
 */
static int
TRANS(TLIGetAddr)(ciptr)
XtransConnInfo	*ciptr;
{
Xtransaddr	sockname;
struct netbuf	netbuf;

PRMSG(3,"TRANS(TLIGetAddr)(%x)\n", ciptr, 0,0 );

netbuf.buf=(char *)&sockname;
netbuf.len=sizeof(sockname);
netbuf.maxlen=sizeof(sockname);

if( t_getname(ciptr->fd,&netbuf,LOCALNAME) < 0 )
	{
	PRMSG(1,"TRANS(TLIGetAddr): t_getname(LOCALNAME) failed: %d\n",
								errno, 0,0 );
	return -1;
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

if( ciptr->addr )
	free(ciptr->addr);

if( (ciptr->addr=(char *)malloc(netbuf.len)) == NULL )
        {
        PRMSG(1, "TRANS(TLIGetAddr): Can't allocate space for the addr\n",
									0,0,0);
        return -1;
        }

ciptr->family=sockname.family;
ciptr->addrlen=netbuf.len;
memcpy(ciptr->addr,&sockname,ciptr->addrlen);

return 0;
}

/*
 * This function gets the remote address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */
static int
TRANS(TLIGetPeerAddr)(ciptr)
XtransConnInfo	*ciptr;
{
Xtransaddr	sockname;
struct netbuf	netbuf;

PRMSG(3,"TRANS(TLIGetPeerAddr)(%x)\n", ciptr, 0,0 );

netbuf.buf=(char *)&sockname;
netbuf.len=sizeof(sockname);
netbuf.maxlen=sizeof(sockname);

if( t_getname(ciptr->fd,&netbuf,REMOTENAME) < 0 )
	{
	PRMSG(1,"TRANS(TLIGetPeerAddr): t_getname(REMOTENAME) failed: %d\n",
						errno, 0,0 );
	return -1;
	}

PRMSG(4,"TRANS(TLIGetPeerAddr): got family %d len %d\n",
				sockname.family ,netbuf.len, 0 );

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

if( ciptr->peeraddr )
	free(ciptr->peeraddr);

if( (ciptr->peeraddr=(char *)malloc(netbuf.len)) == NULL )
        {
        PRMSG(1,
        "TRANS(TLIGetPeerAddr): Can't allocate space for the addr\n",
									0,0,0);
        return -1;
        }

ciptr->peeraddrlen=netbuf.len;
memcpy(ciptr->peeraddr,&sockname,ciptr->peeraddrlen);

return 0;
}

/*
 * This function will establish a local name for the transport. This function
 * do extra work for the local tli connection. It must create a sockaddr_un
 * format address so that it will look like an AF_UNIX connection to the
 * higher layer.
 *
 * This function will only be called by the OPENC?TSClient() functions since
 * the local address is set up in the CreateListner() for the server ends.
 */

static int
TRANS(TLITLIBindLocal)(fd,family,port)
int	fd;
int	family;
char	*port;
{
struct	sockaddr_un	*sunaddr;
struct	t_bind	*req=NULL;

PRMSG(2, "TRANS(TLITLIBindLocal)(%d,%d,%s)\n", fd, family, port);

if( family == AF_UNIX )
	{
	if( (req=(struct t_bind *)t_alloc(fd,T_BIND,T_OPT|T_UDATA)) == NULL )
		{
		PRMSG(1,
		"TRANS(TLITLIBindLocal)() failed to allocate a t_bind\n",
								0,0,0 );
		return -1;
		}

	if( (sunaddr=(struct sockaddr_un *)
				malloc(sizeof(struct sockaddr_un))) == NULL )
		{
		PRMSG(1,
		"TRANS(TLITLIBindLocal): failed to allocate a sockaddr_un\n",
								0,0,0 );
		t_free(req,T_BIND);
		return -1;
		}

	sunaddr->sun_family=AF_UNIX;

#ifdef nuke
	if( *port == '/' ) { /* A full pathname */
		(void) strcpy(sunaddr->sun_path, port);
		} else {
		(void) sprintf(sunaddr->sun_path,"%s%s", TLINODENAME, port );
		}
#endif /*NUKE*/

	(void) sprintf(sunaddr->sun_path,"%s%d",
				TLINODENAME, getpid()^time(NULL) );

	PRMSG(4, "TRANS(TLITLIBindLocal): binding to %s\n",
						sunaddr->sun_path, 0,0);

	req->addr.buf=(char *)sunaddr;
	req->addr.len=sizeof(*sunaddr);
	req->addr.maxlen=sizeof(*sunaddr);
	}

if( t_bind(fd, req, NULL) < 0 )
	{
	PRMSG(1,
		"TRANS(TLIBindLocal): Unable to bind TLI device to %s\n",
		port, 0,0 );
	return -1;
	}
return 0;
}

static XtransConnInfo  *
TRANS(TLIOpen)(device)
char	*device;
{
XtransConnInfo	*ciptr;

PRMSG(3,"TRANS(TLIOpen)(%s)\n", device, 0,0 );

if( (ciptr=(XtransConnInfo  *)calloc(1,sizeof(XtransConnInfo))) == NULL )
	{
	PRMSG(1, "TRANS(TLIOpen): calloc failed\n", 0,0,0 );
	return NULL;
	}

if( (ciptr->fd=t_open( device, O_RDWR, NULL )) < 0 )
	{
	PRMSG(1, "TRANS(TLIOpen): t_open failed for %s\n", device, 0,0 );
	return NULL;
	}

return ciptr;
}

static	int
TRANS(TLIAddrToNetbuf)(tlifamily, host, port, netbufp)
int		tlifamily;
char		*host;
char		*port;
struct netbuf	*netbufp;
{
struct	netconfig *netconfigp;
struct	nd_hostserv	nd_hostserv;
struct	nd_addrlist *nd_addrlistp = NULL;
void	*handlep;

PRMSG(3,"TRANS(TLIAddrToNetbuf)(%d,%s,%s)\n", tlifamily, host, port );

if( (handlep=setnetconfig()) == NULL )
	return -1;

nd_hostserv.h_host = host;
if( port && *port ) {
	nd_hostserv.h_serv = port;
	} else {
	nd_hostserv.h_serv = NULL;
	}

while( (netconfigp=getnetconfig(handlep)) != NULL )
	{
	if( strcmp(netconfigp->nc_protofmly,
		TLItrans2devtab[tlifamily].protofamily) != 0 )
		continue;
	PRMSG(5,"Trying to resolve %s.%s for %s\n",
			host, port, TLItrans2devtab[tlifamily].protofamily );
	if( netdir_getbyname(netconfigp,&nd_hostserv, &nd_addrlistp) == 0 )
		{
		/* we have at least one address to use */

		PRMSG(5, "found address for %s.%s\n", host, port, 0 );
		PRMSG(5, "%s\n",taddr2uaddr(netconfigp,nd_addrlistp->n_addrs),
									0,0 );

		memcpy(netbufp->buf,nd_addrlistp->n_addrs->buf,
						nd_addrlistp->n_addrs->len);
		netbufp->len=nd_addrlistp->n_addrs->len;
		endnetconfig(handlep);
		return 0;
		}
	}
endnetconfig(handlep);

return -1;
}

/*
 * These functions are the interface supplied in the Xtransport structure
 */
static XtransConnInfo *
TRANS(TLIOpenCOTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
XtransConnInfo	*ciptr;
int i;

PRMSG(2,"TRANS(TLIOpenCOTSClient)(%d,%s,%s)\n", protocol, host, port );

if( (i=TRANS(TLISelectFamily)(thistrans->TransName)) < 0 )
	{
	PRMSG(1,"TRANS(TLIOpenCOTSClient): Unable to determine device for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( (ciptr=TRANS(TLIOpen)(TLItrans2devtab[i].devcotsname)) == NULL )
	{
	PRMSG(1,"TRANS(TLIOpenCOTSClient): Unable to open device for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( TRANS(TLITLIBindLocal)(ciptr->fd,TLItrans2devtab[i].family,port) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIOpenCOTSClient): TRANS(TLITLIBindLocal)() failed: %d\n",
								errno, 0,0 );
	t_close(ciptr->fd);
	free(ciptr);
	return NULL;
	}
	
if( TRANS(TLIGetAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIOpenCOTSClient): TRANS(TLIGetPeerAddr)() failed: %d\n",
								errno, 0,0 );
	t_close(ciptr->fd);
	free(ciptr);
	return NULL;
	}

	/* Save the TLIFamily for later use in TLIAddrToNetbuf() lookups */
ciptr->priv=(char *)i;

return ciptr;
}

static XtransConnInfo *
TRANS(TLIOpenCOTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
XtransConnInfo	*ciptr;
int i;

PRMSG(2,"TRANS(TLIOpenCOTSServer)(%d,%s,%s)\n", protocol, host, port );

if( (i=TRANS(TLISelectFamily)(thistrans->TransName)) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIOpenCOTSServer): Unable to determine device for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( (ciptr=TRANS(TLIOpen)(TLItrans2devtab[i].devcotsname)) == NULL )
	{
	PRMSG(1,
		"TRANS(TLIOpenCOTSServer): Unable to open device for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

	/* Save the TLIFamily for later use in TLIAddrToNetbuf() lookups */

ciptr->priv=(char *)i;

return ciptr;
}

static XtransConnInfo *
TRANS(TLIOpenCLTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
XtransConnInfo	*ciptr;
int i;

PRMSG(2,"TRANS(TLIOpenCLTSClient)(%d,%s,%s)\n", protocol, host, port );

if( (i=TRANS(TLISelectFamily)(thistrans->TransName)) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIOpenCLTSClient): Unable to determine device for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( (ciptr=TRANS(TLIOpen)(TLItrans2devtab[i].devcltsname)) == NULL )
	{
	PRMSG(1,
		"TRANS(TLIOpenCLTSClient): Unable to open device for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( TRANS(TLITLIBindLocal)(ciptr->fd,TLItrans2devtab[i].family,port) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIOpenCLTSClient): TRANS(TLITLIBindLocal)() failed: %d\n",
								errno, 0,0 );
	t_close(ciptr->fd);
	free(ciptr);
	return NULL;
	}
	
if( TRANS(TLIGetAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIOpenCLTSClient): TRANS(TLIGetPeerAddr)() failed: %d\n",
								errno, 0,0 );
	t_close(ciptr->fd);
	free(ciptr);
	return NULL;
	}
	
return ciptr;
}			

static XtransConnInfo *
TRANS(TLIOpenCLTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
XtransConnInfo	*ciptr;
int i;

PRMSG(2,"TRANS(TLIOpenCLTSServer)(%d,%s,%s)\n", protocol, host, port );

if( (i=TRANS(TLISelectFamily)(thistrans->TransName)) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIOpenCLTSServer): Unable to determine device for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( (ciptr=TRANS(TLIOpen)(TLItrans2devtab[i].devcltsname)) == NULL )
	{
	PRMSG(1,
		"TRANS(TLIOpenCLTSServer): Unable to open device for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

return ciptr;
}			

static
TRANS(TLISetOption)(ciptr, fd, option, arg)
XtransConnInfo	*ciptr;
int		fd;
int		option;
int		arg;
{
PRMSG(2,"TRANS(TLISetOption)(%d,%d,%d)\n", fd, option, arg );

return -1;
}

static
TRANS(TLICreateListener)(ciptr, fd, req)
XtransConnInfo	*ciptr;
int		fd;
struct t_bind	*req;
{
struct t_bind	*ret;

PRMSG(2,"TRANS(TLICreateListener)(%x,%d,%x)\n", ciptr, fd, req );

if( (ret=(struct t_bind *)t_alloc(fd,T_BIND,T_ALL)) == NULL )
	{
	PRMSG(1, "TRANS(TLICreateListener): failed to allocate a t_bind\n",
									0,0,0 );
	return -1;
	}

if( t_bind(fd, req, ret) < 0 )
	{
	PRMSG(1, "TRANS(TLICreateListener): t_bind failed\n", 0,0,0 );
	if(req)
		t_free(req,T_BIND);
	t_free(ret,T_BIND);
	return -1;
	}

if( memcmp(req->addr.buf,ret->addr.buf,req->addr.len) != 0 )
	{
	PRMSG(1, "TRANS(TLICreateListener): unable to bind to %x\n",
							req, 0,0 );
	if(req)
		t_free(req,T_BIND);
	t_free(ret,T_BIND);
	return -1;
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

if( (ciptr->addr=(char *)malloc(ret->addr.len)) == NULL )
	{
	PRMSG(1,
	"TRANS(TLICreateListener): Unable to allocate space for the address\n",
								0,0,0 );
	if(req)
		t_free(req,T_BIND);
	t_free(ret, T_BIND);
	return -1;
	}

ciptr->addrlen=ret->addr.len;
memcpy(ciptr->addr,ret->addr.buf,ret->addr.len);

if(req)
	t_free(req,T_BIND);
t_free(ret, T_BIND);

return 0;
}

static
TRANS(TLIINETCreateListener)(ciptr, fd, port)
XtransConnInfo	*ciptr;
int		fd;
char		*port;
{
int	ret,tmpport;
#define PORTBUFSIZE     64      /* what is a real size for this? */
char    portbuf[PORTBUFSIZE];
struct t_bind	*req;

PRMSG(2,"TRANS(TLICreateListener)(%x,%d,%s)\n", ciptr, fd, port );

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

ret=sscanf(port,"%d",&tmpport);

if( ret == 1 && tmpport < 1024 ) /* IP_RESERVED */
        sprintf(portbuf,"%d", X_TCP_PORT+tmpport );
else
	strncpy(portbuf,port,PORTBUFSIZE);
port=portbuf;
#endif

if( port && *port ) {
	if( (req=(struct t_bind *)t_alloc(fd,T_BIND,T_ALL)) == NULL )
		{
		PRMSG(1,
		"TRANS(TLICreateListener): failed to allocate a t_bind\n",
									0,0,0 );
		return -1;
		}

	if(TRANS(TLIAddrToNetbuf)(ciptr->priv,HOST_SELF,port,&(req->addr)) < 0)
		{
		PRMSG(1,
		"TRANS(TLICreateListener): can't resolve name:HOST_SELF.%s\n",
							port, 0,0 );
		t_free(req,T_BIND);
		return -1;
		}

	/* Set the qlen */

	req->qlen=1;

	} else {
	req=NULL;
	}

return TRANS(TLICreateListener)(ciptr, fd, req);
}

static
TRANS(TLITLICreateListener)(ciptr, fd, port)
XtransConnInfo	*ciptr;
int		fd;
char		*port;
{
struct t_bind	*req;
struct sockaddr_un	*sunaddr;

PRMSG(2,"TRANS(TLICreateListener)(%x,%d,%s)\n", ciptr, fd, port );

if( (req=(struct t_bind *)t_alloc(fd,T_BIND,T_OPT|T_UDATA)) == NULL )
	{
	PRMSG(1,
	"TRANS(TLICreateListener): failed to allocate a t_bind\n",
								0,0,0 );
	return -1;
	}

if( (sunaddr=(struct sockaddr_un *)
			malloc(sizeof(struct sockaddr_un))) == NULL )
	{
	PRMSG(1,
	"TRANS(TLICreateListener): failed to allocate a sockaddr_un\n",
								0,0,0 );
	t_free(req,T_BIND);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
if( port && *port ) {
	if( *port == '/' ) { /* A full pathname */
		(void) strcpy(sunaddr->sun_path, port);
		} else {
		(void) sprintf(sunaddr->sun_path,"%s%s", TLINODENAME, port );
		}
	} else {
	(void) sprintf(sunaddr->sun_path,"%s%d", TLINODENAME, getpid());
	}

req->addr.buf=(char *)sunaddr;
req->addr.len=sizeof(*sunaddr);
req->addr.maxlen=sizeof(*sunaddr);

/* Set the qlen */

req->qlen=1;

return TRANS(TLICreateListener)(ciptr, fd, req);
}

static XtransConnInfo *
TRANS(TLIAccept)(ciptr, fd)
XtransConnInfo	*ciptr;
int		fd;
{
struct	t_call	*call;
XtransConnInfo	*newciptr;
int	i;

PRMSG(2,"TRANS(TLIAccept)(%x,%d)\n", ciptr, fd, 0 );

if( (call=(struct t_call *)t_alloc(fd,T_CALL,T_ALL)) == NULL )
	{
	PRMSG(1, "TRANS(TLIAccept)() failed to allocate a t_call\n", 0,0,0 );
	return NULL;
	}

if( t_listen(fd,call) < 0 )
	{
	extern char *t_errlist[];
	extern int t_errno;
	PRMSG(1, "TRANS(TLIAccept)() t_listen() failed\n", 0,0,0 );
	PRMSG(1, "%s\n", t_errlist[t_errno], 0,0 );
	t_free(call,T_CALL);
	return NULL;
	}

/*
 * Now we need to set up the new endpoint for the incoming connection.
 */

i=(int)ciptr->priv; /* Makes the next line more readable */

if( (newciptr=TRANS(TLIOpen)(TLItrans2devtab[i].devcotsname)) == NULL )
	{
	PRMSG(1, "TRANS(TLIAccept)() failed to open a new endpoint\n", 0,0,0 );
	t_free(call,T_CALL);
	return NULL;
	}

if( TRANS(TLITLIBindLocal)(newciptr->fd,TLItrans2devtab[i].family,"") < 0 )
	{
	PRMSG(1,
	"TRANS(TLIAccept): TRANS(TLITLIBindLocal)() failed: %d\n",
								errno, 0,0 );
	t_free(call,T_CALL);
	t_close(newciptr->fd);
	free(newciptr);
	return NULL;
	}
	
	
if( t_accept(fd,newciptr->fd,call) < 0 )
	{
	extern char *t_errlist[];
	extern int t_errno;
	PRMSG(1, "TRANS(TLIAccept)() t_accept() failed\n", 0,0,0 );
	PRMSG(1, "%s\n", t_errlist[t_errno], 0,0 );
	t_free(call,T_CALL);
	t_close(newciptr->fd);
	free(newciptr->addr);
	free(newciptr);
	return NULL;
	}

t_free(call,T_CALL);

if( TRANS(TLIGetAddr)(newciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIAccept): TRANS(TLIGetPeerAddr)() failed: %d\n",
						errno, 0,0 );
	t_close(newciptr->fd);
	free(newciptr);
	return NULL;
	}

if( TRANS(TLIGetPeerAddr)(newciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIAccept): TRANS(TLIGetPeerAddr)() failed: %d\n",
						errno, 0,0 );
	t_close(newciptr->fd);
	free(newciptr->addr);
	free(newciptr);
	return NULL;
	}

if( ioctl(newciptr->fd, I_POP,"timod") < 0 )
	{
	PRMSG(1, "TRANS(TLIAccept)() ioctl(I_POP, \"timod\") failed %d\n",
								errno,0,0 );
	t_close(newciptr->fd);
	free(newciptr->addr);
	free(newciptr);
	return NULL;
	}

if( ioctl(newciptr->fd, I_PUSH,"tirdwr") < 0 )
	{
	PRMSG(1, "TRANS(TLIAccept)() ioctl(I_PUSH,\"tirdwr\") failed %d\n",
								errno,0,0 );
	t_close(newciptr->fd);
	free(newciptr->addr);
	free(newciptr);
	return NULL;
	}

return newciptr;
}

static
TRANS(TLIConnect)(ciptr, fd, sndcall )
XtransConnInfo	*ciptr;
int		fd;
struct t_call	*sndcall;
{
PRMSG(2, "TRANS(TLIConnect)(%x,%d,%x)\n", ciptr, fd, sndcall);

if( t_connect(fd,sndcall,NULL) < 0 )
	{
	extern char *t_errlist[];
	extern int t_errno;
	PRMSG(1, "TRANS(TLIConnect)() t_connect() failed\n", 0,0,0 );
	PRMSG(1, "%s\n", t_errlist[t_errno], 0,0 );
	t_free(sndcall,T_CALL);
	return -1;
	}

t_free(sndcall,T_CALL);

if( ioctl(fd, I_POP,"timod") < 0 )
	{
	PRMSG(1, "TRANS(TLIConnect)() failed to pop timod\n", 0,0,0 );
	return -1;
	}

if( ioctl(fd, I_PUSH,"tirdwr") < 0 )
	{
	PRMSG(1, "TRANS(TLIConnect)() failed to push tirdwr\n", 0,0,0 );
	return -1;
	}

/*
 * Sync up the address fields of ciptr.
 */

if( TRANS(TLIGetAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIConnect): TRANS(TLIGetAddr)() failed: %d\n",
						errno, 0,0 );
	return -1;
	}

if( TRANS(TLIGetPeerAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(TLIConnect): TRANS(TLIGetPeerAddr)() failed: %d\n",
						errno, 0,0 );
	return -1;
	}

return 0;
}

static
TRANS(TLIINETConnect)(ciptr, fd, host, port)
XtransConnInfo	*ciptr;
int		fd;
char		*host;
char		*port;
{
#define PORTBUFSIZE	64	/* what is a real size for this? */
char	portbuf[PORTBUFSIZE];	
struct	t_call	*sndcall;

PRMSG(2, "TRANS(TLIINETConnect)(%s,%s)\n", host, port, 0);

#ifdef X11
/*
 * X has a well known port, that is transport dependant. It is easier
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
 {
int	tmpport=atoi(port);

if( tmpport != -1 && tmpport > 1024 ) /* IP_RESERVE */
	sprintf(portbuf,"%d", X_TCP_PORT+tmpport );
else
	strncpy(portbuf,port,PORTBUFSIZE);
 }
#else
strncpy(portbuf,port,PORTBUFSIZE);
#endif

if( (sndcall=(struct t_call *)t_alloc(fd,T_CALL,T_ALL)) == NULL )
	{
	PRMSG(1, "TRANS(TLIINETConnect)() failed to allocate a t_call\n", 0,0,0 );
	return -1;
	}

if( TRANS(TLIAddrToNetbuf)(ciptr->priv, host, portbuf, &(sndcall->addr) ) < 0 )
	{
	PRMSG(1, "TRANS(TLIINETConnect)() unable to resolve name:%s.%s\n",
					host, port, 0 );
	t_free(sndcall,T_CALL);
	return -1;
	}

return TRANS(TLIConnect)(ciptr, fd, sndcall );
}

static
TRANS(TLITLIConnect)(ciptr, fd, host, port)
XtransConnInfo	*ciptr;
int		fd;
char		*host;
char		*port;
{
struct	t_call	*sndcall;
struct	sockaddr_un	*sunaddr;

PRMSG(2, "TRANS(TLITLIConnect)(%s,%s)\n", host, port, 0);

if( (sndcall=(struct t_call *)t_alloc(fd,T_CALL,T_OPT|T_UDATA)) == NULL )
	{
	PRMSG(1, "TRANS(TLITLIConnect)() failed to allocate a t_call\n", 0,0,0 );
	return -1;
	}

if( (sunaddr=(struct sockaddr_un *)
			malloc(sizeof(struct sockaddr_un))) == NULL )
	{
	PRMSG(1,
	"TRANS(TLICreateListener): failed to allocate a sockaddr_un\n",
								0,0,0 );
	t_free(sndcall,T_CALL);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
if( *port == '/' ) { /* A full pathname */
	(void) strcpy(sunaddr->sun_path, port);
	} else {
	(void) sprintf(sunaddr->sun_path,"%s%s", TLINODENAME, port );
	}

sndcall->addr.buf=(char *)sunaddr;
sndcall->addr.len=sizeof(*sunaddr);
sndcall->addr.maxlen=sizeof(*sunaddr);

return TRANS(TLIConnect)(ciptr, fd, sndcall );
}

static
TRANS(TLIBytesReadable)(XtransConnInfo *ciptr, int fd, BytesReadable_t *pend)
{
PRMSG(2, "TRANS(TLIByteReadable)(%x,%d,%x)\n", ciptr, fd, pend );
/*
 * On a streams-based system, I_NREAD only returns the amount of data
 * in the FIRST message only, not the total amount of data waiting to be read.
 *
 * Because of this, if the size of the message is smaller that the size of
 * what is needed, the message must be read so the size of the next message
 * can be read.
 */

/* XXXSTU still need to read some data and save it for the next read call. */

return ioctl(fd, I_NREAD, (char *)pend);
}

static
TRANS(TLIRead)(XtransConnInfo *ciptr, int fd, char *buf, int size)
{
PRMSG(2, "TRANS(TLIRead)(%d,%x,%d)\n", fd, buf, size );

return read(fd,buf,size);
}

static
TRANS(TLIWrite)(XtransConnInfo *ciptr, int fd, char *buf, int size)
{
PRMSG(2, "TRANS(TLIWrite)(%d,%x,%d)\n", fd, buf, size );

return write(fd,buf,size);
}

static
TRANS(TLIReadv)(XtransConnInfo *ciptr, int fd, struct iovec *buf, int size)
{
PRMSG(2, "TRANS(TLIReadv)(%d,%x,%d)\n", fd, buf, size );

return READV(fd,buf,size);
}

static
TRANS(TLIWritev)(XtransConnInfo *ciptr, int fd, struct iovec *buf, int size)
{
PRMSG(2, "TRANS(TLIWritev)(%d,%x,%d)\n", fd, buf, size );

return WRITEV(fd,buf,size);
}

static
TRANS(TLIDisconnect)(XtransConnInfo *ciptr, int fd)
{
PRMSG(2, "TRANS(TLIDisconnect)(%x,%d)\n", ciptr, fd, 0 );

return 0;
}

static
TRANS(TLIClose)(XtransConnInfo *ciptr, int fd)
{
PRMSG(2, "TRANS(TLIClose)(%x,%d)\n", ciptr, fd, 0 );

t_unbind(fd);
t_close(fd);
}

static
TRANS(TLINameToAddr)(XtransConnInfo *ciptr, int fd /*???what else???*/ )
{
}

static
TRANS(TLIAddrToName)(XtransConnInfo *ciptr, int fd /*???what else???*/ )
{
}

Xtransport	TRANS(TLIINETFuncs) = {
	/* TLI Interface */
	"inet",
	0,
	TRANS(TLIOpenCOTSClient),
	TRANS(TLIOpenCOTSServer),
	TRANS(TLIOpenCLTSClient),
	TRANS(TLIOpenCLTSServer),
	TRANS(TLISetOption),
	TRANS(TLIINETCreateListener),
	TRANS(TLIAccept),
	TRANS(TLIINETConnect),
	TRANS(TLIBytesReadable),
	TRANS(TLIRead),
	TRANS(TLIWrite),
	TRANS(TLIReadv),
	TRANS(TLIWritev),
	TRANS(TLIDisconnect),
	TRANS(TLIClose),
	TRANS(TLINameToAddr),
	TRANS(TLIAddrToName),
};

Xtransport	TRANS(TLITCPFuncs) = {
	/* TLI Interface */
	"tcp",
	TRANS_ALIAS,
	TRANS(TLIOpenCOTSClient),
	TRANS(TLIOpenCOTSServer),
	TRANS(TLIOpenCLTSClient),
	TRANS(TLIOpenCLTSServer),
	TRANS(TLISetOption),
	TRANS(TLIINETCreateListener),
	TRANS(TLIAccept),
	TRANS(TLIINETConnect),
	TRANS(TLIBytesReadable),
	TRANS(TLIRead),
	TRANS(TLIWrite),
	TRANS(TLIReadv),
	TRANS(TLIWritev),
	TRANS(TLIDisconnect),
	TRANS(TLIClose),
	TRANS(TLINameToAddr),
	TRANS(TLIAddrToName),
};

Xtransport	TRANS(TLITLIFuncs) = {
	/* TLI Interface */
	"tli",
	0,
	TRANS(TLIOpenCOTSClient),
	TRANS(TLIOpenCOTSServer),
	TRANS(TLIOpenCLTSClient),
	TRANS(TLIOpenCLTSServer),
	TRANS(TLISetOption),
	TRANS(TLITLICreateListener),
	TRANS(TLIAccept),
	TRANS(TLITLIConnect),
	TRANS(TLIBytesReadable),
	TRANS(TLIRead),
	TRANS(TLIWrite),
	TRANS(TLIReadv),
	TRANS(TLIWritev),
	TRANS(TLIDisconnect),
	TRANS(TLIClose),
	TRANS(TLINameToAddr),
	TRANS(TLIAddrToName),
	};