/*
 * display manager
 */

# include	<stdio.h>
# include	<sys/signal.h>
# include	"dm.h"
# include	"buf.h"

main ()
{
	/*
	 * Step 1 - load configuration parameters
	 */
	InitResources ();
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

ScanServers ()
{
	struct buffer	*serversFile;
	struct display	*d;
	int		fd;

	fd = open (servers, 0);
	if (fd == -1) {
		LogError ("cannot access servers file %s\n", servers);
		return;
	}
	serversFile = fileOpen (fd);
	if (servers == NULL)
		return;
	SetDefaults ("false", "true");
	while (d = ReadDisplay (serversFile))
		if (d->status == notRunning)
			StartDisplay (d);
	bufClose (serversFile);
}

/*
 * notice that a child has died and may need another
 * sub-daemon started
 */

CleanChildren ()
{
	int		pid;
	struct display	*d;
	waitType	status;

#ifdef SYSV
	signal (SIGCHLD, CleanChildren);
#endif
	Debug ("CleanChildren\n");
	pid = wait (&status);
	Debug ("pid: %d\n", pid);
	d = FindDisplayByPid (pid);
	if (d) {
		d->status = notRunning;
		switch (waitVal (status)) {
		case UNMANAGE_DISPLAY:
			break;
		case OBEYSESS_DISPLAY:
			if (!d->multipleSessions)
				break;
		case REMANAGE_DISPLAY:
		default:
			StartDisplay (d);
		}
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
		exit (REMANAGE_DISPLAY);
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
