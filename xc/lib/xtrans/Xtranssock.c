#include <ctype.h>
#ifndef WIN32

#if defined(TCPCONN) || defined(UNIXCONN)
#include <netinet/in.h>
#else
#ifdef ESIX
#include <lan/in.h>
#endif
#endif
#if defined(TCPCONN) || defined(UNIXCONN)
#include <netdb.h>
#endif

#ifdef UNIXCONN
#include <sys/un.h>
#endif

#ifdef hpux
#define NO_TCP_H
#endif /* hpux */
#ifdef MOTOROLA
#ifdef SYSV
#define NO_TCP_H
#endif /* SYSV */
#endif /* MOTOROLA */
#ifndef NO_TCP_H
#ifdef __osf__
#include <sys/param.h>
#endif /* osf */
#include <netinet/tcp.h>
#endif /* !NO_TCP_H */
#include <sys/ioctl.h>
#ifdef SVR4
#include <sys/filio.h>
#endif
#if (defined(SYSV386) && defined(SYSV)) || defined(_SEQUENT_)
#if !defined(_SEQUENT_) && !defined(ESIX)
#include <net/errno.h>
#endif /* _SEQUENT_  || ESIX */
#include <sys/stropts.h>
#endif /* SYSV386 ** SYSV || _SEQUENT_ */
#endif /* !WIN32 */

#ifdef WIN32
#define _WILLWINSOCK_
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
#define ECONNREFUSED WSAECONNREFUSED
#define EPROTOTYPE WSAEPROTOTYPE
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef EINTR
#define EINTR WSAEINTR
#endif /* WIN32 */

#if defined(SO_DONTLINGER) && defined(SO_LINGER)
#undef SO_DONTLINGER
#endif


/*
 * This is the Socket implementation of the X Transport service layer
 *
 * This file contains the implementation for both the UNIX and INET domains,
 * and can be built for either one, or both.
 *
 */

typedef struct _Sockettrans2dev {      
	char	*transname;
	int	family;
	int	devcotsname;
	int	devcltsname;
	int	protocol;
	} Sockettrans2dev;

static Sockettrans2dev Sockettrans2devtab[] = {
#ifdef TCPCONN
	{"inet",AF_INET,SOCK_STREAM,SOCK_DGRAM,0},
	{"tcp",AF_INET,SOCK_STREAM,SOCK_DGRAM,0},
#endif /* TCPCONN */
#ifdef UNIXCONN
	{"unix",AF_UNIX,SOCK_STREAM,SOCK_DGRAM,0},
#if !defined(LOCALCONN)
	{"local",AF_UNIX,SOCK_STREAM,SOCK_DGRAM,0},
#endif /* !LOCALCONN */
#endif /* UNIXCONN */
	};

#define NUMSOCKETFAMILIES (sizeof(Sockettrans2devtab)/sizeof(Sockettrans2dev))

/*
 * These are some utility function used by the real interface function below.
 */
static int
TRANS(SocketSelectFamily)(family)
char *family;
{
int     i;

PRMSG(3,"TRANS(SocketSelectFamily)(%s)\n", family, 0,0 );

for(i=0;i<NUMSOCKETFAMILIES;i++)
        {
        if( !strcmp(family,Sockettrans2devtab[i].transname) )
                return i;
        }
return -1;
}


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


/*
 * This function gets the local address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */
