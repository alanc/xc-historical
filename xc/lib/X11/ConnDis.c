/*
 * $XConsortium: XConnDis.c,v 1.1 89/06/14 16:33:42 jim Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * 
 * This file contains operating system dependencies.
 */

#include "copyright.h"
#define NEED_EVENTS
#ifdef USG
#define NEED_REPLIES
#endif

#include <stdio.h>
#include <X11/Xos.h>
#include "Xlibint.h"
#include <X11/Xauth.h>

#ifdef USG
/*
 * UNIX System V Release 3.2
 *
 * USG connection portions are Copyright 1988, 1989 AT&T, Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of AT&T not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  AT&T makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * AT&T DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL AT&T
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
*/

/*
 * THIS IS AN OS DEPENDENT FILE FOR SYSV BASED SYSTEMS
 */
#ifdef TLI
#include <sys/tiuser.h>
#endif

#include <ctype.h>
#include <memory.h>
#include <signal.h>
#include <sys/utsname.h>
#include "XIO.h"
#include <X11/Xproto.h>
#include <errno.h>

extern char *getenv();
/* if you change this define (for BUFFERSIZE) here remember to change it in
 * Xstreams.c as well.   (I know, I know, I know ) 
 */
#define BUFFERSIZE 2048
extern char * _XsInputBuffer[];
extern int _XsInputBuffersize[];
extern int _XsInputBufferptr[];

#ifdef DEBUG
#	define ERROR(x)		{ fprintf x; return -1; }
#else
#	define ERROR(x)		{ return -1; }
#endif


#define MAX_BUF 256

/* 
 * Attempts to connect to server, given display name. Returns file descriptor
 * (network socket) or -1 if connection fails. The expanded display name
 * of the form hostname:number.screen ("::" if DECnet) is returned in a result
 * parameter. The screen number to use is also returned.
 */
int _XConnectDisplay (display_name, expanded_name, prop_name, screen_num,
		      dpy_numlen, dpy_num, conn_family,
		      server_addrlen, server_addr)
    char *display_name;
    char *expanded_name;	/* return */
    char *prop_name;		/* return */
    int *screen_num;		/* return */
    int *dpy_numlen;		/* return */
    char **dpy_num;		/* return */
    int *conn_family;		/* return */
    int *server_addrlen;	/* return */
    char **server_addr;		/* return */ /* caller frees when done */
{
	struct utsname local;

	char		buf [MAX_BUF];
	char *		p = buf;
	char *		host = buf;
	char *		display;
	char *		screen;
	int		idisplay;
	int		iscreen;
        int		fd;
	int		server;
	char *		slave;
	unsigned char	c;
	struct iovec	v[2];
	void		(*savef)();

	/*
	 * get system name (local.sysname) and network name (local.nodename)
	 */
	if (uname (&local) < 0)
		ERROR ((stderr, "bad return from uname\n"));
	/*
	 * fill buf and check for overflow
	 */
	if ((p = memccpy (buf, display_name, '\0', MAX_BUF)) == NULL)
		ERROR ((stderr, "buf overflow\n"));
	/*
	 * find ':' in buf else error
	 */
	if ((p = memchr (buf, ':', p-buf)) == NULL)
		ERROR ((stderr, "no : in display_name\n"));
	/*
	 * get display number and validate
	 */
	*p = '\0';
	display = ++p;
	while (*p && isascii(*p) && isdigit(*p))
		++p;
	if (p == display)
		ERROR ((stderr, "no display number given\n"));
	/*
	 * get screen number if present else set to "0"
	 */
	prop_name[0] = '\0';
	switch (*p)
	{
		case '\0':
			screen = "0";
			break;
		case '.':
			*p = '\0';
			screen = ++p;
			while (*p && isascii(*p) && isdigit(*p))
				++p;
			if (*p == '.') {
			    strcpy (prop_name, p);
			    *p = '\0';
			} 
			if (*p) 
			  ERROR ((stderr, "bad char in screen number\n"));

			if (p == screen)
				screen = "0";
			break;
		default:
			ERROR ((stderr, "bad char in display number\n"));
	}
	idisplay = atoi (display);
	iscreen = atoi (screen);
	/*
	 * set host to local machine name if not set
	 */
	if (!*host)
		host = local.sysname;
	/*
	 * check for local server
	 */
	
	*server_addr = NULL;
	fd = GetConnectionType (host, idisplay, conn_family,
				server_addrlen, server_addr);
	
	if(fd < 0){
		if (*server_addr) free (*server_addr);
		return(-1);
		}
	/*
	 * set it non-blocking.  This is so we can read data when blocked
	 * for writing in the library.
	 */
	(void) fcntl(fd, F_SETFL, FNDELAY);

	/*
	 * set it to close-on-exec so that forks don't get screwed
	 */
	(void) fcntl(fd, F_SETFD, 1);

	/*
	 * Return the id if the connection succeeded. Rebuild the expanded
	 * spec and return it in the result parameter.
	 */
	strcpy (expanded_name, host);
	strcat (expanded_name, ":");
	strcat (expanded_name, display);
	strcat (expanded_name, ".");
	strcat (expanded_name, screen);
	*screen_num = iscreen;
	*dpy_numlen = strlen (display);
	*dpy_num = (char *) malloc ((*dpy_numlen) + 1);
	if (!*dpy_num) {
	    if (*server_addr) free (*server_addr);
	    return -1;
	}
	strcpy (*dpy_num, display);
	return (fd);
}

