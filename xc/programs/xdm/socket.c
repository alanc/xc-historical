/*
 * socket.c
 */

# include "dm.h"

#ifdef UDP_SOCKET
# include	<sys/types.h>
# include	<sys/socket.h>
# include	"buf.h"

int	socketFd;

fd_set	WellKnownSocketsMask;
int	WellKnownSocketsMax;

#ifndef FD_ZERO
/* typedef	struct	fd_set { int fds_bits[1]; } fd_set; */
# define FD_ZERO(fdp)	bzero ((fdp), sizeof (*(fdp)))
# define FD_SET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] |= (1 << ((f) % (sizeof (int) * 8))))
# define FD_ISSET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] & (1 << ((f) % (sizeof (int) * 8))))
#endif

#endif

CreateWellKnownSockets ()
{
#ifdef UDP_SOCKET
	struct sockaddr_in	sock_addr;

	if (request_port == 0)
		return;
	Debug ("creating socket %d\n", request_port);
	socketFd = socket (AF_INET, SOCK_DGRAM, 0);
	if (socketFd == -1) {
		LogError ("socket creation failed\n");
		return;
	}
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons ((short) request_port);
	sock_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind (socketFd, &sock_addr, sizeof (sock_addr)) == -1)
		LogError ("error binding socket address %d\n", request_port);
	else {
		WellKnownSocketsMax = socketFd;
		FD_SET (socketFd, &WellKnownSocketsMask);
	}
#endif
}

WaitForSomething ()
{
#ifdef UDP_SOCKET
	fd_set	reads;
	int	nready;

	Debug ("WaitForSomething\n");
	reads = WellKnownSocketsMask;
	nready = select (WellKnownSocketsMax + 1, &reads, 0, 0, 0);
	if (nready > 0 && FD_ISSET (socketFd, &reads))
		ProcessRequestSocket (socketFd);
#else
	pause ();
#endif
}

#ifdef UDP_SOCKET
ProcessRequestSocket (fd)
int	fd;
{
	struct display	*d;
	char	buf[4096];
	int	len;
	struct buffer	*f;
	char	from[1024];
	struct sockaddr_in	*from_in;
	int		fromlen;
	static char	poll_providers[] = POLL_PROVIDERS;
	static char	advertise[] = ADVERTISE;
	static DisplayType	acceptableTypes [] =
		{ foreign, transient, remove, unknown };

	Debug ("ProcessRequestSocket\n");
	fromlen = sizeof (from);
	len = recvfrom (fd, buf, sizeof (buf), 0, 
			(struct sockaddr *) from, &fromlen);
	if (len <= 0)
		return;
	from_in = (struct sockaddr_in *) from;		
	/*
	 * respond to broadcasts for services
	 */
	if (len == strlen (poll_providers) && !strncmp (buf, poll_providers, len))
	{
		Debug (
"acknowledging request for display manager addresses to %08x port %d\n",
ntohl (from_in->sin_addr.s_addr), ntohs (from_in->sin_port));
		sendto (fd, advertise, strlen (advertise), 0, 
			(struct sockaddr *) from, fromlen);
		return;
	}
	f = dataOpen (buf, len);
	ReadDisplay (f, acceptableTypes, (char *) from_in);
	bufClose (f);
	StartDisplays ();
}

serverMessage (d, text)
struct display	*d;
char		*text;
{
	char	buf[1024];

	sprintf (buf, "%s %s %s", d->name, d->argv[0], text);
	sendto (socketFd, buf, strlen (buf), 0, &d->addr, sizeof (d->addr));
}
#endif