static int
TRANS(SocketGetAddr)(ciptr)
XtransConnInfo ciptr;
{
struct sockaddr_in sockname;
int		namelen=sizeof sockname;

PRMSG(3,"TRANS(SocketGetAddr)(%x)\n", ciptr, 0,0 );

if( getsockname(ciptr->fd,(struct sockaddr *)&sockname,&namelen) < 0 )
	{
	PRMSG(1,"TRANS(SocketGetAddr): getsockname() failed: %d\n",
	    						EGET(),0,0);
	return -1;
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

if( (ciptr->addr=(char *)malloc(namelen)) == NULL )
        {
        PRMSG(1, "TRANS(SocketGetAddr): Can't allocate space for the addr\n",
									0,0,0);
        return -1;
        }

ciptr->family=sockname.sin_family;
ciptr->addrlen=namelen;
memcpy(ciptr->addr,(char*)&sockname,ciptr->addrlen);

return 0;
}

/*
 * This function gets the remote address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */
static int
TRANS(SocketGetPeerAddr)(ciptr)
XtransConnInfo ciptr;
{
Xtransaddr	sockname;
int		namelen=sizeof(sockname);

PRMSG(3,"TRANS(SocketGetPeerAddr)(%x)\n", ciptr, 0,0 );

if( getpeername(ciptr->fd,(struct sockaddr *)&sockname,&namelen) < 0 )
	{
	PRMSG(1,"TRANS(SocketGetPeerAddr): getpeername() failed: %d\n",
						EGET(), 0,0 );
	return -1;
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

if( (ciptr->peeraddr=(char *)malloc(namelen)) == NULL )
        {
        PRMSG(1,
        "TRANS(SocketGetPeerAddr): Can't allocate space for the addr\n",
									0,0,0);
        return -1;
        }

ciptr->peeraddrlen=namelen;
memcpy(ciptr->peeraddr,&sockname,ciptr->peeraddrlen);

return 0;
}

static XtransConnInfo
TRANS(SocketOpen)(i, type)
int i;
int type;
{
XtransConnInfo	ciptr;
#ifdef WIN32
static WSADATA wsadata;
#endif /* WIN32 */

PRMSG(3,"TRANS(SocketOpen)(%d,%d)\n", i, type, 0 );

#ifdef WIN32
if (Sockettrans2devtab[i].family == AF_INET &&
    !wsadata.wVersion && WSAStartup(MAKEWORD(1,1), &wsadata))
	return NULL;
#endif /* WIN32 */

if( (ciptr=(XtransConnInfo)calloc(1,sizeof(struct _XtransConnInfo))) == NULL )
	{
	PRMSG(1, "TRANS(SocketOpen): malloc failed\n", 0,0,0 );
	return NULL;
	}

if( (ciptr->fd=socket( Sockettrans2devtab[i].family, type,
				Sockettrans2devtab[i].protocol )) < 0
#ifndef WIN32
#if (defined(X11_t) && !defined(USE_POLL)) || defined(FS_t) || defined(FONT_t)
	|| ciptr->fd >= OPEN_MAX
#endif
#endif
	) {
	PRMSG(1, "TRANS(SocketOpen): socket() failed for %s\n", Sockettrans2devtab[i].transname, 0,0 );
	free ((char *) ciptr);
	return NULL;
	}

#ifdef TCP_NODELAY
if (Sockettrans2devtab[i].family == AF_INET)
	{
	/*
	 * turn off TCP coalescence for INET sockets
	 */

	int tmp = 1;
	setsockopt (ciptr->fd, IPPROTO_TCP, TCP_NODELAY,
	    (char *)&tmp, sizeof (int));
	}
#endif

return ciptr;
}

/*
 * These functions are the interface supplied in the Xtransport structure
 */
static XtransConnInfo
TRANS(SocketOpenCOTSClient)(thistrans, protocol, host, port)
Xtransport *thistrans;
char *protocol;
char *host;
char *port;
{
XtransConnInfo	ciptr;
int	i;

PRMSG(2,"TRANS(SocketOpenCOTSClient)(%s,%s,%s)\n", protocol, host, port );

if( (i=TRANS(SocketSelectFamily)(thistrans->TransName)) < 0 )
	{
	PRMSG(1,
	   "TRANS(SocketOpenCOTSClient): Unable to determine socket type for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( (ciptr=TRANS(SocketOpen)(i,Sockettrans2devtab[i].devcotsname)) == NULL )
	{
	PRMSG(1,"TRANS(SocketOpenCOTSClient): Unable to open socket for %s\n",
						thistrans->TransName, 0,0 );
	return NULL;
	}

/* Save the Family for later use */

ciptr->priv=(char *)i;

return ciptr;
}

static XtransConnInfo
TRANS(SocketOpenCOTSServer)(thistrans, protocol, host, port)
Xtransport *thistrans;
char *protocol;
char *host;
char *port;
{
XtransConnInfo	ciptr;
int	i;

PRMSG(2,"TRANS(SocketOpenCOTSServer)(%s,%s,%s)\n", protocol, host, port );

if( (i=TRANS(SocketSelectFamily)(thistrans->TransName)) < 0 )
	{
	PRMSG(1,
	   "TRANS(SocketOpenCOTSServer): Unable to determine socket type for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( (ciptr=TRANS(SocketOpen)(i,Sockettrans2devtab[i].devcotsname)) == NULL )
	{
	PRMSG(1,"TRANS(SocketOpenCOTSServer): Unable to open socket for %s\n",
						thistrans->TransName, 0,0 );
	return NULL;
	}

#ifdef SO_REUSEADDR
/*
 * SO_REUSEADDR only applied to AF_INET
 */
if( Sockettrans2devtab[i].family == AF_INET )
	{
	int one = 1;
	setsockopt (ciptr->fd, SOL_SOCKET, SO_REUSEADDR,
					(char *) &one, sizeof (int));
	}
#endif

/* Save the Family for later use */

ciptr->priv=(char *)i;

return ciptr;
}

static XtransConnInfo
TRANS(SocketOpenCLTSClient)(thistrans, protocol, host, port)
Xtransport *thistrans;
char *protocol;
char *host;
char *port;
{
XtransConnInfo	ciptr;
int	i;

PRMSG(2,"TRANS(SocketOpenCLTSClient)(%s,%s,%s)\n", protocol, host, port );

if( (i=TRANS(SocketSelectFamily)(thistrans->TransName)) < 0 )
	{
	PRMSG(1,
	   "TRANS(SocketOpenCLTSClient): Unable to determine socket type for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( (ciptr=TRANS(SocketOpen)(i,Sockettrans2devtab[i].devcotsname)) == NULL )
	{
	PRMSG(1,"TRANS(SocketOpenCLTSClient): Unable to open socket for %s\n",
						thistrans->TransName, 0,0 );
	return NULL;
	}

/* Save the Family for later use */

ciptr->priv=(char *)i;

return ciptr;
}

static XtransConnInfo
TRANS(SocketOpenCLTSServer)(thistrans, protocol, host, port)
Xtransport *thistrans;
char *protocol;
char *host;
char *port;
{
XtransConnInfo	ciptr;
int	i;

PRMSG(2,"TRANS(SocketOpenCLTSServer)(%s,%s,%s)\n", protocol, host, port );

if( (i=TRANS(SocketSelectFamily)(thistrans->TransName)) < 0 )
	{
	PRMSG(1,
	   "TRANS(SocketOpenCLTSServer): Unable to determine socket type for %s\n",
		thistrans->TransName, 0,0 );
	return NULL;
	}

if( (ciptr=TRANS(SocketOpen)(i,Sockettrans2devtab[i].devcotsname)) == NULL )
	{
	PRMSG(1,"TRANS(SocketOpenCLTSServer): Unable to open socket for %s\n",
						thistrans->TransName, 0,0 );
	return NULL;
	}

/* Save the Family for later use */

ciptr->priv=(char *)i;

return ciptr;
}

static int
TRANS(SocketSetOption)(ciptr, option, arg)
XtransConnInfo ciptr;
int option;
int arg;
{
PRMSG(2,"TRANS(SocketSetOption)(%d,%d,%d)\n", ciptr->fd, option, arg );

return -1;
}

static int
TRANS(SocketCreateListener)(ciptr, sockname, socknamelen)
XtransConnInfo	ciptr;
struct sockaddr	*sockname;
int		socknamelen;
{
int	namelen=socknamelen;
int	fd=ciptr->fd;
int	retry;

PRMSG(3, "TRANS(SocketCreateListener)(%x,%d)\n", ciptr, fd, 0 );

if (Sockettrans2devtab[(int)ciptr->priv].family == AF_INET)
    retry = 20;
else
    retry = 0;

while (bind(fd, (struct sockaddr *)sockname, namelen) < 0 )
{
    if (retry-- == 0) {
	PRMSG(1, "TRANS(SocketCreateListener): failed to bind listener\n",
								0,0,0 );
	close(fd);
	return -1;
    }
#ifdef SO_REUSEADDR
    sleep (1);
#else
    sleep (10);
#endif /* SO_REUSEDADDR */
}

if (Sockettrans2devtab[(int)ciptr->priv].family == AF_INET) {
#ifdef SO_DONTLINGER
setsockopt (fd, SOL_SOCKET, SO_DONTLINGER, (char *) NULL, 0);
#else
#ifdef SO_LINGER
	{
	static int linger[2] = { 0, 0 };
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) linger, sizeof (linger));
	}
#endif
#endif
}

if( listen(fd, 5) < 0 )
	{
	PRMSG(1, "TRANS(SocketCreateListener): listen() failed\n", 0,0,0 );
	close(fd);
	return -1;
	}
	
/* Set a flag to indicate that this connection is a listener */

ciptr->flags=1;

return 0;
}

#ifdef TCPCONN
static int
TRANS(SocketINETCreateListener)(ciptr, port)
XtransConnInfo ciptr;
char *port;
{
struct	sockaddr_in	sockname;
int	namelen=sizeof(sockname);
int	ret;
short	tmpport;
struct	servent	*servp;
#define PORTBUFSIZE	64	/* what is a real size for this? */
char	portbuf[PORTBUFSIZE];

PRMSG(2, "TRANS(SocketINETCreateListener)(%s)\n", port, 0,0 );

#ifdef X11_t
/*
 * X has a well known port, that is transport dependent. It is easier
 * to handle it here, than try and come up with a transport independent
 * representation that can be passed in and resolved the usual way.
 *
 * The port that is passed here is really a string containing the idisplay
 * from ConnectDisplay().
 */

if (is_numeric (port))
{
    tmpport = (short) atoi (port);

    sprintf(portbuf,"%d", X_TCP_PORT+tmpport );
}
else
    strncpy(portbuf,port,PORTBUFSIZE);
port=portbuf;
#endif

if( port && *port )
	{
	/* Check to see if the port string is just a number (handles X11) */

	if( !is_numeric (port) )
		{
		if( (servp=getservbyname(port,"tcp")) == NULL)
			{
			PRMSG(1,
	"TRANS(SocketINETCreateListener): Unable to get service for %s\n",
								port, 0,0 );
			return -1;
			}

		sockname.sin_port=servp->s_port;
		}
	else
		{
		tmpport = (short) atoi (port);
		sockname.sin_port=htons(tmpport);
		}
	}
else
	sockname.sin_port=htons(0);

#ifdef BSD44SOCKETS
sockname.sin_len=sizeof(sockname);
#endif
sockname.sin_family=AF_INET;
sockname.sin_addr.s_addr=htonl(INADDR_ANY);

if( TRANS(SocketCreateListener)( ciptr,
		(struct sockaddr *)&sockname, namelen ) < 0 )
	{
	PRMSG(1,
"TRANS(SocketINETCreateListener): TRANS(SocketCreateListener)() failed\n",
						0,0,0 );
	return -1;
	}

if( TRANS(SocketGetAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(SocketINETCreateListener): TRANS(SocketGetAddr)() failed\n",
						0,0,0 );
	return -1;
	}

return 0;
}
#endif /* SOCKCONN */


#ifdef UNIXCONN

#ifdef hpux

#if defined(X11_t)
#define UNIX_PATH "/usr/spool/sockets/X11/"
#define UNIX_DIR "/usr/spool/sockets/X11"
#define OLD_UNIX_PATH "/tmp/.X11-unix/X"
#endif /* X11_t */
#if defined(XIM_t)
#define UNIX_PATH "/usr/spool/sockets/XIM/"
#define UNIX_DIR "/usr/spool/sockets/XIM"
#define OLD_UNIX_PATH "/tmp/.XIM-unix/XIM"
#endif /* XIM_t */
#if defined(FS_t) || defined(FONT_t)
#define UNIX_PATH "/usr/spool/sockets/fontserv/"
#define UNIX_DIR "/usr/spool/sockets/fontserv"
#endif /* FS_t || FONT_t */
#if defined(ICE_t)
#define UNIX_PATH "/usr/spool/sockets/ICE/"
#define UNIX_DIR "/usr/spool/sockets/ICE"
#endif /* ICE_t */
#if defined(TEST_t)
#define UNIX_PATH "/usr/spool/sockets/xtrans_test/"
#define UNIX_DIR "/usr/spool/sockets/xtrans_test"
#endif

#else /* !hpux */

#if defined(X11_t)
#define UNIX_PATH "/tmp/.X11-unix/X"
#define UNIX_DIR "/tmp/.X11-unix"
#endif /* X11_t */
#if defined(XIM_t)
#define UNIX_PATH "/tmp/.XIM-unix/XIM"
#define UNIX_DIR "/tmp/.XIM-unix"
#endif /* XIM_t */
#if defined(FS_t) || defined(FONT_t)
#define UNIX_PATH "/tmp/.font-unix/fs"
#define UNIX_DIR "/tmp/.font-unix"
#endif /* FS_t || FONT_t */
#if defined(ICE_t)
#define UNIX_PATH "/tmp/.ICE-unix/"
#define UNIX_DIR "/tmp/.ICE-unix"
#endif /* ICE_t */
#if defined(TEST_t)
#define UNIX_PATH "/tmp/.Test-unix/test"
#define UNIX_DIR "/tmp/.Test-unix"
#endif

#endif /* hpux */


static
TRANS(SocketUNIXCreateListener)(ciptr, port)
XtransConnInfo ciptr;
char *port;
{
struct	sockaddr_un	sockname;
int	namelen;
int	oldUmask;

PRMSG(2, "TRANS(SocketUNIXCreateListener)(%s)\n", port, 0,0 );

/* Make sure the directory is created */

oldUmask = umask (0);

#ifdef UNIX_DIR
    if (!mkdir (UNIX_DIR, 0777))
        chmod (UNIX_DIR, 0777);
#endif

sockname.sun_family=AF_UNIX;

if( port && *port ) {
	if( *port == '/' ) { /* a full pathname */
		sprintf(sockname.sun_path, "%s", port );
		} else {
		sprintf(sockname.sun_path, "%s%s", UNIX_PATH, port );
		}
	} else {
	sprintf(sockname.sun_path, "%s%d", UNIX_PATH, getpid() );
	}

#ifdef BSD44SOCKETS
sockname.sun_len=strlen(sockname.sun_path);
namelen = SUN_LEN(&sockname);
#else
namelen = strlen(sockname.sun_path) + sizeof(sockname.sun_family);
#endif

unlink(sockname.sun_path);

if( TRANS(SocketCreateListener)( ciptr,
		(struct sockaddr *)&sockname, namelen ) < 0 )
	{
	PRMSG(1,
"TRANS(SocketUNIXCreateListener): TRANS(SocketCreateListener)() failed\n",
						0,0,0 );
	return -1;
	}

/*
 * Now that the listener is esablished, create the addr info for
 * this connection. getpeername() doesn't work for UNIX Domain Sockets
 * on some systems (hpux at least), so we will just do it manually, instead
 * of calling TRANS(SocketGetAddr).
 */
namelen=sizeof(sockname); /* this will always make it the same size */

if( (ciptr->addr=(char *)malloc(namelen)) == NULL )
        {
        PRMSG(1,
	"TRANS(SocketUNIXCreateListener): Can't allocate space for the addr\n",
									0,0,0);
        return -1;
        }

ciptr->family=sockname.sun_family;
ciptr->addrlen=namelen;
memcpy(ciptr->addr,&sockname,ciptr->addrlen);

(void)umask(oldUmask);

return 0;
}
#endif /* UNIXCONN */


#ifdef TCPCONN
static XtransConnInfo
TRANS(SocketINETAccept)(ciptr)
XtransConnInfo ciptr;
{
XtransConnInfo	newciptr;
Xtransaddr	sockname;
int	namelen=sizeof(sockname);

PRMSG(2, "TRANS(SocketINETAccept)(%x,%d)\n", ciptr, ciptr->fd, 0 );

if( (newciptr=(XtransConnInfo)calloc(1,sizeof(struct _XtransConnInfo))) == NULL )
	{
	PRMSG(1, "TRANS(SocketINETAccept): malloc failed\n", 0,0,0 );
	return NULL;
	}

if( (newciptr->fd=accept(ciptr->fd,(struct sockaddr *)&sockname, &namelen)) < 0 )
	{
	PRMSG(1, "TRANS(SocketINETAccept): accept() failed\n", 0,0,0 );
	free(newciptr);
	return NULL;
	}

#ifdef TCP_NODELAY
        {
	/*
	 * turn off TCP coalescence for INET sockets
	 */

	int tmp = 1;
	setsockopt (newciptr->fd, IPPROTO_TCP, TCP_NODELAY,
	    (char *)&tmp, sizeof (int));
	}
#endif

/*
 * Get this address again because the transport may give a more 
 * specific address now that a connection is established.
 */
if( TRANS(SocketGetAddr)(newciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(SocketINETAccept): TRANS(SocketGetAddr)() failed:\n", 0,0,0 );
	close(newciptr->fd);
	free(newciptr);
        return NULL;
	}

if( TRANS(SocketGetPeerAddr)(newciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(SocketINETAccept): TRANS(SocketGetPeerAddr)() failed:\n",
								0,0,0 );
	close(newciptr->fd);
	if(newciptr->addr) free(newciptr->addr);
	free(newciptr);
        return NULL;
	}

return newciptr;
}
#endif /* TCPCONN */


#ifdef UNIXCONN
static XtransConnInfo
TRANS(SocketUNIXAccept)(ciptr)
XtransConnInfo ciptr;
{
XtransConnInfo	newciptr;
struct sockaddr_un	sockname;
int	namelen=sizeof(sockname);

PRMSG(2, "TRANS(SocketUNIXAccept)(%x,%d)\n", ciptr, ciptr->fd, 0 );

if( (newciptr=(XtransConnInfo)calloc(1,sizeof(struct _XtransConnInfo))) == NULL )
	{
	PRMSG(1, "TRANS(SocketUNIXAccept): malloc failed\n", 0,0,0 );
	return NULL;
	}

if( (newciptr->fd=accept(ciptr->fd,(struct sockaddr *)&sockname, &namelen)) < 0 )
	{
	PRMSG(1, "TRANS(SocketUNIXAccept): accept() failed\n", 0,0,0 );
	free(newciptr);
	return NULL;
	}

/*
 * Get the socket name and the peer name from the listener socket,
 * since this is unix domain.
 */

if( (newciptr->addr=(char *)malloc(ciptr->addrlen)) == NULL )
        {
        PRMSG(1,
        "TRANS(SocketUNIXAccept): Can't allocate space for the addr\n",
									0,0,0);
	close(newciptr->fd);
	free(newciptr);
        return NULL;
        }


newciptr->addrlen=ciptr->addrlen;
memcpy(newciptr->addr,ciptr->addr,newciptr->addrlen);

if( (newciptr->peeraddr=(char *)malloc(ciptr->addrlen)) == NULL )
        {
        PRMSG(1,
        "TRANS(SocketUNIXAccept): Can't allocate space for the addr\n",
									0,0,0);
	close(newciptr->fd);
	if(newciptr->addr) free(newciptr->addr);
	free(newciptr);
        return NULL;
        }

newciptr->peeraddrlen=ciptr->addrlen;
memcpy(newciptr->peeraddr,ciptr->addr,newciptr->addrlen);

return newciptr;
}
#endif /* UNIXCONN */


#ifdef TCPCONN
static int
TRANS(SocketINETConnect)(ciptr, host, port)
XtransConnInfo ciptr;
char *host;
char *port;
{
struct	sockaddr_in	sockname;
int			namelen=sizeof(sockname);
struct	hostent		*hostp;
struct	servent		*servp;
#define PORTBUFSIZE	64	/* what is a real size for this? */
char	portbuf[PORTBUFSIZE];
int	ret;
short	tmpport;
unsigned long tmpaddr;
char hostnamebuf[256];		/* tmp space */

PRMSG(2,"TRANS(SocketINETConnect)(%d,%s,%s)\n", ciptr->fd, host, port );

if (!host)
{
	hostnamebuf[0] = '\0';
	(void) TRANS(GetHostname) (hostnamebuf, sizeof hostnamebuf);
	host = hostnamebuf;
}

#ifdef X11_t
/*
 * X has a well known port, that is transport dependent. It is easier
 * to handle it here, than try and come up with a transport independent
 * representation that can be passed in and resolved the usual way.
 *
 * The port that is passed here is really a string containing the idisplay
 * from ConnectDisplay().
 */

if (is_numeric (port))
{
    tmpport = (short) atoi (port);

    sprintf(portbuf,"%d", X_TCP_PORT+tmpport );
}
else
#endif
strncpy(portbuf,port,PORTBUFSIZE);

/*
 * Build the socket name.
 */

#ifdef BSD44SOCKETS
sockname.sin_len=sizeof (struct sockaddr_in);
#endif
sockname.sin_family=AF_INET;

/*
 * fill in sin_addr
 */

/* check for ww.xx.yy.zz host string */

if (isascii(host[0]) && isdigit(host[0])) {
    tmpaddr=inet_addr(host); /* returns network byte order */
} else {
    tmpaddr = -1;
}

PRMSG(4,"TRANS(SocketINETConnect) inet_addr(%s) = %x\n", host,tmpaddr, 0);

if( tmpaddr == -1 )
	{
	if( (hostp=gethostbyname(host)) == NULL)
		{
		PRMSG(1,"TRANS(SocketINETConnect)() can't get address for %s\n",
								host, 0,0 );
		ESET(EINVAL);
		return TRANS_CONNECT_FAILED;
		}
	if (hostp->h_addrtype != AF_INET)  /* is IP host? */
		{
		PRMSG(1,"TRANS(SocketINETConnect)() not INET host%s\n",
								host, 0,0 );
		ESET(EPROTOTYPE);
		return TRANS_CONNECT_FAILED;
		}

#if defined(CRAY) && defined(OLDTCP)
        /* Only Cray UNICOS3 and UNICOS4 will define this */
        {
            long t;
            memcpy ((char *)&t, (char *)hostp->h_addr, sizeof(t));
            sockname.sin_addr = t;
        }
#else
        memcpy ((char *)&sockname.sin_addr, (char *)hostp->h_addr,
               sizeof(sockname.sin_addr));
#endif /* CRAY and OLDTCP */

	}
else
	{
#if defined(CRAY) && defined(OLDTCP)
	/* Only Cray UNICOS3 and UNICOS4 will define this */
	sockname.sin_addr = tmpaddr;
#else
	sockname.sin_addr.s_addr = tmpaddr;
#endif /* CRAY and OLDTCP */
	}

/*
 * fill in sin_port
 */

/* Check for number in the port string */
if( !is_numeric (portbuf) )
	{
	if( (servp=getservbyname(portbuf,"tcp")) == NULL)
		{
		PRMSG(1,"TRANS(SocketINETConnect)() can't get service for %s\n",
								portbuf, 0,0 );
		return TRANS_CONNECT_FAILED;
		}
	sockname.sin_port=servp->s_port;
	}
else
	{
	tmpport = (short) atoi (portbuf);
	sockname.sin_port=htons(tmpport);
	}

PRMSG(4,"TRANS(SocketINETConnect) sockname.sin_port=%d\n",
					ntohs(sockname.sin_port), 0,0 );

/*
 * Do the connect()
 */

if( connect(ciptr->fd,(struct sockaddr *)&sockname,namelen) < 0 )
{
#ifdef WIN32
    int olderrno = WSAGetLastError();
#else
    int olderrno = errno;
#endif

    PRMSG(1,"TRANS(SocketINETConnect)() can't connect: errno=%d\n",
	  						EGET(),0,0 );

    if (olderrno == ECONNREFUSED)
	return TRANS_TRY_CONNECT_AGAIN;
    else
	return TRANS_CONNECT_FAILED;	
}


/*
 * Sync up the address fields of ciptr.
 */

if( TRANS(SocketGetAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(SocketINETConnect): TRANS(SocketGetAddr)() failed:\n", 0,0,0 );
	return TRANS_CONNECT_FAILED;
	}

if( TRANS(SocketGetPeerAddr)(ciptr) < 0 )
	{
	PRMSG(1,
	"TRANS(SocketINETConnect): TRANS(SocketGetPeerAddr)() failed:\n",
								0,0,0 );
	return TRANS_CONNECT_FAILED;
	}

return 0;
}
#endif /* TCPCONN */

#ifdef UNIXCONN
static int
TRANS(SocketUNIXConnect)(ciptr, host, port)
XtransConnInfo ciptr;
char *host;
char *port;
{
struct	sockaddr_un	sockname;
int			namelen;

#if defined(hpux) && defined(X11_t)
struct	sockaddr_un	old_sockname;
int			old_namelen;
#endif


PRMSG(2,"TRANS(SocketUNIXConnect)(%d,%s,%s)\n", ciptr->fd, host, port );

if( !port || !*port )
	{
	PRMSG(1,"TRANS(SocketUNIXConnect): Missing port specification\n",
									0,0,0);
	return TRANS_CONNECT_FAILED;
	}

/*
 * Build the socket name.
 */

sockname.sun_family=AF_UNIX;

if( *port == '/' ) { /* a full pathname */
	sprintf(sockname.sun_path, "%s", port );
	} else {
	sprintf(sockname.sun_path, "%s%s", UNIX_PATH, port );
	}

#ifdef BSD44SOCKETS
sockname.sun_len=strlen(sockname.sun_path);
namelen = SUN_LEN(&sockname);
#else
namelen = strlen(sockname.sun_path) + sizeof(sockname.sun_family);
#endif


#if defined(hpux) && defined(X11_t)
/*
 * This is gross, but it was in Xlib
 */
old_sockname.sun_family=AF_UNIX;
if( *port == '/' ) { /* a full pathname */
	sprintf(old_sockname.sun_path, "%s", port );
	} else {
	sprintf(old_sockname.sun_path, "%s%s", OLD_UNIX_PATH, port );
	}
old_namelen = strlen(old_sockname.sun_path) + sizeof(old_sockname.sun_family);
#endif


/*
 * Do the connect()
 */

if( connect(ciptr->fd,(struct sockaddr *)&sockname, namelen) < 0 )
{
    int olderrno = errno;
    int connected = 0;

#if defined(hpux) && defined(X11_t)
    if (olderrno == ENOENT)
    {
	if (connect (ciptr->fd,
	    (struct sockaddr *)&old_sockname, old_namelen) >= 0)
	{
	    connected = 1;
	}
	else
	    olderrno = errno;
    }
#endif
    if (!connected)
    {
	errno = olderrno;
	
	PRMSG(1,"TRANS(SocketUNIXConnect)() can't connect: errno=%d\n",
	      EGET(),0,0 );

	if (olderrno == ENOENT)
	    return TRANS_TRY_CONNECT_AGAIN;
	else
	    return TRANS_CONNECT_FAILED;
    }
}

/*
 * Get the socket name and the peer name from the connect socket,
 * since this is unix domain.
 */

if( (ciptr->addr=(char *)malloc(namelen)) == NULL ||
    (ciptr->peeraddr=(char *)malloc(namelen)) == NULL)
        {
        PRMSG(1,
	"TRANS(SocketUNIXCreateListener): Can't allocate space for the addr\n",
									0,0,0);
        return TRANS_CONNECT_FAILED;
        }

ciptr->family=AF_UNIX;
ciptr->addrlen=namelen;
ciptr->peeraddrlen=namelen;
memcpy(ciptr->addr,&sockname,ciptr->addrlen);
memcpy(ciptr->peeraddr,&sockname,ciptr->peeraddrlen);

return 0;
}
#endif /* UNIXCONN */

static int
TRANS(SocketBytesReadable)(ciptr, pend)
XtransConnInfo ciptr;
BytesReadable_t *pend;
{
PRMSG(2,"TRANS(SocketBytesReadable)(%x,%d,%x)\n", ciptr, ciptr->fd, pend );

#ifdef WIN32
return ioctlsocket((SOCKET)ciptr->fd, FIONREAD, (u_long *)pend);
#else
#if (defined(SYSV386) && defined(SYSV)) || defined(_SEQUENT_)
return ioctl(ciptr->fd, I_NREAD, (char *)pend);
#else
return ioctl(ciptr->fd, FIONREAD, (char *)pend);
#endif /* SYSV386 && SYSV || _SEQUENT_ */
#endif /* WIN32 */
}

static int
TRANS(SocketRead)(ciptr, buf, size)
XtransConnInfo ciptr;
char *buf;
int size;
{
PRMSG(2,"TRANS(SocketRead)(%d,%x,%d)\n", ciptr->fd, buf, size );

#ifdef WIN32
return recv((SOCKET)ciptr->fd, buf, size, 0);
#else
return read(ciptr->fd,buf,size);
#endif /* WIN32 */
}

static int
TRANS(SocketWrite)(ciptr, buf, size)
XtransConnInfo ciptr;
char *buf;
int size;
{
PRMSG(2,"TRANS(SocketWrite)(%d,%x,%d)\n", ciptr->fd, buf, size );

#ifdef WIN32
return send((SOCKET)ciptr->fd, buf, size, 0);
#else
return write(ciptr->fd,buf,size);
#endif /* WIN32 */
}

static int
TRANS(SocketReadv)(ciptr, buf, size)
XtransConnInfo ciptr;
struct iovec *buf;
int size;
{
PRMSG(2,"TRANS(SocketReadv)(%d,%x,%d)\n", ciptr->fd, buf, size );

return READV(ciptr,buf,size);
}

static int
TRANS(SocketWritev)(ciptr, buf, size)
XtransConnInfo ciptr;
struct iovec *buf;
int size;
{
PRMSG(2,"TRANS(SocketWritev)(%d,%x,%d)\n", ciptr->fd, buf, size );

return WRITEV(ciptr,buf,size);
}

static int
TRANS(SocketDisconnect)(ciptr)
XtransConnInfo ciptr;
{
PRMSG(2,"TRANS(SocketDisconnect)(%x,%d)\n", ciptr, ciptr->fd, 0 );

return shutdown(ciptr->fd,2); /* disallow further sends and receives */
}

#ifdef TCPCONN
static int
TRANS(SocketINETClose)(ciptr)
XtransConnInfo ciptr;
{
PRMSG(2,"TRANS(SocketINETClose)(%x,%d)\n", ciptr, ciptr->fd, 0 );

return close(ciptr->fd);
}
#endif /* TCPCONN */

#ifdef UNIXCONN
static int
TRANS(SocketUNIXClose)(ciptr)
XtransConnInfo ciptr;
{
/*
 * If this is the server side, then once the socket is closed,
 * it must be unlinked to completely close it
 */
struct sockaddr_un	*sockname=(struct sockaddr_un *) ciptr->addr;
char	path[200]; /* > sizeof sun_path +1 */

PRMSG(2,"TRANS(SocketUNIXClose)(%x,%d)\n", ciptr, ciptr->fd, 0 );

close(ciptr->fd);

if( ciptr->flags
     && sockname
     && sockname->sun_family == AF_UNIX
     && sockname->sun_path[0] )
	{
	strncpy(path,sockname->sun_path,
			ciptr->addrlen-sizeof(sockname->sun_family));
	unlink(path);
	}
}
#endif /* UNIXCONN */

static int
TRANS(SocketNameToAddr)(ciptr /*???what else???*/ )
XtransConnInfo ciptr;
{
return -1;
}

static int
TRANS(SocketAddrToName)(ciptr /*???what else???*/ )
XtransConnInfo ciptr;
{
return -1;
}

#ifdef TCPCONN
Xtransport	TRANS(SocketINETFuncs) = {
	/* Socket Interface */
	"inet",
	0,
	TRANS(SocketOpenCOTSClient),
	TRANS(SocketOpenCOTSServer),
	TRANS(SocketOpenCLTSClient),
	TRANS(SocketOpenCLTSServer),
	TRANS(SocketSetOption),
	TRANS(SocketINETCreateListener),
	TRANS(SocketINETAccept),
	TRANS(SocketINETConnect),
	TRANS(SocketBytesReadable),
	TRANS(SocketRead),
	TRANS(SocketWrite),
	TRANS(SocketReadv),
	TRANS(SocketWritev),
	TRANS(SocketDisconnect),
	TRANS(SocketINETClose),
	TRANS(SocketNameToAddr),
	TRANS(SocketAddrToName),
	};

Xtransport	TRANS(SocketTCPFuncs) = {
	/* Socket Interface */
	"tcp",
	TRANS_ALIAS,
	TRANS(SocketOpenCOTSClient),
	TRANS(SocketOpenCOTSServer),
	TRANS(SocketOpenCLTSClient),
	TRANS(SocketOpenCLTSServer),
	TRANS(SocketSetOption),
	TRANS(SocketINETCreateListener),
	TRANS(SocketINETAccept),
	TRANS(SocketINETConnect),
	TRANS(SocketBytesReadable),
	TRANS(SocketRead),
	TRANS(SocketWrite),
	TRANS(SocketReadv),
	TRANS(SocketWritev),
	TRANS(SocketDisconnect),
	TRANS(SocketINETClose),
	TRANS(SocketNameToAddr),
	TRANS(SocketAddrToName),
	};
#endif /* TCPCONN */

#ifdef UNIXCONN
Xtransport	TRANS(SocketUNIXFuncs) = {
	/* Socket Interface */
	"unix",
	0,
	TRANS(SocketOpenCOTSClient),
	TRANS(SocketOpenCOTSServer),
	TRANS(SocketOpenCLTSClient),
	TRANS(SocketOpenCLTSServer),
	TRANS(SocketSetOption),
	TRANS(SocketUNIXCreateListener),
	TRANS(SocketUNIXAccept),
	TRANS(SocketUNIXConnect),
	TRANS(SocketBytesReadable),
	TRANS(SocketRead),
	TRANS(SocketWrite),
	TRANS(SocketReadv),
	TRANS(SocketWritev),
	TRANS(SocketDisconnect),
	TRANS(SocketUNIXClose),
	TRANS(SocketNameToAddr),
	TRANS(SocketAddrToName),
	};

#if !defined(LOCALCON)
Xtransport	TRANS(SocketLocalFuncs) = {
	/* Socket Interface */
	"local",
	TRANS_ALIAS,
	TRANS(SocketOpenCOTSClient),
	TRANS(SocketOpenCOTSServer),
	TRANS(SocketOpenCLTSClient),
	TRANS(SocketOpenCLTSServer),
	TRANS(SocketSetOption),
	TRANS(SocketUNIXCreateListener),
	TRANS(SocketUNIXAccept),
	TRANS(SocketUNIXConnect),
	TRANS(SocketBytesReadable),
	TRANS(SocketRead),
	TRANS(SocketWrite),
	TRANS(SocketReadv),
	TRANS(SocketWritev),
	TRANS(SocketDisconnect),
	TRANS(SocketUNIXClose),
	TRANS(SocketNameToAddr),
	TRANS(SocketAddrToName),
	};
#endif /* !LOCALCONN */
#endif /* UNIXCONN */
