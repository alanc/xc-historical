/*
 * These are some utility functions created for convenience or to provide
 * an interface that is similar to an existing interface. These are built
 * only using the Transport Independant API, and have no knowledge of
 * the internal implementation.
 */

int
TRANS(MakeConnection) (host, portname, retries,
                             familyp, serveraddrlenp, serveraddrp)
char		*host;
char		*portname;
int		retries;
int		*familyp;		/* return */
int		*serveraddrlenp;	/* return */
Xtransaddr	**serveraddrp;		/* return */
{
int fd;
char	*hostonly;
char	addrbuf[1024]; /* What size ??? */

PRMSG(2,"TRANS(MakeConnection)(%s,%s,%d)\n", host, portname, retries);

sprintf(addrbuf,"%s:%s",host,portname);

/* Establish the connection */

if( (fd=TRANS(OpenCOTSClient)(addrbuf)) < 0 )
	{
	PRMSG(1, "TRANS(MakeConnection)() failed to open device\n", 0,0,0);
	return -1;
	}

if( (hostonly=strchr(host,'/')) == NULL )
	{
	hostonly=host;
	}
else
	hostonly++; /* Skip the '/' */

if( TRANS(Connect)(fd,addrbuf) < 0 )
	{
	PRMSG(1, "TRANS(MakeConnection)() failed to connect\n", 0,0,0);
	TRANS(Close)(fd);
	return -1;
	}

/* Get the addr info */

TRANS(GetMyAddr)(fd,familyp, serveraddrlenp, serveraddrp );

return fd;
}

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
 * TRANS(ConvertAddress) converts a sockaddr based address to an X authorization
 * based address. Some of this is defined as part of the ChangeHost protocol.
 * The rest is just doen in a consistent manner.
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
	case AF_INET:
		{
		struct sockaddr_in saddr = *(struct sockaddr_in *)addrp;
		*familyp=FamilyInternet;
		*addrlenp=4;
		memcpy(addrp,&saddr.sin_addr.s_addr,*addrlenp);
		break;
		}
#if defined(DNETCONN)
	case AF_DECnet:
		*familyp=FamilyDECnet;
		*addrlenp=2;
		break;
#endif /* DNETCONN */
#if defined(UNIXCONN)
	case AF_UNIX:
		{
		struct sockaddr_un saddr = *(struct sockaddr_un *)addrp;
		*familyp=FamilyLocal;
		*addrlenp=strlen(saddr.sun_path);
		memcpy(addrp,saddr.sun_path,*addrlenp);
		break;
		}
#endif /* UNIXCONN */
	default:
		PRMSG(1,"TRANS(ConvertFamily) Unknown family type %d\n",
								*familyp, 0,0 );
		return -1;
	}

return 0;
}

#ifdef X11
void
_X11TransCreateWellKnowListeners(display, fds)
char *display;
FdMask *fds;
{
char	buffer[10]; /* ???? what size ??? */
/*
 * Display will point to a string containing the number of the
 * display. This will be used to create an xserverN string to use for the
 * port.
 */

PRMSG(2,"_X11TransCreateWellKnowListeners(%s)\n",display, 0,0);

TRANS(MakeAllCOTSServerListeners)(display, fds);
return;
}
#endif
