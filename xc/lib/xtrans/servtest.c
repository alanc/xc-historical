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

fd_set	readfds;

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
int	loop,i;
int	count, partial;
XtransConnInfo *ciptrs, newciptr;
char	buf[128];
int	family;
int	addrlen;
Xtransaddr	*addr;
char	*port="transtest";

if( argc > 1 )
	port=argv[1];
	
if (_TESTTransMakeAllCOTSServerListeners(port,&partial, &count,&ciptrs) < 0)
    exit (1);

for(i=0;i<count;i++)
	{
	    _TESTTransGetMyAddr(ciptrs[i],&family,&addrlen,&addr);
	    print_addr("Listner",addr,addrlen);
	    free(addr);
	}

for(loop=0;loop<30;loop++)
	{
	FD_ZERO(&readfds);
	for(i=0;i<count;i++)
		{
		    int fd = _TESTTransGetConnectionNumber(ciptrs[i]);
		    FD_SET(fd, &readfds);
		    fprintf(stderr,"%d, ",fd);
		}
	fprintf(stderr,"\n");
	select(32,(int *)&readfds,NULL,NULL,NULL);
	for(i=0;i<count;i++)
		{
		int fd = _TESTTransGetConnectionNumber(ciptrs[i]);
		if(FD_ISSET(fd,&readfds))
			{
			fprintf(stderr,"Accepting on %d\n", fd );
			if( (newciptr=_TESTTransAccept(ciptrs[i])) == NULL )
				{
				fprintf(stderr,"_TESTTransAccept(%d) failed\n",
								fd );
				continue;
				}

			_TESTTransGetMyAddr(newciptr,&family,&addrlen,&addr);
			print_addr("Server",addr,addrlen);
			_TESTTransGetPeerAddr(newciptr,&family,&addrlen,&addr);
			print_addr("Client",addr,addrlen);

			if( _TESTTransWrite(newciptr,"SERVER OK", 10 ) < 0 )
				{
				fprintf(stderr,"_TESTTransWrite() failed\n" );
				_TESTTransClose(newciptr);
				continue;
				}
			if( _TESTTransRead(newciptr,buf,sizeof(buf)) < 0 )
				{
				fprintf(stderr,"_TESTTransRead() failed\n" );
				_TESTTransClose(newciptr);
				continue;
				}
			fprintf(stderr,"message from client: %s\n",buf );
			_TESTTransClose(newciptr);
			}
		}
	}

for(i=0;i<count;i++)
	{
	    fprintf(stderr,"closing: %d\n",
		_TESTTransGetConnectionNumber(ciptrs[i]));
	    _TESTTransClose(ciptrs[i]);
	}
}
