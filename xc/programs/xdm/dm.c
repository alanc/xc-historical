/*
 * display manager
 */

# include	<stdio.h>
# include	<sys/types.h>
# include	<sys/socket.h>
# include	<netinet/in.h>
# include	<sys/wait.h>
# include	<sys/signal.h>
# include	<X11/Xlib.h>
# include	<X11/Xresource.h>
# include	"dm.h"

char	*malloc (), *realloc (), *strcpy ();
#ifndef FD_ZERO
/* typedef	struct	fd_set { int fds_bits[1]; } fd_set; */
# define FD_ZERO(fdp)	bzero ((fdp), sizeof (*(fdp)))
# define FD_SET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] |= (1 << ((f) % (sizeof (int) * 8))))
# define FD_ISSET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] & (1 << ((f) % (sizeof (int) * 8))))
#endif

# define FORK_RETRY	5

int	socketFd;

fd_set	WellKnownSocketsMask;
int	WellKnownSocketsMax;

struct display	*FindDisplayByName (), *FindDisplayByPid (), *NewDisplay ();

/*
 * these should be resources in /etc/Xdm-config
 */

# define DM_RESOURCES	"/usr/lib/xdm/Xdm-config"

char	*servers;
char	*resources;
char	*xrdb;
char	*startup;
char	*reset;
char	*session;
int	request_port;
char	*errorLogFile;
int	openDelay;
int	openRepeat;

# define DM_STRING	0
# define DM_INT		1

struct dmResources {
	char	*name, *class;
	int	type;
	char	**dm_value;
	char	*default_value;
} DmResources[] = {
"DisplayManager.servers",	"DisplayManager.Servers",
	DM_STRING,	&servers,	"/usr/lib/xdm/Xservers",
"DisplayManager.resources",	"DisplayManager.Resources",
	DM_STRING,	&resources,	"/usr/lib/xdm/Xresources",
"DisplayManager.xrdb",		"DisplayManager.Xrdb",
	DM_STRING,	&xrdb,		"/usr/bin/X11/xrdb",
"DisplayManager.startup",	"DisplayManager.Startup",
	DM_STRING,	&startup,	"/usr/lib/xdm/Xstartup",
"DisplayManager.reset",		"DisplayManager.Reset",	
	DM_STRING,	&reset,		"/usr/lib/xdm/Xreset",
"DisplayManager.session",	"DisplayManager.Session",
	DM_STRING,	&session,	"/usr/lib/xdm/Xsession",
"DisplayManager.requestPort",	"DisplayManager.RequestPort",
	DM_INT,		(char **) &request_port,	"5999",
"DisplayManager.errorLogFile",	"DisplayManager.ErrorLogFile",
	DM_STRING,	&errorLogFile,	"/usr/adm/Xdm-errors",
"DisplayManager.openDelay",	"DisplayManager.OpenDelay",
	DM_INT,		(char **) &openDelay,		"5",
"DisplayManager.openRepeat",	"DisplayManager.OpenRepeat",
	DM_INT,		(char **) &openRepeat,		"5",
};

# define NUM_DM_RESOURCES	(sizeof DmResources / sizeof DmResources[0])

XrmDatabase	DmResourceDB;

main ()
{
	/*
	 * Step 1 - load configuration parameters
	 */
	LoadDMResources ();
	InitErrorLog ();
	/*
	 * Step 2 - Read /etc/Xservers and set up
	 *	    the socket.  For now, this
	 *	    is a TCP socket to make testing
	 *	    easier with telnet
	 *
	 *	    Keep a sub-daemon running
	 *	    for each entry
	 */
	ScanServers ();
	CreateWellKnownSockets ();
	for (;;)
		WaitForSomething ();
}

LoadDMResources ()
{
	int	i;
	char	*type;
	XrmValue	value;
	char	*string, *strncpy (), *malloc ();
	int	len;

	DmResourceDB = XrmGetFileDatabase ( DM_RESOURCES );
	for (i = 0; i < NUM_DM_RESOURCES; i++) {
		if (XrmGetResource (DmResourceDB,
			DmResources[i].name, DmResources[i].class,
			&type, &value))
		{
			string = value.addr;
			len = value.size;
		} else {
			string = DmResources[i].default_value;
			len = strlen (string);
		}
		string = strncpy (malloc (len+1), string, len);
		string[len] = '\0';
		Debug ("resource %s value %s\n", DmResources[i].name, string);
		switch (DmResources[i].type) {
		case DM_STRING:
			*(DmResources[i].dm_value) = string;
			break;
		case DM_INT:
			*((int *) DmResources[i].dm_value) = atoi (string);
			free (string);
			break;
		}
	}
}

