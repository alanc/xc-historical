/*
 * xdm - display manager daemon
 *
 * $XConsortium: auth.c,v 1.1 88/11/23 16:59:05 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * auth.c
 *
 * maintain the authorization generation daemon
 */

# include   "dm.h"
# include   <sys/signal.h>
# include   <setjmp.h>
# include   <sys/types.h>
# include   <sys/ioctl.h>
# include   <sys/socket.h>
# include   <sys/stat.h>
#ifdef hpux
# include   <sys/utsname.h>
# include   <netdb.h>
#else
# include    <net/if.h>
#endif
#ifdef TCPCONN
# include   <netinet/in.h>
#endif
#ifdef DNETCONN
# include    <netdnet/dn.h>
# include    <netdnet/dnetdb.h>
#endif
# include    <X11/X.h>

static int  auth_pid = -1;
static int  to_auth, from_auth;
static FILE *from_auth_file;

InitAuthorization ()
{
    int	    pipein[2], pipeout[2];
    char    **argv, **parseArgs ();

    Debug ("InitAuthorization\n");
    if (authGen == 0 || authGen[0] == '\0')
	return;
    if (pipe (pipein) == -1)
	return;
    if (pipe (pipeout) == -1) {
	close (pipein[0]);
	close (pipein[1]);
	return;
    }
    switch (auth_pid = fork ()) {
    case 0:
	Debug ("starting authGen: %s\n", authGen);
	argv = parseArgs ((char **) 0, authGen);
	close (pipein[1]);
	if (pipein[0] != 0) {
	    dup2 (pipein[0], 0);
	    close (pipein[0]);
	}
	close (pipeout[0]);
	if (pipeout[1] != 1) {
	    dup2 (pipeout[1], 1);
	    close (pipeout[1]);
	}
	execv (argv[0], argv);
	exit (1);
    case -1:
	close (pipein[0]);
	close (pipein[1]);
	close (pipeout[0]);
	close (pipeout[1]);
	return;
    default:
	close (pipein[0]);
	to_auth=pipein[1];
	close (pipeout[1]);
	from_auth=pipeout[0];
	from_auth_file = fdopen (from_auth, "r");
	break;
    }
}

static jmp_buf	authAbort;
static int	(*oldpipe)();

static
abortAuth ()
{
    longjmp (authAbort);
}

Xauth *
GenerateAuthorization (timeout)
{
    Xauth   *ret;

    Debug ("GenerateAuthorization %d\n", timeout);
    if (auth_pid == -1)
	return 0;
    oldpipe = (int (*)()) signal (SIGPIPE, SIG_IGN);
    if (setjmp (authAbort)) {
	Debug ("Authorization timeout\n");
	ret = 0;
    } else {
        signal (SIGALRM, abortAuth);
	alarm (timeout);
	Debug ("writing byte\n");
        if (write (to_auth, "\n", 1) != 1) {
	    Debug ("Write failed\n");
	    ret = 0;
	} else {
	    Debug ("reading authorization\n");
	    ret = XauReadAuth (from_auth_file);
	    Debug ("Got 0x%x (%d %*.*s)\n", ret,
	    ret->name_length, ret->name_length, ret->name_length, ret->name);
	}
	alarm (0);
    }
    signal (SIGALRM, SIG_DFL);
    signal (SIGPIPE, oldpipe);
    Debug ("Done generate\n");
    return ret;
}

SetServerAuthorization (d)
struct display	*d;
{
	Xauth	*auth, *GenerateAuthorization ();
	FILE	*auth_file;
	int	ret = 1;
	int	mask;

	Debug ("SetServerAuthorization\n");
	if (d->authorization) {
		XauDisposeAuth (d->authorization);
		d->authorization = 0;
	}
	if (!d->authorize || !d->authFile || !d->authFile[0])
		return 0;
	auth = GenerateAuthorization (d->openTimeout);
	if (!auth) {
		LogError ("Authorization generation failed for %s\n",
				d->name);
		return 0;
	}
	mask = umask (0077);
	(void) unlink (d->authFile);
	auth_file = fopen (d->authFile, "w");
	umask (mask);
	if (!auth_file) {
		LogError ("Cannot open server authorization file %s\n",
				d->authFile);
		XauDisposeAuth (auth);
		return 0;
	}
	printf ("File: %s auth: %x\n", d->authFile, auth);
	if (!XauWriteAuth (auth_file, auth) || fflush (auth_file) == EOF) {
		LogError ("Cannot write server authorization file %s\n",
				d->authFile);
		fclose (auth_file);
		XauDisposeAuth (auth);
		return 0;
	}
	XSetAuthorization (auth->name, (int) auth->name_length,
			   auth->data, (int) auth->data_length);
	d->authorization = auth;
	fclose (auth_file);
	Debug ("Success\n");
	return 1;
}

