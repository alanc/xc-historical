/*
 * xdm - display manager daemon
 *
 * $XConsortium: session.c,v 1.4 88/09/23 14:21:31 keith Exp $
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
 * session.c
 */

# include "dm.h"
# include <X11/Xlib.h>
# include <signal.h>

static int	clientPid;

ManageSession (d)
struct display	*d;
{
	struct greet_info	greet;
	struct verify_info	verify;
	int			pid;

	/*
	 * Step 5: Load system default Resources
	 */
	Debug ("ManageSession %s\n", d->name);
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
	Debug ("Greet loop finished\n");
	/*
	 * Step 8: Run system-wide initialization file
	 *	   /etc/Xstartup
	 */
	source (d->startup);
	/*
	 * Step 9: Start the clients, changing uid/groups
	 *	   setting up environment and running /etc/Xsession
	 */
	Debug ("Startup sourced\n");
	if (StartClient (&verify, &clientPid)) {
		Debug ("Client Started\n");
		/*
		 * Step 13: Wait for session to end,
		 */
		for (;;) {
			pid = wait (0);
			if (pid == clientPid)
				break;
		}
	} else {
		LogError ("session start failed\n");
	}
	/*
	 * Step 15: run /etc/Xreset
	 */
	source (d->reset);
	exit (OBEYTERM_DISPLAY);
}

LoadXloginResources (d)
struct display	*d;
{
	char	cmd[1024];

	if (d->resources[0] && access (d->resources, 4) == 0) {
		sprintf (cmd, "%s -display %s -merge %s",
				d->xrdb, d->name, d->resources);
		Debug ("Loading resource file: %s\n", cmd);
		system (cmd);
	}
}

StartClient (verify, pidp)
struct verify_info	*verify;
int			*pidp;
{
	char	**f, *home, *getEnv ();
	int	pid;

	Debug ("StartSession %s: ", verify->argv[0]);
	for (f = verify->argv; *f; f++)
		Debug ("%s ", *f);
	Debug ("; ");
	for (f = verify->environ; *f; f++)
		Debug ("%s ", *f);
	Debug ("\n");
	switch (pid = fork ()) {
	case 0:
		setpgrp (0, getpid ());
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
		*pidp = pid;
		return 1;
	}
}

source (file)
char	*file;
{
	if (file[0] && access (file, 1) == 0) {
		Debug ("source %s\n", file);
		system (file);
	}
}