struct display	*ReadDisplay ();

# define EOB	-1

# define bufc(b)	((b)->cnt == (b)->bufp ? ((b)->fill ?\
			 (*(b)->fill) (b) : EOB) : (b)->buf[(b)->bufp++])

struct buffer {
	char	*buf;
	int	cnt;
	int	bufp;
	int	size;
	int	(*fill)();
	int	private;
};

struct buffer *
dataOpen (d, len)
char	*d;
int	len;
{
	struct buffer	*b;

	b = (struct buffer *) malloc (sizeof (struct buffer));
	b->buf = d;
	b->cnt = len;
	b->bufp = 0;
	b->fill = 0;
	b->size = len;
	b->private = -1;
	return b;
}

fileFill (b)
	struct buffer	*b;
{
	b->cnt = read (b->private, b->buf, b->size);
	b->bufp = 0;
	if (b->cnt <= 0) {
		b->cnt = 0;
		return EOB;
	}
	return b->buf[b->bufp++];
}

# define BUFFER_SIZE	1024

struct buffer *
fileOpen (fd)
int	fd;
{
	struct buffer	*b;

	b = (struct buffer *) malloc (sizeof (struct buffer));
	if (!b)
		LogPanic ("out of memory\n");
	b->buf = malloc (b->size = BUFFER_SIZE);
	if (!b->buf)
		LogPanic ("out of memory\n");
	b->cnt = b->bufp = 0;
	b->private = fd;
	b->fill = fileFill;
	return b;
}

bufClose (b)
	struct buffer	*b;
{
	if (b->private != -1)
		free (b->buf);
	free ((char *) b);
}

ScanServers ()
{
	struct buffer	*serversFile;
	int		fd;

	fd = open (servers, 0);
	if (fd == -1) {
		LogError ("cannot access servers file %s\n", servers);
		return;
	}
	serversFile = fileOpen (fd);
	if (servers == NULL)
		return;
	ScanFile (serversFile);
	bufClose (serversFile);
}

ScanFile (file)
struct buffer	*file;
{
	while (ReadDisplay (file))
		;
	StartDisplays ();
}

struct display *
ReadDisplay (file)
struct buffer	*file;
{
	int		c;
	char		**args;
	struct display	*d;
	char		word[1024];
	int		i;

	int		isNew = 0;

	c = readWord (file, word, sizeof (word));
	if (word[0] != '\0') {
		d = FindDisplayByName (word);
		if (!d) {
			d = NewDisplay (word);
			isNew = 1;
		}
		i = 0;
		args = (char **) malloc (sizeof (char *));
		if (!args)
			LogPanic ("out of memory\n");
		while (c != EOB && c != '\n') {
			c = readWord (file, word, sizeof (word));
			if (word[0] != '\0') {
				args[i] = strcpy (malloc (strlen (word) + 1), word);
				i++;
				args = (char **) 
				    realloc ((char *) args, (i+1) * sizeof (char **));
			}
		}
		args[i] = 0;
		d->argv = args;
		return d;
	}
	return 0;
}

# define isbreak(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')

int
readWord (file, word, len)
struct buffer	*file;
char	*word;
int	len;
{
	int	c;
	int	i;

	while ((c = bufc (file)) != EOB)
		if (!isbreak (c))
			break;
	if (c == EOB) {
		word[0] = '\0';
		return EOB;
	}
	word[0] = c;
	i = 1;
	while ((c = bufc (file)) != EOB && !isbreak (c)) {
		if (i < len-1 && c != '\r') {
			word[i] = c;
			++i;
		}
	}
	word[i] = '\0';
	return c;
}