int _BytesReadable (fd, ptr)
int fd;
int * ptr;
{
	int inbuf = _XsInputBuffersize[fd] - _XsInputBufferptr[fd];
	int n;
	int flg;

        if (inbuf >= sizeof(xReply))
	{
		*ptr = inbuf;
		return (0);
	}

	if (_XsInputBufferptr[fd] > 0)
	{
		/* move tidbit to front of buffer */
		bcopy(&_XsInputBuffer[fd][_XsInputBufferptr[fd]],
		      &_XsInputBuffer[fd][0], inbuf);

		/* Adjust pointers in buffer to reflect move */
		_XsInputBuffersize[fd] = inbuf;
		_XsInputBufferptr[fd] = 0;
	}

	if (inbuf < 0)
	{
		inbuf = 0;
		_XsInputBuffersize[fd] = 0;
	}
	/* Read no more than number of bytes left in buffer */

        errno = 0;

	if(TypeOfStream[fd] == X_LOCAL_STREAM)
	   	n = read(fd, &_XsInputBuffer[fd][inbuf], BUFFERSIZE-inbuf);
	else
	   	n = t_rcv(fd, &_XsInputBuffer[fd][inbuf], BUFFERSIZE-inbuf, &flg);

	if (n > 0)
	{
		_XsInputBuffersize[fd] += n;
		*ptr = _XsInputBuffersize[fd];
		return (0);
	}
	else
	{
#ifdef EWOULDBLOCK
		if (errno == EWOULDBLOCK)
		{
			*ptr = _XsInputBuffersize[fd];
			return (0);
		}
		else
#endif
		{
			if (n == 0)
			{
				errno = EPIPE;
				return (-1);
			}
			else
			{
				if (errno != EINTR)
					return (-1);
				else
				{
					*ptr = _XsInputBuffersize[fd];
					return (0);
				}
			}
		}
	}
}


/*
 * XXX - what is this used for?
 */
int
XSelect(dpy, nfds, r_mask, w_mask, e_mask, timeout)
  Display		*dpy;
  int			nfds;
  unsigned long		*r_mask, *w_mask, *e_mask;
  struct timeval	*timeout;
{
  int	retval = 0;
  

  if ((GETBIT(r_mask, dpy->fd) &&
       (_XsInputBuffersize[ dpy->fd ] - _XsInputBufferptr[ dpy->fd ]) > 0))
    {
      BITCLEAR(r_mask, dpy->fd);
      if (ANYSET(r_mask))
	{
	  struct timeval	notime;
	  
	  notime.tv_sec = notime.tv_usec = 0;
	  if ((retval = select(nfds, r_mask, w_mask, e_mask, notime)) < 0)
	    return retval;
	}
      BITSET(r_mask, dpy->fd);
      return retval + 1;
    }
  else
    return select(nfds, r_mask, w_mask, e_mask, timeout);
} 

#else					/* else is bsd-ish system */
/*
 * 4.2bsd-based systems
 */
#include <sys/socket.h>

#ifndef hpux
#ifndef apollo			/* nest ifndefs because makedepend is broken */
#include <netinet/tcp.h>
#endif
#endif

#ifdef UNIXCONN
#include <sys/un.h>
#ifndef X_UNIX_PATH
#define X_UNIX_PATH "/tmp/.X11-unix/X"
#endif /* X_UNIX_PATH */
#endif /* UNIXCONN */
void bcopy();


