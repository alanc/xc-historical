/*
 * display manager
 */

# include	<stdio.h>
# include	<sys/signal.h>
# include	"dm.h"
# include	"buf.h"

main (argc, argv)
int	argc;
char	**argv;
{
	/*
	 * Step 1 - load configuration parameters
	 */
	InitResources (argc, argv);
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
	static DisplayType	acceptableTypes[] =
		{ secure, insecure, remove, unknown };

	fd = open (servers, 0);
	if (fd == -1) {
		LogError ("cannot access servers file %s\n", servers);
		return;
	}
	serversFile = fileOpen (fd);
	if (serversFile == NULL)
		return;
	while (ReadDisplay (serversFile, acceptableTypes, (char *) 0) != EOB)
		;
	StartDisplays ();
	bufClose (serversFile);
	close (servers);
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
			RemoveDisplay (d);
			break;
		case OBEYSESS_DISPLAY:
			switch (d->displayType) {
			case secure:
			case insecure:
			case foreign:
				StartDisplay (d);
				break;
			case transient:
				RemoveDisplay (d);
				break;
			}
			break;
		case REMANAGE_DISPLAY:
		default:
			StartDisplay (d);
			break;
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

TerminateDisplay (d)
struct display	*d;
{
	DisplayStatus	status;
	int		pid;

	status = d->status;
	pid = d->pid;
	RemoveDisplay (d);
	if (status == running)
		kill (pid, SIGTERM);
}