CreateWellKnownSockets ()
{
	struct sockaddr_in	sock_addr;

	socketFd = socket (AF_INET, SOCK_DGRAM, 0);
	if (socketFd == -1) {
		perror ("socket");
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
}

ProcessRequestSocket (fd)
int	fd;
{
	Debug ("ProcessRequestSocket\n");
	StartSocket (fd);
}

WaitForSomething ()
{
	fd_set	reads;
	int	nready;

	Debug ("WaitForSomething\n");
	reads = WellKnownSocketsMask;
	nready = select (WellKnownSocketsMax + 1, &reads, 0, 0, 0);
	if (nready > 0 && FD_ISSET (socketFd, &reads))
		ProcessRequestSocket (socketFd);
}

/*
 * notice that a child has died and may need another
 * sub-daemon started
 */

CleanChildren ()
{
	int		pid;
	struct display	*d;
	union wait	status;

	Debug ("CleanChildren\n");
	pid = wait (&status);
	Debug ("pid: %d\n", pid);
	d = FindDisplayByPid (pid);
	if (d) {
		d->status = notRunning;
		StartDisplays ();
	}
}

StartDisplay (d)
struct display	*d;
{
	int	pid;

	Debug ("StartDisplay %s\n", d->name);
	switch (pid = fork ()) {
	case 0:
		ManageDisplay (d);
		exit (0);
	case -1:
		break;
	default:
		Debug ("pid: %d\n", pid);
		d->pid = pid;
		d->status = running;
		signal (SIGCHLD, CleanChildren);
		break;
	}
}

StartSocket (s)
int	s;
{
	int	pid;
	struct display	*d;
	char	buf[4096];
	int	len;
	struct buffer	*f;
	char	from[1024];
	struct sockaddr_in	*from_in;
	int		fromlen;
	static char	poll_providers[] = POLL_PROVIDERS;
	static char	advertise[] = ADVERTISE;

	Debug ("StartSocket %d\n", s);
	fromlen = sizeof (from);
	len = recvfrom (s, buf, sizeof (buf), 0, 
			(struct sockaddr *) from, &fromlen);
	if (len <= 0)
		return;
	/*
	 * respond to broadcasts for services
	 */
	if (len == strlen (poll_providers) && !strncmp (buf, poll_providers, len))
	{
		from_in = (struct sockaddr_in *) from;
		
		Debug (
"acknowledging request for display manager addresses to %08x port %d\n",
ntohl (from_in->sin_addr.s_addr), ntohs (from_in->sin_port));
		sendto (s, advertise, strlen (advertise), 0, 
			(struct sockaddr *) from, fromlen);
		return;
	}
	switch (pid = fork ()) {
	case 0:
		close (socketFd);
		close (s);
		f = dataOpen (buf, len);
		d = ReadDisplay (f);
		bufClose (f);
		if (!d)
			LogError ("Invalid packet received\n");
		else {
			Debug ("Starting %d from socket\n", d->name);
			ManageDisplay (d);
		}
		exit (0);
	case -1:
		break;
	default:
		break;
	}
}

struct display	*displays;

StartDisplays ()
{
	struct display	*d;

	Debug ("StartDisplays\n");
	for (d = displays; d; d = d->next)
		if (d->status == notRunning)
			StartDisplay (d);
}

struct display *
FindDisplayByName (name)
char	*name;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (!strcmp (name, d->name))
			return d;
	return 0;
}

struct display *
FindDisplayByPid (pid)
int	pid;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (pid == d->pid)
			return d;
	return 0;
}

RemoveDisplay (old)
struct display	*old;
{
	struct display	*d, *p;

	p = 0;
	for (d = displays; d; d = d->next)
		if (d == old) {
			if (p)
				p->next = d->next;
			else
				displays = d->next;
			free (d);
			break;
		}
}

struct display *
NewDisplay (name)
char	*name;
{
	struct display	*d;

	d = (struct display *) malloc (sizeof (struct display));
	if (!d)
		LogPanic ("out of memory\n");
	d->next = displays;
	d->name = strcpy (malloc (strlen (name) + 1), name);
	if (!d->name)
		LogPanic ("out of memory\n");
	d->argv = 0;
	d->status = notRunning;
	d->pid = -1;
	displays = d;
	return d;
}

/*
 * sub-daemon portion.
 *
 * this part manages a single display
 */

int	serverPid;
int	sessionPid;

int	serverDead;
int	sessionDead;

sessionDied ()
{
	int	pid;

	pid = wait (0);
	if (pid == serverPid) {
		Debug ("server died\n");
		serverDead = 1;
	}
	if (pid == sessionPid) {
		Debug ("session died\n");
		sessionDead = 1;
	}
}

ManageDisplay (d)
struct display	*d;
{
	struct greet_info	greet;
	struct verify_info	verify;
	int			pid;