/* 
 * Attempts to connect to server, given display name. Returns file descriptor
 * (network socket) or -1 if connection fails. The expanded display name
 * of the form hostname:number.screen ("::" if DECnet) is returned in a result
 * parameter. The screen number to use is also returned.
 */
int _XConnectDisplay (display_name, expanded_name, prop_name, screen_num,
		      dpy_numlen, dpy_num, conn_family,
		      server_addrlen, server_addr)
    char *display_name;
    char *expanded_name;	/* return */
    char *prop_name;		/* return */
    int *screen_num;		/* return */
    int *dpy_numlen;		/* return */
    char **dpy_num;		/* return */
    int *conn_family;		/* return */
    int *server_addrlen;	/* return */
    char **server_addr;		/* return */ /* caller frees when done */
{
	char displaybuf[256];		/* Display string buffer */	
	register char *display_ptr;	/* Display string buffer pointer */
	register char *numbuf_ptr;	/* Server number buffer pointer */
	char *screen_ptr;		/* Pointer for locating screen num */
	int display_num;		/* Display number */
	struct sockaddr_in inaddr;	/* INET socket address. */
	unsigned long hostinetaddr;	/* result of inet_addr of arpa addr */
#ifdef UNIXCONN
	struct sockaddr_un unaddr;	/* UNIX socket address. */
#endif
	struct sockaddr *addr;		/* address to connect to */
        struct hostent *host_ptr;
	int addrlen;			/* length of address */
	extern char *getenv();
	extern struct hostent *gethostbyname();
        int fd;				/* Network socket */
	char numberbuf[16];
	char *dot_ptr = NULL;		/* Pointer to . before screen num */
#ifdef DNETCONN
	int dnet = 0;
	char objname[20];
	extern int dnet_conn();
#endif
	int tmp_dpy_numlen = 0;
	char *tmp_dpy_num = NULL;
	int tmp_server_addrlen = 0;
	char *tmp_server_addr = NULL;
	int tmpfamily = -1;		/* since protocol families >= 0 */

	/* 
	 * Find the ':' seperator and extract the hostname and the
	 * display number.
	 * NOTE - if DECnet is to be used, the display name is formatted
	 * as "host::number"
	 */
	(void) strncpy(displaybuf, display_name, sizeof(displaybuf));
	if ((display_ptr = SearchString(displaybuf,':')) == NULL) return (-1);
#ifdef DNETCONN
	if (*(display_ptr + 1) == ':') {
	    dnet++;
	    *(display_ptr++) = '\0';
	}
#endif
	*(display_ptr++) = '\0';
 
	/* displaybuf now contains only a null-terminated host name, and
	 * display_ptr points to the display number.
	 * If the display number is missing there is an error. */

	if (*display_ptr == '\0') return(-1);

	tmp_server_addrlen = 0;
	tmp_server_addr = NULL;

	/*
	 * Build a string of the form <display-number>.<screen-number> in
	 * numberbuf, using ".0" as the default.
	 */
	screen_ptr = display_ptr;		/* points to #.#.propname */
	numbuf_ptr = numberbuf;			/* beginning of buffer */
	while (*screen_ptr != '\0') {
	    if (*screen_ptr == '.') {		/* either screen or prop */
		if (dot_ptr) {			/* then found prop_name */
		    screen_ptr++;
		    break;
		}
		dot_ptr = numbuf_ptr;		/* found screen_num */
		*(screen_ptr++) = '\0';
		*(numbuf_ptr++) = '.';
	    } else {
		*(numbuf_ptr++) = *(screen_ptr++);
	    }
	}

	/*
	 * If the spec doesn't include a screen number, add ".0" (or "0" if
	 * only "." is present.)
	 */
	if (dot_ptr == NULL) {			/* no screen num or prop */
	    dot_ptr = numbuf_ptr;
	    *(numbuf_ptr++) = '.';
	    *(numbuf_ptr++) = '0';
	} else {
	    if (*(numbuf_ptr - 1) == '.')
		*(numbuf_ptr++) = '0';
	}
	*numbuf_ptr = '\0';

	/*
	 * Return the screen number and property names in the result parameters
	 */
	*screen_num = atoi(dot_ptr + 1);
	strcpy (prop_name, screen_ptr);

	/*
	 * Convert the server number string to an integer.
	 */
	display_num = atoi(display_ptr);
	tmp_dpy_numlen = strlen (display_ptr);
	tmp_dpy_num = Xmalloc (tmp_dpy_numlen + 1);
	if (!tmp_dpy_num) {
	    return(-1);
	}
	strcpy (tmp_dpy_num, display_ptr);

	/*
	 * If the display name is missing, use current host.
	 */
	if (displaybuf[0] == '\0')
#ifdef DNETCONN
	    if (dnet) 
		(void) strcpy (displaybuf, "0");
            else
#endif
#ifdef UNIXCONN
		;	/* Do nothing if UNIX DOMAIN. Will be handled below. */
#else
	        (void) _XGetHostname (displaybuf, sizeof displaybuf);
#endif /* UNIXCONN else TCPCONN (assumed) */

#ifdef DNETCONN
	if (dnet) {
	    struct dn_naddr *dnaddrp, dnaddr;
	    struct nodeent *np;

	    /*
	     * build the target object name.
	     */
	    sprintf(objname, "X$X%d", display_num);
	    /*
	     * Attempt to open the DECnet connection, return -1 if fails.
	     */
	    if ((fd = dnet_conn(displaybuf, 
		   objname, SOCK_STREAM, 0, 0, 0, 0)) < 0) {
		if (tmp_dpy_num) Xfree (tmp_dpy_num);
		return(-1);	    /* errno set by dnet_conn. */
	    }

	    tmpfamily = FamilyDECnet;
	    if (dnaddrp = dnet_addr(displaybuf)) {  /* stolen from xhost */
		dnaddr = *dnaddrp;
	    } else {
		if ((np = getnodebyname (name)) == NULL) {
		    (void) close (fd);
		    if (tmp_dpy_num) Xfree (tmp_dpy_num);
		    return(-1);
		}
		dnaddr.a_len = np->n_length;
		bcopy (np->n_addr, dnaddr.a_addr, np->n_length);
	    }
	    tmp_server_addrlen = sizeof(struct dn_anaddr);
	    tmp_server_addr = Xmalloc(tmp_server_addrlen);
	    if (!tmp_server_addr) {
		(void) close (fd);
		if (tmp_dpy_num) Xfree (tmp_dpy_num);
		return(-1);
	    }
	    bcopy ((char *)&dnaddr, tmp_server_addr, tmp_server_addrlen);
	} else
#endif
	{
#ifdef UNIXCONN
	    if ((displaybuf[0] == '\0') || 
		(strcmp("unix", displaybuf) == 0)) {
		/* Connect locally using Unix domain. */
		unaddr.sun_family = AF_UNIX;
		(void) strcpy(unaddr.sun_path, X_UNIX_PATH);
		strcat(unaddr.sun_path, display_ptr);
		addr = (struct sockaddr *) &unaddr;
		addrlen = strlen(unaddr.sun_path) + 2;
		/*
		 * Open the network connection.
	 	 */
	        if ((fd = socket((int) addr->sa_family, SOCK_STREAM, 0)) < 0) {
		    if (tmp_dpy_num) Xfree (tmp_dpy_num);
		    return(-1);	    /* errno set by system call. */
		}

		/*
		 * This is a hack and is not part of the protocol
		 */
		tmpfamily = FamilyLocal;
		{
		    char tmpbuf[1024];

		    tmp_server_addrlen = _XGetHostname (tmpbuf, sizeof tmpbuf);
		    tmp_server_addr = Xmalloc (tmp_server_addrlen + 1);
		    if (!tmp_server_addr) {
			if (tmp_dpy_num) Xfree (tmp_dpy_num);
			(void) close (fd);
			return(-1);
		    }
		    strcpy (tmp_server_addr, tmpbuf);
		}
	    } else
#endif
	    {
		/* Get the statistics on the specified host. */
		hostinetaddr = inet_addr (displaybuf);
		if (hostinetaddr == -1) {
			if ((host_ptr = gethostbyname(displaybuf)) == NULL) {
				/* No such host! */
				errno = EINVAL;
				if (tmp_dpy_num) Xfree (tmp_dpy_num);
				return(-1);
			}
			/* Check the address type for an internet host. */
			if (host_ptr->h_addrtype != AF_INET) {
				/* Not an Internet host! */
				errno = EPROTOTYPE;
				if (tmp_dpy_num) Xfree (tmp_dpy_num);
				return(-1);
			}
 
			/* Set up the socket data. */
			inaddr.sin_family = host_ptr->h_addrtype;
#if defined(CRAY) && defined(OLDTCP)
			/* Only Cray UNICOS3 and UNICOS4 will define this */
			{
				long t;
				bcopy((char *)host_ptr->h_addr,
				      (char *)&t,
				      sizeof(t));
				inaddr.sin_addr = t;
			}
#else
			bcopy((char *)host_ptr->h_addr, 
			      (char *)&inaddr.sin_addr, 
			      sizeof(inaddr.sin_addr));
#endif /* CRAY and OLDTCP */
		} else {
#if defined(CRAY) && defined(OLDTCP)
			/* Only Cray UNICOS3 and UNICOS4 will define this */
			inaddr.sin_addr = hostinetaddr;
#else
			inaddr.sin_addr.s_addr = hostinetaddr;
#endif /* CRAY and OLDTCP */
			inaddr.sin_family = AF_INET;
		}
		tmpfamily = FamilyInternet;
		addr = (struct sockaddr *) &inaddr;
		addrlen = sizeof (struct sockaddr_in);
		inaddr.sin_port = display_num;
		inaddr.sin_port += X_TCP_PORT;
		inaddr.sin_port = htons(inaddr.sin_port);
		/*
		 * Open the network connection.
		 */

		if ((fd = socket((int) addr->sa_family, SOCK_STREAM, 0)) < 0) {
		    if (tmp_dpy_num) Xfree (tmp_dpy_num);
		    return(-1);	    /* errno set by system call. */
		}

		/* save address information */
		{
		    char *cp;
		    char tmpbuf[1024];
#if defined(CRAY) && defined(OLDTCP)
		    tmp_server_addrlen = sizeof(inaddr.sin_addr);
		    cp = (char *) &inaddr.sin_addr;
#else
		    tmp_server_addrlen = sizeof(inaddr.sin_addr.s_addr);
		    cp = (char *) &inaddr.sin_addr.s_addr;
#endif /* CRAY and OLDTCP */
		    if ((tmp_server_addrlen == 4) &&
			(cp[0] == 127) && (cp[1] == 0) &&
			(cp[2] == 0) && (cp[3] == 1))
		    {
			/*
			 * We are special casing the BSD hack localhost address
			 * 127.0.0.1, since this address shouldn't be copied to
			 * other machines.  So, we convert it to FamilyLocal.
			 * This is a hack and is not part of the protocol
			 */
			tmpfamily = FamilyLocal;
			tmp_server_addrlen = _XGetHostname (tmpbuf, sizeof tmpbuf);
			cp = tmpbuf;
		    }
		    tmp_server_addr = Xmalloc (tmp_server_addrlen);
		    if (!tmp_server_addr) {
			(void) close (fd);
			if (tmp_dpy_num) Xfree (tmp_dpy_num);
			return(-1);
		    }
		    bcopy (cp, tmp_server_addr, tmp_server_addrlen);
		}

		/* make sure to turn off TCP coalescence */
#ifdef TCP_NODELAY
		{
		int mi = 1;
		setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, &mi, sizeof (int));
		}
#endif
	    }
 

	    if (connect(fd, addr, addrlen) == -1) {
		(void) close (fd);
		if (tmp_dpy_num) Xfree (tmp_dpy_num);
		if (tmp_server_addr) Xfree (tmp_server_addr);
		return(-1); 	    /* errno set by system call. */
	    }
        }
	/*
	 * set it non-blocking.  This is so we can read data when blocked
	 * for writing in the library.
	 */