static
openFiles (name, new_name, oldp, newp)
char	*name, *new_name;
FILE	**oldp, **newp;
{
	int	mask;

	strcpy (new_name, name);
	strcat (new_name, "-n");
	mask = umask (0077);
	(void) unlink (new_name);
	*newp = fopen (new_name, "w");
	(void) umask (mask);
	if (!*newp) {
		Debug ("can't open new file %s\n", new_name);
		return 0;
	}
	*oldp = fopen (name, "r");
	Debug ("opens succeeded %s %s\n", name, new_name);
	return 1;
}

binaryEqual (a, b, len)
char	*a, *b;
unsigned short	len;
{
	while (len-- > 0)
		if (*a++ != *b++)
			return 0;
	return 1;
}

dumpBytes (len, data)
char	*data;
{
	int	i;

	Debug ("%d: ", len);
	for (i = 0; i < len; i++)
		Debug ("%02x ", data[i] & 0377);
	Debug ("\n");
}

dumpAuth (auth)
Xauth	*auth;
{
	Debug ("family: %d\n", auth->family);
	Debug ("addr:   ");
	dumpBytes (auth->address_length, auth->address);
	Debug ("number: ");
	dumpBytes (auth->number_length, auth->number);
	Debug ("name:   ");
	dumpBytes (auth->name_length, auth->name);
	Debug ("data:   ");
	dumpBytes (auth->data_length, auth->data);
}

struct addrList {
	char	family;
	short	address_length;
	char	*address;
	short	number_length;
	char	*number;
	struct addrList	*next;
};

static struct addrList	*addrs;

initAddrs ()
{
	addrs = 0;
}

doneAddrs ()
{
	struct addrList	*a, *n;
	for (a = addrs; a; a = n) {
		n = a->next;
		if (a->address)
			free (a->address);
		if (a->number)
			free (a->number);
		free (a);
	}
}

saveAddr (family, address_length, address, number_length, number)
char	family;
char	*address, *number;
{
	struct addrList	*new;
	char		*malloc ();

	if (checkAddr (family, address_length, address, number_length, number))
		return;
	new = (struct addrList *) malloc (sizeof (struct addrList));
	if (!new)
		return;
	if ((new->address_length = address_length) > 0) {
		new->address = malloc (address_length);
		if (!new->address) {
			free (new);
			return;
		}
		bcopy (address, new->address, address_length);
	} else
		new->address = 0;
	if ((new->number_length = number_length) > 0) {
		new->number = malloc (number_length);
		if (!new->number) {
			free (new->address);
			free (new);
			return;
		}
		bcopy (number, new->number, number_length);
	} else
		new->number = 0;
	new->family = family;
	new->next = addrs;
	addrs = new;
}

checkAddr (family, address_length, address, number_length, number)
char	family;
char	*address, *number;
{
	struct addrList	*a;

	for (a = addrs; a; a = a->next) {
		if (a->family == family &&
		    a->address_length == address_length &&
 		    binaryEqual (a->address, address, address_length) &&
		    a->number_length == number_length &&
 		    binaryEqual (a->number, number, number_length))
		{
			return 1;
		}
	}
	return 0;
}

writeAuth (file, auth)
FILE	*file;
Xauth	*auth;
{
	saveAddr (auth->family, auth->address_length, auth->address,
				auth->number_length,  auth->number);
	XauWriteAuth (file, auth);
}

writeAddr (family, addr_length, addr, file, auth)
int	family;
int	addr_length;
char	*addr;
FILE	*file;
Xauth	*auth;
{
	int	i;

	Debug ("writeAddr ");
	dumpAuth (auth);
	auth->family = (char) family;
	auth->address_length = addr_length;
	auth->address = addr;
	writeAuth (file, auth);
}

DefineLocal (file, auth)
{
	char	displayname[100];

	/* stolen from xinit.c */
#ifdef hpux
	/* Why not use gethostname()?  Well, at least on my system, I've had to
	 * make an ugly kernel patch to get a name longer than 8 characters, and
	 * uname() lets me access to the whole string (it smashes release, you
	 * see), whereas gethostname() kindly truncates it for me.
	 */
	{
	struct utsname name;

	uname(&name);
	strcpy(displayname, name.nodename);
	}
#else
	gethostname(displayname, sizeof(displayname));
#endif
	writeAddr (FamilyLocal, strlen (displayname), displayname, file, auth);
}

