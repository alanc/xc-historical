/*
 * xdm - display manager daemon
 *
 * $XConsortium: dm.c,v 1.12 88/12/05 17:26:40 keith Exp $
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

extern void	exit (), abort ();

static void	RescanServers ();
static int	Rescan;

main (argc, argv)
int	argc;
char	**argv;
{
	void	TerminateAll (), RescanNotify ();
#ifndef SYSV
	void	ChildNotify ();
#endif

	/*
	 * Step 1 - load configuration parameters
	 */
	InitResources (argc, argv);
	LoadDMResources ();
	if (debugLevel == 0 && daemonMode)
		BecomeDaemon ();
	InitErrorLog ();
	StorePid ();
	(void) signal (SIGTERM, TerminateAll);
	(void) signal (SIGINT, TerminateAll);
	/*
	 * Step 2 - Read /etc/Xservers and set up
	 *	    the socket.
	 *
	 *	    Keep a sub-daemon running
	 *	    for each entry
	 */
	ScanServers ();
	(void) signal (SIGHUP, RescanNotify);
#ifndef SYSV
	(void) signal (SIGCHLD, ChildNotify);
#endif
	while (AnyDisplaysLeft ()) {
		if (Rescan)
			RescanServers ();
		WaitForChild ();
	}
	Debug ("Nothing left to do, exiting\n");
}


void
RescanNotify ()
{
	void	RescanNotify ();

	Debug ("Caught SIGHUP\n");
	Rescan = 1;
#ifdef SYSV
	signal (SIGHUP, RescanNotify);
#endif
}

ScanServers ()
{
	struct buffer	*serversFile;
	int		fd;
	static DisplayType	acceptableTypes[] =
		{ { Local, Permanent, Secure },
		  { Local, Transient, Secure },
		  { Local, Permanent, Insecure },
		  { Local, Transient, Insecure },
		  { Foreign, Permanent, Secure },
		  { Foreign, Transient, Secure },
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
 			    sizeof (acceptableTypes) / sizeof (acceptableTypes[0]))
		 	    != EOB)
		;
	StartDisplays ();
	bufClose (serversFile);
	if (fd != -1)
		close (fd);
}

static void
MarkDisplay (d)
struct display	*d;
{
	d->state = MissingEntry;
}

static void
RescanServers ()
{
	Debug ("rescanning servers\n");
	Rescan = 0;
	ForEachDisplay (MarkDisplay);
	ReinitResources ();
	ScanServers ();
}

/*
 * catch a SIGTERM, kill all displays and exit
 */

static void
TerminateAll ()
{
	void	TerminateDisplay ();

	ForEachDisplay (TerminateDisplay);
}

/*
 * notice that a child has died and may need another
 * sub-daemon started
 */

#ifndef SYSV
static int	ChildReady;

void
ChildNotify ()
{
	ChildReady = 1;
}
#endif

WaitForChild ()
{
	int		pid;
	struct display	*d;
	waitType	status;

#ifdef SYSV
	/* XXX classic sysV signal race condition here with RescanNotify */
	if ((pid = wait (&status)) != -1)
#else
	sigblock (sigmask (SIGCHLD) | sigmask (SIGHUP));
	if (!ChildReady && !Rescan)
		sigpause (0);
	else
		sigblock (0);
	ChildReady = 0;
	while ((pid = wait3 (&status, WNOHANG, (struct rusage *) 0)) != 0)
#endif
	{
		Debug ("pid: %d\n", pid);
		d = FindDisplayByPid (pid);
		if (d) {
			d->status = notRunning;
			switch (waitVal (status)) {
			case UNMANAGE_DISPLAY:
				Debug ("Display exited with UNMANAGE_DISPLAY\n");
				RemoveDisplay (d);
				break;
			case OBEYSESS_DISPLAY:
				Debug ("Display exited with OBEYSESS_DISPLAY\n");
				if (d->displayType.lifetime == Permanent)
					StartDisplay (d);
				else
					RemoveDisplay (d);
				break;
			default:
				Debug ("Display exited with unknown status %d\n", waitVal(status));
				StartDisplay (d);
				break;
			case REMANAGE_DISPLAY:
				Debug ("Display exited with REMANAGE_DISPLAY\n");
				StartDisplay (d);
				break;
			}
		}
	}
}

static void
CheckDisplayStatus (d)
struct display	*d;
{
	switch (d->state) {
	case MissingEntry:
		TerminateDisplay (d);
		break;
	case NewEntry:
		StartDisplay (d);
		break;
	case OldEntry:
		break;
	}
}

StartDisplays ()
{
	ForEachDisplay (CheckDisplayStatus);
}

StartDisplay (d)
struct display	*d;
{
	int	pid;

	Debug ("StartDisplay %s\n", d->name);
	switch (pid = fork ()) {
	case 0:
		CleanUpChild ();
		ManageDisplay (d);
		exit (REMANAGE_DISPLAY);
	case -1:
		break;
	default:
		Debug ("pid: %d\n", pid);
		d->pid = pid;
		d->status = running;
		break;
	}
}

void
TerminateDisplay (d)
struct display	*d;
{
	DisplayStatus	status;
	int		pid;

	status = d->status;
	pid = d->pid;
	RemoveDisplay (d);
	if (status == running) {
		if (pid < 2)
			abort ();
		kill (pid, SIGTERM);
	}
}

static FD_TYPE	CloseMask;
static int	max;

RegisterCloseOnFork (fd)
int	fd;
{
	FD_SET (fd, &CloseMask);
	if (fd > max)
		max = fd;
}

ClearCloseOnFork (fd)
int	fd;
{
	FD_CLR (fd, &CloseMask);
	if (fd == max) {
		while (--fd >= 0)
			if (FD_ISSET (fd, &CloseMask))
				break;
		max = fd;
	}
}

CloseOnFork ()
{
	int	fd;

	for (fd = 0; fd < max; fd++)
		if (FD_ISSET (fd, &CloseMask))
			close (fd);
	FD_ZERO (&CloseMask);
	max = 0;
}

StorePid ()
{
	FILE	*f;

	if (pidFile[0] != '\0') {
		f = fopen (pidFile, "w");
		if (!f) {
			LogError ("process-id file %s cannot be opened\n",
				  pidFile);
		} else {
			fprintf (f, "%d\n", getpid ());
			fclose (f);
		}
	}
}