#ifdef FIOSNBIO
	{
	    int arg = 1;
	    ioctl(fd, FIOSNBIO, &arg);
	}
#else
	(void) fcntl(fd, F_SETFL, FNDELAY);
#endif /* FIOSNBIO */

	/*
	 * set it to close-on-exec
	 */
	(void) fcntl(fd, F_SETFD, 1);

	/*
	 * Return the id if the connection succeeded. Rebuild the expanded
	 * spec and return it in the result parameter.
	 */
	display_ptr = displaybuf-1;
	while (*(++display_ptr) != '\0')
	    ;
	*(display_ptr++) = ':';
#ifdef DNETCONN
	if (dnet)
	    *(display_ptr++) = ':';
#endif
	numbuf_ptr = numberbuf;
	while (*numbuf_ptr != '\0')
	    *(display_ptr++) = *(numbuf_ptr++);
	if (prop_name[0] != '\0') {
	    char *cp;

	    *(display_ptr++) = '.';
	    for (cp = prop_name; *cp; cp++) *(display_ptr++) = *cp;
	}
	*display_ptr = '\0';
	(void) strcpy(expanded_name, displaybuf);
	*conn_family = tmpfamily;
	*server_addrlen = tmp_server_addrlen;
	*server_addr = tmp_server_addr;
	*dpy_numlen = tmp_dpy_numlen;
	*dpy_num = tmp_dpy_num;
	return(fd);

}

