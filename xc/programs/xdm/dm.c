/*
 * xdm - display manager daemon
 *
 * $XConsortium: $
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
	if (debugLevel == 0)
		BecomeDaemon ();
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
	while (AnyWellKnownSockets () || AnyDisplaysLeft ())
		WaitForSomething ();
}


ScanServers ()
{
	struct buffer	*serversFile;
	struct display	*d;
	int		fd;
	static DisplayType	acceptableTypes[] =
		{ { Local, Permanent, Secure },
		  { Local, Transient, Secure },
		  { Local, Permanent, Insecure },
		  { Local, Transient, Insecure },
		};

	if (servers[0] == '/') {
		fd = open (servers, 0);
		if (fd == -1) {
			LogError ("cannot access servers file %s\n", servers);
			return;
		}
		serversFile = fileOpen (fd);
	} else {
		fd = -1;
		serversFile = dataOpen (servers, strlen (servers));
	}
	if (serversFile == NULL)
		return;
	while (ReadDisplay (serversFile, acceptableTypes,
 			    sizeof (acceptableTypes) / sizeof (acceptableTypes[0]),
		 	    (char *) 0) != EOB)
		;
	StartDisplays ();
	bufClose (serversFile);
	if (fd != -1)
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
			if (d->displayType.lifetime == Permanent)
				StartDisplay (d);
			else
				RemoveDisplay (d);
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
