#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <X11/Xtrans.h>
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

main()
{
int	i;
int	family;
int	addrlen;
Xtransaddr	*addr;

_TESTTransMakeAllCOTSServerListeners(NULL,&fds); /* bind to any port */

for(i=0;i<32;i++)
	{
	if( fds & (1<<i) )
		{
		_TESTTransGetMyAddr(i,&family,&addrlen,&addr);
		print_addr("Listner",addr,addrlen);
		free(addr);
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