#endif /* USG else bsd */






/* 
 * Disconnect from server.
 */

int _XDisconnectDisplay (server)

    int server;

{
    (void) close(server);
}

#undef NULL
#define NULL ((char *) 0)
/*
 * This is an OS dependent routine which:
 * 1) returns as soon as the connection can be written on....
 * 2) if the connection can be read, must enqueue events and handle errors,
 * until the connection is writable.
 */
_XWaitForWritable(dpy)
    Display *dpy;
{
    unsigned long r_mask[MSKCNT];
    unsigned long w_mask[MSKCNT];
    int nfound;

    CLEARBITS(r_mask);
    CLEARBITS(w_mask);

    while (1) {
	BITSET(r_mask, dpy->fd);
        BITSET(w_mask, dpy->fd);

	do {
	    nfound = select (dpy->fd + 1, r_mask, w_mask, NULL, NULL);
	    if (nfound < 0 && errno != EINTR)
		(*_XIOErrorFunction)(dpy);
	} while (nfound <= 0);

	if (ANYSET(r_mask)) {
	    char buf[BUFSIZE];
	    long pend_not_register;
	    register long pend;
	    register xEvent *ev;

	    /* find out how much data can be read */
	    if (BytesReadable(dpy->fd, (char *) &pend_not_register) < 0)
		(*_XIOErrorFunction)(dpy);
	    pend = pend_not_register;

	    /* must read at least one xEvent; if none is pending, then
	       we'll just block waiting for it */
	    if (pend < SIZEOF(xEvent)) pend = SIZEOF(xEvent);
		
	    /* but we won't read more than the max buffer size */
	    if (pend > BUFSIZE) pend = BUFSIZE;

	    /* round down to an integral number of XReps */
	    pend = (pend / SIZEOF(xEvent)) * SIZEOF(xEvent);

	    _XRead (dpy, buf, pend);

	    /* no space between comma and type or else macro will die */
	    STARTITERATE (ev,xEvent, buf, (pend > 0),
			  (pend -= SIZEOF(xEvent))) {
		if (ev->u.u.type == X_Error)
		    _XError (dpy, (xError *) ev);
		else		/* it's an event packet; enqueue it */
		    _XEnq (dpy, ev);
	    }
	    ENDITERATE
	}
	if (ANYSET(w_mask))
	    return;
    }
}


