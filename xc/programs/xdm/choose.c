/*
 * $XConsortium: choose.c,v 1.2 91/01/31 22:02:55 gildea Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
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
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * choose.c
 *
 * xdm interface to chooser program
 */

# include   "dm.h"

#ifdef XDMCP

# include	<X11/X.h>
# include	<sys/types.h>
# include	<sys/socket.h>
# include	<netinet/in.h>
# include	<sys/un.h>
# include	<netdb.h>
# include	<ctype.h>

ProcessChooserSocket (fd)
{
    int client_fd;
    char	buf[1024];
    int		len;
    XdmcpBuffer	buffer;
    ARRAY8	clientAddress;
    CARD16	connectionType;
    ARRAY8	choice;

    Debug ("Process chooser socket\n");
    len = sizeof (buf);
    client_fd = accept (fd, buf, &len);
    if (client_fd == -1)
    {
	LogError ("Cannot accept chooser connection\n");
	return;
    }
    Debug ("Accepted %d\n", client_fd);
    
    len = read (client_fd, buf, sizeof (buf));
    Debug ("Read returns %d\n", len);
    if (len > 0)
    {
    	buffer.data = (BYTE *) buf;
    	buffer.size = sizeof (buf);
    	buffer.count = len;
    	buffer.pointer = 0;
	clientAddress.data = 0;
	clientAddress.length = 0;
	choice.data = 0;
	choice.length = 0;
	if (XdmcpReadARRAY8 (&buffer, &clientAddress) &&
	    XdmcpReadCARD16 (&buffer, &connectionType) &&
	    XdmcpReadARRAY8 (&buffer, &choice))
	{
	    Debug ("Read from chooser succesfully\n");
	    RegisterIndirectChoice (&clientAddress, connectionType, &choice);
	}
	XdmcpDisposeARRAY8 (&clientAddress);
	XdmcpDisposeARRAY8 (&choice);
    }
    else
    {
	LogError ("Invalid choice response length %d\n", len);
    }

    close (client_fd);
}

FormatBytes (data, length, buf, buflen)
    char    *data;
    int	    length;
    char    *buf;
    int	    buflen;
{
    int	    i;
    static char	    HexChars[] = "0123456789abcdef";

    if (buflen < length * 2 + 1)
	return 0;
    for (i = 0; i < length; i++)
    {
	*buf++ = HexChars[(data[i] >> 4) & 0xf];
	*buf++ = HexChars[(data[i]) & 0xf];
    }
    *buf++ = '\0';
    return 1;
}

FormatARRAY8 (a, buf, buflen)
    ARRAY8Ptr	a;
    char	*buf;
    int		buflen;
{
    return FormatBytes (a->data, a->length, buf, buflen);
}

typedef struct _IndirectUsers {
    struct _IndirectUsers   *next;
    ARRAY8	client;
    CARD16	connectionType;
} IndirectUsersRec, *IndirectUsersPtr;

static IndirectUsersPtr	indirectUsers;

RememberIndirectClient (clientAddress, connectionType)
    ARRAY8Ptr	clientAddress;
    CARD16	connectionType;
{
    IndirectUsersPtr	i;

    for (i = indirectUsers; i; i = i->next)
	if (XdmcpARRAY8Equal (clientAddress, &i->client) &&
	    connectionType == i->connectionType)
	    return;
    i = (IndirectUsersPtr) malloc (sizeof (IndirectUsersRec));
    if (!XdmcpCopyARRAY8 (clientAddress, &i->client))
    {
	free ((char *) i);
	return 0;
    }
    i->connectionType = connectionType;
    i->next = indirectUsers;
    indirectUsers = i;
    return 1;
}

ForgetIndirectClient (clientAddress, connectionType)
    ARRAY8Ptr	clientAddress;
    CARD16	connectionType;
{
    IndirectUsersPtr	i, prev;

    prev = 0;
    for (i = indirectUsers; i; i = i->next)
    {
	if (XdmcpARRAY8Equal (clientAddress, &i->client) &&
	    connectionType == i->connectionType)
	{
	    if (prev)
		prev->next = i->next;
	    else
		indirectUsers = i->next;
	    XdmcpDisposeARRAY8 (&i->client);
	    free ((char *) i);
	    break;
	}
	prev = i;
    }
}

IsIndirectClient (clientAddress, connectionType)
    ARRAY8Ptr	clientAddress;
    CARD16	connectionType;
{
    IndirectUsersPtr	i;

    for (i = indirectUsers; i; i = i->next)
	if (XdmcpARRAY8Equal (clientAddress, &i->client) &&
	    connectionType == i->connectionType)
	    return 1;
    return 0;
}

FormatChooserArgument (buf, len)
    char    *buf;
    int	    len;
{
    struct sockaddr *addr;
    char	    addr_buf[1024];
    int		    addr_len = sizeof (addr_buf);
    char	    result_buf[1024];
    int		    result_len = 0;
    int		    i;

    if (GetChooserAddr ((struct sockaddr *) addr_buf, &addr_len) == -1)
    {
	Debug ("Cannot get chooser socket address\n");
	return 0;
    }
    addr = (struct sockaddr *) addr_buf;
    switch (addr->sa_family) {
    case AF_INET:
	{
	    struct sockaddr_in	*in_addr = (struct sockaddr_in *) addr;
	    ARRAY8Ptr		localAddress, getLocalAddress ();

	    addr_buf[0] = AF_INET >> 8;
	    addr_buf[1] = AF_INET & 0xFF;
	    bcopy (&in_addr->sin_port, addr_buf+2, 2);
	    localAddress = getLocalAddress ();
	    bcopy (localAddress->data, addr_buf+4, 4);
	    addr_len = 8;
	}
	break;
#ifdef AF_DECnet
    case AF_DECnet:
	break;
#endif
    default:
	Debug ("Chooser family %d isn't known\n", addr->sa_family);
	return 0;
    }

    return FormatBytes (addr_buf, addr_len, buf, len);
}

