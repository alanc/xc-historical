/* $XConsortium$ */

#include <stdio.h>
#include <memory.h>
#include <X11/Xtrans.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

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
		fprintf(stderr,"**Got unknown family %d\n", family );
		break;
	}
}

main()
{
int	i;
int	family;
int	addrlen;
Xtransaddr	*addr;
int	count, partial;
XtransConnInfo *ciptrs;

/* bind to any port */
if (_TESTTransMakeAllCOTSServerListeners(NULL,&partial,&count,&ciptrs) < 0)
    exit (1);

for(i=0;i<count;i++)
	{
	    _TESTTransGetMyAddr(ciptrs[i],&family,&addrlen,&addr);
	    print_addr("Listner",addr,addrlen);
	    free(addr);
	}

for(i=0;i<count;i++)
	{
	    fprintf(stderr,"closing: %d\n",
		_TESTTransGetConnectionNumber(ciptrs[i]) );
	    _TESTTransClose(ciptrs[i]);
	}
}
