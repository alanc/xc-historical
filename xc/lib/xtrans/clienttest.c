/* $XConsortium$ */

#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#else
#include <winsock.h>
#include <X11/Xw32defs.h>
#endif
#include "Xtrans.h"

#ifndef HOSTADDR
#define	HOSTADDR {0x00,0x00,0x99,0x4e,0x11,0x10}
#endif

struct	connectioninfo	{
	char		*host;
	char		*port;
	unsigned short	family;
	unsigned char	addr[XTRANS_MAX_ADDR_LEN];
	};

struct connectioninfo	connections[] = {
	"","transtest",AF_UNIX,{0},
	THISHOST,"transtest",AF_INET,HOSTADDR,
	"inet/","transtest",AF_INET,HOSTADDR,
	"INET/"THISHOST,"transtest",AF_INET,HOSTADDR,
	"INET/153.78.17.16","transtest",AF_INET,HOSTADDR,
	"inet/"THISHOST,"transtest",AF_INET,HOSTADDR,
	"tcp/"THISHOST,"transtest",AF_INET,HOSTADDR,
#ifdef STREAMSCONN
	"tli/","transtest",AF_UNIX,HOSTADDR,
	"tli/"THISHOST,"transtest",AF_UNIX,HOSTADDR,
#endif
#ifdef UNIXCONN
	"unix/","transtest",AF_UNIX,HOSTADDR,
	"unix/"THISHOST,"transtest",AF_UNIX,HOSTADDR,
#endif
#ifdef DNETCONN
	":","transtest",AF_UNIX,{0},
	THISHOST":","transtest",AF_DECnet,{0},
	"decnet/","transtest",AF_DECnet,{0},
	"decnet/"THISHOST,"transtest",AF_DECnet,{0},
	"decnet/"THISHOST":","transtest",AF_DECnet,{0},
#endif
#ifdef LOCALCONN
	"local/","transtest",AF_UNIX,{0},
	"local/"THISHOST,"transtest",AF_UNIX,{0},
	"LOCAL/"THISHOST,"transtest",AF_UNIX,{0},
	"PTS/","transtest",AF_UNIX,{0},
	"pts/","transtest",AF_UNIX,{0},
	"NAMED/junk","transtest",AF_UNIX,{0},
	"named/junk","transtest",AF_UNIX,{0},
	"sco/"THISHOST,"transtest",AF_UNIX,{0},
	"SCO/"THISHOST,"transtest",AF_UNIX,{0},
	"isc/"THISHOST,"transtest",AF_UNIX,{0},
	"ISC/"THISHOST,"transtest",AF_UNIX,{0},
#endif /* LOCALCONN */
	};

#define NUMDISPLAYS (sizeof(connections)/sizeof(struct connectioninfo))

int		i; /* index of connections */

print_addr(title,addr,addrlen)
char		*title;
Xtransaddr	*addr;
int		addrlen;
{
int family;

fprintf(stderr,"%s address: (%d)\n", title, addrlen);

family = ((struct sockaddr *) addr)->sa_family;

switch( family )
	{
#ifdef UNIXCONN
	case AF_UNIX:
		{
		struct sockaddr_un *saddr=(struct sockaddr_un *)addr;
		fprintf(stderr,"**Got AF_UNIX %s\n",
					saddr->sun_path );
		break;
		}
#endif
	case AF_INET:
		{
		struct sockaddr_in *saddr=(struct sockaddr_in *)addr;
		fprintf(stderr,"**Got AF_INET %x.%x\n",
			saddr->sin_addr, saddr->sin_port );
		if(memcmp(&saddr->sin_addr,&(connections[i].addr[2]),4))
			fprintf(stderr,
				"%%Addr %x%x%x%x didn't match %x\n",
					connections[i].addr[2],
					connections[i].addr[3],
					connections[i].addr[4],
					connections[i].addr[5],
					saddr->sin_addr);
		break;
		}
	default:
		fprintf(stderr,"**Got unknown family %d\n", family );
		break;
	}
}

main(argc,argv)
int	argc;
char	*argv[];
{
XtransConnInfo  ciptr;
char		buf[128];
int		family;
int		addrlen;
Xtransaddr	*addr;
char	addrbuf[1024]; /* What size ??? */
char	*port="transtest";

if( argc > 1 )
	port=argv[1];

for(i=0;i<NUMDISPLAYS;i++)
	{
	fprintf(stderr,"**Trying to open connection for %s:%s\n",
				connections[i].host, port);
	sprintf(addrbuf,"%s:%s",connections[i].host, port);

	if( (ciptr=_TESTTransOpenCOTSClient(addrbuf)) == NULL ||
	   _TESTTransConnect (ciptr, addrbuf) < 0)
		{
		fprintf(stderr,"%%%%Failed to open connection for %s:%s\n",
				connections[i].host, port);
		}
	else
		{
		fprintf(stderr,"**Opened connection for %s:%s\n",
				connections[i].host, port);

		print_addr("Client", addr,addrlen );

		free(addr);

		_TESTTransGetPeerAddr(ciptr,&family, &addrlen, &addr);

		print_addr("Server", addr,addrlen );

		free(addr);
				
		/* Exchange some data with server */

		_TESTTransRead(ciptr,buf,sizeof(buf));
		fprintf(stderr,"message from server: %s\n",buf );
		sprintf(buf,"Connected to %s:%s",
				connections[i].host, port);
		_TESTTransWrite(ciptr,buf, strlen(buf)+1 );
		_TESTTransClose(ciptr);
		}
	sleep(1);
	}
}