/* code stolen from server/os/4.2bsd/access.c */

ConvertAddr (saddr, len, addr)
    register struct sockaddr	*saddr;
    int				*len;
    char			**addr;
{
    if (len == 0)
        return (0);
    switch (saddr->sa_family)
    {
      case AF_UNSPEC:
#ifndef hpux
      case AF_UNIX:
#endif
        return (0);
#ifdef TCPCONN
      case AF_INET:
        *len = sizeof (struct in_addr);
        *addr = (char *) &(((struct sockaddr_in *) saddr)->sin_addr);
        return (AF_INET);
#else
	break;
#endif

#ifdef DNETCONN
      case AF_DECnet:
        *len = sizeof (struct dn_naddr);
        *addr = (char *) &(((struct sockaddr_dn *) saddr)->sdn_add);
        return (AF_DECnet);
#else
	break;
#endif
      default:
        break;
    }
    return (-1);
}

#if defined (TCPCONN) || defined (DNETCONN)
#ifdef hpux
/* Define this host for access control.  Find all the hosts the OS knows about 
 * for this fd and add them to the selfhosts list.
 * HPUX version - hpux does not have SIOCGIFCONF ioctl;
 */
DefineSelf (fd, file, auth)
    int fd;
{
    register int n;
    int	len;
    caddr_t	addr;
    int		family;

    struct utsname name;
    register struct hostent  *hp;

    union {
	struct  sockaddr   sa;
	struct  sockaddr_in  in;
    } saddr;
	
    struct	sockaddr_in	*inetaddr;

    /* Why not use gethostname()?  Well, at least on my system, I've had to
     * make an ugly kernel patch to get a name longer than 8 characters, and
     * uname() lets me access to the whole string (it smashes release, you
     * see), whereas gethostname() kindly truncates it for me.
     */
    uname(&name);
    hp = gethostbyname (name.nodename);
    if (hp != NULL) {
	saddr.sa.sa_family = hp->h_addrtype;
	inetaddr = (struct sockaddr_in *) (&(saddr.sa));
	bcopy ( hp->h_addr, &(inetaddr->sin_addr), hp->h_length);
	family = ConvertAddr ( &(saddr.sa), &len, &addr);
	if ( family > 0) {
	    writeAddr (FamilyInternet, sizeof (inetaddr->sin_addr),
			(char *) (&inetaddr->sin_addr), file, auth);
	}
    }
}

#else

/* Define this host for access control.  Find all the hosts the OS knows about 
 * for this fd and add them to the selfhosts list.
 */
DefineSelf (fd, file, auth)
    int fd;
    FILE	*file;
    Xauth	*auth;
{
    char		buf[2048];
    struct ifconf	ifc;
    register int	n;
    int 		len;
    char 		*addr;
    int 		family;
    register struct ifreq *ifr;
    
    ifc.ifc_len = sizeof (buf);
    ifc.ifc_buf = buf;
    if (ioctl (fd, (int) SIOCGIFCONF, (char *) &ifc) < 0)
        LogError ("Trouble getting network interface configuration");
    for (ifr = ifc.ifc_req, n = ifc.ifc_len / sizeof (struct ifreq); --n >= 0;
     ifr++)
    {
#ifdef DNETCONN
	/*
	 * this is ugly but SIOCGIFCONF returns decnet addresses in
	 * a different form from other decnet calls
	 */
	if (ifr->ifr_addr.sa_family == AF_DECnet) {
		len = sizeof (struct dn_naddr);
		addr = (char *)ifr->ifr_addr.sa_data;
		family = FamilyDECnet;
	} else
#endif /* DNETCONN */
	{
        	if (ConvertAddr (&ifr->ifr_addr, &len, &addr) <= 0)
	    	    continue;
		/*
		 * don't write out 'localhost' entries, as
		 * they may conflict with other local entries.
		 * DefineLocal will always be called to add
		 * the local entry anyway, so this one can
		 * be tossed.
		 */
		if (len == 4 &&
		    addr[0] == 127 && addr[1] == 0 &&
		    addr[2] == 0 && addr[3] == 1)
 		{
			Debug ("Skipping localhost address\n");
			continue;
		}
		family = FamilyInternet;
	}
	Debug ("DefineSelf: write network address, length %d\n", len);
	writeAddr (family, len, addr, file, auth);
    }
}
#endif hpux
#endif