_XWaitForReadable(dpy)
  Display *dpy;
{
    unsigned long r_mask[MSKCNT];
    int result;
	
    CLEARBITS(r_mask);
    do {
	BITSET(r_mask, dpy->fd);
	result = select(dpy->fd + 1, r_mask, NULL, NULL, NULL);
	if (result == -1 && errno != EINTR) (*_XIOErrorFunction)(dpy);
    } while (result <= 0);
}


static int padlength[4] = {0, 3, 2, 1};	 /* make sure auth is multiple of 4 */

_XSendClientPrefix (dpy, client, auth_proto, auth_string)
     Display *dpy;
     xConnClientPrefix *client;		/* contains count for auth_* */
     char *auth_proto, *auth_string;	/* NOT null-terminated */
{
    int auth_length = client->nbytesAuthProto;
    int auth_strlen = client->nbytesAuthString;
    char padbuf[3];			/* for padding to 4x bytes */
    int pad;
    struct iovec iovarray[5], *iov = iovarray;
    int niov = 0;

#define add_to_iov(b,l) \
	  { iov->iov_base = (b); iov->iov_len = (l); iov++, niov++; }

    add_to_iov ((caddr_t) client, SIZEOF(xConnClientPrefix));

    /*
     * write authorization protocol name and data
     */
    if (auth_length > 0) {
	add_to_iov (auth_proto, auth_length);
	pad = padlength [auth_length & 3];
	if (pad) add_to_iov (padbuf, pad);
    }
    if (auth_strlen > 0) {
	add_to_iov (auth_string, auth_strlen);
	pad = padlength [auth_strlen & 3];
	if (pad) add_to_iov (padbuf, pad);
    }

#undef add_to_iov

    (void) WritevToServer (dpy->fd, iovarray, niov);
    return;
}