	Debug ("manage display %s\n", d->name);
	sessionDead = 0;
	serverDead = 0;
	signal (SIGCHLD, sessionDied);
	/*
	 * Step 4: Start server control program
	 */
	for (;;) {
		switch (StartServer (d)) {
		case -1:
			Debug ("aborting display %s\n", d->name);
			exit (0);
		case 0:
			continue;
		}
		break;
	}
	signal (SIGCHLD, SIG_IGN);
	/*
	 * Step 5: Load system default Resources
	 */
	LoadXloginResources (d);
	InitGreet (d);
	for (;;) {
		/*
		 * Step 6: Greet user, requesting name/password
		 */
		Greet (d, &greet);
		/*
		 * Step 7: Verify user
		 */
		if (Verify (d, &greet, &verify))
			break;
		else
			FailedLogin (d, &greet);
	}
	CloseGreet (d);
	/*
	 * Step 8: Run system-wide initialization file
	 *	   /etc/Xstartup
	 */
	source (startup);
	/*
	 * Step 9: Start user session, changing uid/groups
	 *	   setting up environment and running /etc/Xsession
	 */
	if (StartSession (&verify)) {
		/*
		 * Step 13: Wait for session to end,
		 *	    also see if the server has died
		 *	    unexpectedly and restart
		 */
		for (;;) {
			pid = wait (0);
			if (pid == serverPid) {
				Debug ("server died\n");
				break;
			}
			if (pid == sessionPid) {
				Debug ("session died\n");
				/*
				 * Step 14: send the server-control
				 *	    program a TERM.
				 */
				TerminateServer ();
				break;
			}
		}
	} else {
		LogError ("session start failed\n");
		TerminateServer ();
	}
	/*
	 * make sure the server is dead
	 */
	while ((pid = wait (0)) != -1)
		if (pid == serverPid)
			break;
	/*
	 * Step 15: run /etc/Xreset
	 */
	source (reset);
	exit (0);
}

Display	*dpy;

# include	<setjmp.h>

jmp_buf	openAbort;

abortOpen ()
{
	longjmp (openAbort, 1);
}

StartServer (d)
struct display	*d;
{
	char	**f;
	int	i;

	Debug ("StartServer ");
	for (f = d->argv; *f; f++)
		Debug ("'%s' ", *f);
	Debug ("\n");
	switch (serverPid = fork ()) {
	case 0:
		execv (d->argv[0], d->argv);
		LogError ("server control program %s cannot be executed",
				d->argv[0]);
		sleep (openDelay);
		exit (1);
	case -1:
		LogError ("fork failed, sleeping\n");
		sleep (openDelay);
		return 0;
	default:
		break;
	}
	/*
	 * kludge to avoid race condition in TCP
	 */
	sleep (openDelay);
	for (i = 0; i < openRepeat; i++) {
		signal (SIGALRM, abortOpen);
		alarm (openDelay);
		if (!setjmp (openAbort)) {
			dpy = XOpenDisplay (d->name);
			alarm (0);
			signal (SIGALRM, SIG_DFL);
			if (!dpy) {
				if (serverDead == 1) {
					Debug ("server died\n");
					return -1;
				}
			} else {
				return 1;
			}
			Debug ("waiting for server to start\n");
			sleep (openDelay);
		} else {
			Debug ("hung in open, aborting\n");
			break;
		}
	}
	Debug ("giving up on server\n");
	LogError ("server start failed for %s, giving up\n", d->name);
	TerminateServer ();
	return -1;
}

TerminateServer ()
{
	Debug ("terminiating server\n");
	if (dpy)
		XCloseDisplay (dpy);
	kill (serverPid, SIGTERM);
}

HupServer ()
{
	Debug ("hupping server\n");
	XCloseDisplay (dpy);
	kill (serverPid, SIGHUP);
}

LoadXloginResources (d)
struct display	*d;
{
	char	cmd[1024];

	if (access (resources, 4) == 0) {
		sprintf (cmd, "%s -display %s -merge %s",
				xrdb, d->name, resources);
		Debug ("Loading resource file: %s\n", cmd);
		system (cmd);
	}
}

StartSession (verify)
struct verify_info	*verify;
{
	char	**f, *home, *getEnv ();
	Debug ("StartSession %s: ", verify->argv[0]);
	for (f = verify->argv; *f; f++)
		Debug ("%s ", *f);
	Debug ("; ");
	for (f = verify->environ; *f; f++)
		Debug ("%s ", *f);
	Debug ("\n");
	switch (sessionPid = fork ()) {
	case 0:
#ifdef NGROUPS
		setgroups (verify->ngroups, verify->groups);
		setgid (verify->groups[0]);
#else
		setgid (verify->gid);
#endif
		setuid (verify->uid);
		home = getEnv (verify->environ, "HOME");
		if (home)
			if (chdir (home) == -1)
				chdir ("/");
		execve (verify->argv[0], verify->argv, verify->environ);
		Debug ("exec failed\n");
		exit (1);
	case -1:
		Debug ("StartSession failed\n");
		return 0;
	default:
		Debug ("StartSession suceeded\n");
		return 1;
	}
}

source (file)
{
	Debug ("source %s\n", file);
	if (access (file, 0) == 0)
		system (file);
}