writeLocalAuth (file, auth, name)
FILE	*file;
Xauth	*auth;
char	*name;
{
	int	fd;
	char	*colon, *malloc ();

	colon = rindex (name, ':');
	if (colon) {
		++colon;
		auth->number_length = strlen (colon);
		auth->number = malloc (auth->number_length + 1);
		if (auth->number) {
			strcpy (auth->number, colon);
		} else {
			auth->number_length = 0;
		}
	}
	Debug ("writeLocalAuth\n");
#ifdef TCPCONN
	fd = socket (AF_INET, SOCK_STREAM, 0);
	DefineSelf (fd, file, auth);
	close (fd);
#endif
#ifdef DNETCONN
	fd = socket (AF_DECnet, SOCK_STREAM, 0);
	DefineSelf (fd, file, auth);
	close (fd);
#endif
	DefineLocal (file, auth);
}

SetUserAuthorization (d, verify)
struct display		*d;
struct verify_info	*verify;
{
	FILE	*old, *new;
	char	home_name[1024], backup_name[1024], new_name[1024];
	char	*name;
	char	*home;
	char	*envname = 0;
	int	lockStatus;
	Xauth	*entry, *auth;
	int	setenv;
	char	**setEnv (), *getEnv ();
	struct stat	statb;

	Debug ("SetUserAuthorization\n");
	if (auth = d->authorization) {
		home = getEnv (verify->userEnviron, "HOME");
		lockStatus = LOCK_ERROR;
		if (home) {
			sprintf (home_name, "%s/.Xauthority", home);
			Debug ("XauLockAuth %s\n", home_name);
			lockStatus = XauLockAuth (home_name, 1, 2, 10);
			Debug ("Lock is %d\n", lockStatus);
			if (lockStatus == LOCK_SUCCESS) {
				if (openFiles (home_name, new_name, &old, &new)) {
					name = home_name;
					setenv = 0;
				} else {
					Debug ("openFiles failed\n");
					XauUnlockAuth (home_name);
					lockStatus = LOCK_ERROR;
				}	
			}
		}
		if (lockStatus != LOCK_SUCCESS) {
			sprintf (backup_name, "%s/.XauthXXXXXX", d->userAuthDir);
			mktemp (backup_name);
			lockStatus = XauLockAuth (backup_name, 1, 2, 10);
			Debug ("backup lock is %d\n", lockStatus);
			if (lockStatus == LOCK_SUCCESS) {
				if (openFiles (backup_name, new_name, &old, &new)) {
					name = backup_name;
					setenv = 1;
				} else {
					XauUnlockAuth (backup_name);
					lockStatus = LOCK_ERROR;
				}	
			}
		}
		if (lockStatus != LOCK_SUCCESS) {
			Debug ("can't lock auth file %s or backup %s\n",
					home_name, backup_name);
			LogError ("can't lock authorization file %s or backup %s\n",
					home_name, backup_name);
			return;
		}
		initAddrs ();
		if (d->displayType.location == Local)
			writeLocalAuth (new, auth, d->name);
		else
	 		writeAuth (new, auth);
		if (old) {
			if (fstat (fileno (old), &statb) != -1)
				chmod (name, statb.st_mode & 0777);
			while (entry = XauReadAuth (old)) {
				if (!checkAddr (entry->family,
					       entry->address_length, entry->address,
					       entry->number_length, entry->number))
				{
					Debug ("Saving an entry\n");
					dumpAuth (entry);
					writeAuth (new, entry);
				}
				XauDisposeAuth (entry);
			}
			fclose (old);
		}
		doneAddrs ();
		fclose (new);
		if (unlink (name) == -1)
			Debug ("unlink %s failed\n", name);
		envname = name;
		if (link (new_name, name) == -1) {
			Debug ("link failed %s %s\n", new_name, name);
			LogError ("Can't move authorization into place\n");
			setenv = 1;
			envname = new_name;
		} else {
			Debug ("new is in place, go for it!\n");
			unlink (new_name);
		}
		if (setenv) {
			verify->userEnviron = setEnv (verify->userEnviron,
						"XAUTHORITY", envname);
			verify->systemEnviron = setEnv (verify->systemEnviron,
						"XAUTHORITY", envname);
		}
		XauUnlockAuth (name);
		if (envname) {
#ifdef SYSV
			chown (envname, verify->uid);
#ifdef NGROUPS
			chgrp (envname, verify->groups[0]);
#else
			chgrp (envname, verify->gid);
#endif
#else
#ifdef NGROUPS
			chown (envname, verify->uid, verify->groups[0]);
#else
			chown (envname, verify->uid, verify->gid);
#endif
#endif
		}
	}
	Debug ("done SetUserAuthorization\n");
}