typedef struct _Choices {
    struct _Choices *next;
    ARRAY8	    client;
    CARD16	    connectionType;
    ARRAY8	    choice;
    long	    time;
} ChoiceRec, *ChoicePtr;

static ChoicePtr   choices;

ARRAY8Ptr
IndirectChoice (clientAddress, connectionType)
    ARRAY8Ptr	clientAddress;
    CARD16	connectionType;
{
    ChoicePtr	c, next, prev;
    long	now;

    now = time (0);
    prev = 0;
    for (c = choices; c; c = next)
    {
	next = c->next;
	if (now - c->time > 15)
	{
	    Debug ("Timeout choice\n");
	    if (prev)
		prev->next = next;
	    else
		choices = next;
	    XdmcpDisposeARRAY8 (&c->client);
	    XdmcpDisposeARRAY8 (&c->choice);
	    free ((char *) c);
	}
	else
	{
	    if (XdmcpARRAY8Equal (clientAddress, &c->client) &&
	    	connectionType == c->connectionType)
	    	return &c->choice;
	    prev = c;
	}
    }
    return 0;
}

int
RegisterIndirectChoice (clientAddress, connectionType, choice)
    ARRAY8Ptr	clientAddress, choice;
{
    ChoicePtr	c;
    int		insert;

    Debug ("Got indirect choice back\n");
    for (c = choices; c; c = c->next)
	if (XdmcpARRAY8Equal (clientAddress, &c->client) &&
	    connectionType == c->connectionType)
	    break;
    insert = 0;
    if (!c)
    {
	c = (ChoicePtr) malloc (sizeof (ChoiceRec));
	insert = 1;
	if (!c)
	    return 0;
	c->connectionType = connectionType;
    	if (!XdmcpCopyARRAY8 (clientAddress, &c->client))
    	{
	    free ((char *) c);
	    return 0;
    	}
    }
    else
    {
	XdmcpDisposeARRAY8 (&c->choice);
    }
    if (!XdmcpCopyARRAY8 (choice, &c->choice))
    {
	XdmcpDisposeARRAY8 (&c->client);
	free ((char *) c);
	return 0;
    }
    if (insert)
    {
	c->next = choices;
	choices = c;
    }
    c->time = time (0);
    return 1;
}

RemoveIndirectChoice (clientAddress, connectionType)
    ARRAY8Ptr	clientAddress;
    CARD16	connectionType;
{
    ChoicePtr	c, prev;

    prev = 0;
    for (c = choices; c; c = c->next)
    {
	if (XdmcpARRAY8Equal (clientAddress, &c->client) &&
	    connectionType == c->connectionType)
	{
	    if (prev)
		prev->next = c->next;
	    else
		choices = c->next;
	    XdmcpDisposeARRAY8 (&c->client);
	    XdmcpDisposeARRAY8 (&c->choice);
	    free ((char *) c);
	    return;
	}
	prev = c;
    }
}

AddChooserHost (connectionType, addr, closure)
    CARD16	connectionType;
    ARRAY8Ptr	addr;
    char	*closure;
{
    char	***argp, **parseArgs();
    char	hostbuf[1024];

    argp = (char ***) closure;
    if (addr->length == strlen ("BROADCAST") &&
	!strncmp ((char *)addr->data, "BROADCAST", addr->length))
    {
	*argp = parseArgs (*argp, "BROADCAST");
    }
    else if (FormatARRAY8 (addr, hostbuf, sizeof (hostbuf)))
    {
	*argp = parseArgs (*argp, hostbuf);
    }
}

RunChooser (d)
    struct display  *d;
{
    char    **args, **parseArgs(), **defaultEnv(), **setEnv();
    char    buf[1024];
    char    **env;

    Debug ("RunChooser %s\n", d->name);
    SetTitle (d->name, "chooser", (char *) 0);
    LoadXloginResources (d);
    args = parseArgs ((char **) 0, d->chooser);
    strcpy (buf, "-xdmaddress ");
    if (FormatChooserArgument (buf + strlen (buf), sizeof (buf) - strlen (buf)))
	args = parseArgs (args, buf);
    strcpy (buf, "-clientaddress ");
    if (FormatARRAY8 (&d->clientAddr, buf + strlen (buf), sizeof (buf) - strlen (buf)))
	args = parseArgs (args, buf);
    sprintf (buf, "-connectionType %d", d->connectionType);
    args = parseArgs (args, buf);
    ForEachChooserHost (&d->clientAddr, d->connectionType, AddChooserHost,
			(char *) &args);
    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    if (d->authFile)
	env = setEnv (env, "XAUTHORITY", d->authFile);
    Debug ("Running %s\n", args[0]);
    execute (args, env);
    Debug ("Couldn't run %s\n", args[0]);
    LogError ("Cannot execute %s\n", args[0]);
    exit (REMANAGE_DISPLAY);
}

#endif /* XDMCP */
