#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include "Xtrans.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

FdMask	fds;
fd_set	readfds;

print_addr(title,addr,addrlen)
char		*title;
Xtransaddr	*addr;
int		addrlen;
{
fprintf(stderr,"%s address: (%d)\n", title, addrlen);

switch( addr->family )
	{
	case AF_UNIX:
		{
		struct sockaddr_un *saddr=(struct sockaddr_un *)addr;
		fprintf(stderr,"**Got AF_UNIX %s\n",
					saddr->sun_path );
		break;
		}
	case AF_INET:
		{
		struct sockaddr_in *saddr=(struct sockaddr_in *)addr;
		fprintf(stderr,"**Got AF_INET %x.%x\n",
			saddr->sin_addr, saddr->sin_port );
		break;
		}
	default:
		fprintf(stderr,"**Got unknown family %d\n", addr->family );
		break;
	}
}

main(argc,argv)
int	argc;
char	*argv[];
{
int	loop,i;
int	newfd;
char	buf[128];
int	family;
int	addrlen;
Xtransaddr	*addr;
char	*port="transtest";

if( argc > 1 )
	port=argv[1];
	
_TESTTransMakeAllCOTSServerListeners(port,&fds);

for(i=0;i<32;i++)
	{
	if( fds & (1<<i) )
		{
		_TESTTransGetMyAddr(i,&family,&addrlen,&addr);
		print_addr("Listner",addr,addrlen);
		free(addr);
		}
	}

for(loop=0;loop<30;loop++)
	{
	FD_ZERO(&readfds);
	for(i=0;i<32;i++)
		{
		if( fds & (1<<i) )
			{
			FD_SET(i, &readfds);
			fprintf(stderr,"%d, ",i);
			}
		}
	fprintf(stderr,"\n");
	select(32,&readfds,NULL,NULL,NULL);
	for(i=0;i<32;i++)
		{
		if(FD_ISSET(i,&readfds))
			{
			fprintf(stderr,"Accepting on %d\n", i );
			if( (newfd=_TESTTransAccept(i)) < 0 )
				{
				fprintf(stderr,"_TESTTransAccept(%d) failed\n",
								i );
				continue;
				}

			_TESTTransGetMyAddr(i,&family,&addrlen,&addr);
			print_addr("Server",addr,addrlen);
			_TESTTransGetPeerAddr(i,&family,&addrlen,&addr);
			print_addr("Client",addr,addrlen);

			if( _TESTTransWrite(newfd,"SERVER OK", 10 ) < 0 )
				{
				fprintf(stderr,"_TESTTransWrite() failed\n" );
				_TESTTransClose(newfd);
				continue;
				}
			if( _TESTTransRead(newfd,buf,sizeof(buf)) < 0 )
				{
				fprintf(stderr,"_TESTTransRead() failed\n" );
				_TESTTransClose(newfd);
				continue;
				}
			fprintf(stderr,"message from client: %s\n",buf );
			_TESTTransClose(newfd);
			}
		}
	}

for(i=0;i<32;i++)
	{
	if( fds & (1<<i) )
		{
		fprintf(stderr,"closing: %d\n", i );
		_TESTTransClose(i);
		}
	}
}
