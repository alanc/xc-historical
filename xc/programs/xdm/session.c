/*
 * xdm - display manager daemon
 *
 * $XConsortium: session.c,v 1.11 88/12/15 18:32:15 keith Exp $
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
# include <X11/Xatom.h>
# include <setjmp.h>
# include <sys/errno.h>

static int	clientPid;

static jmp_buf	abortSession;

static void
catchTerm ()
{
	longjmp (abortSession, 1);
}

extern void	exit ();

ManageSession (d)
struct display	*d;
{
	struct greet_info	greet;
	struct verify_info	verify;
	int			pid;
	Display			*dpy, *InitGreet ();

	Debug ("ManageSession %s\n", d->name);
	/*
	 * Step 5: Load system default Resources
	 */
	LoadXloginResources (d);
	Debug ("name now %s\n", d->name);
	dpy = InitGreet (d);
	Debug ("name now %s\n", d->name);
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
		Debug ("after verify, name %s\n", d->name);
	}
	DeleteXloginResources (d, dpy);
	CloseGreet (d);
	Debug ("Greet loop finished\n");
	/*
	 * Step 8: Run system-wide initialization file
	 */
	if (source (&verify, d->startup) != 0)
		SessionExit (OBEYTERM_DISPLAY);
	clientPid = 0;
	if (!setjmp (abortSession)) {
		signal (SIGTERM, catchTerm);
		/*
	 	 * Step 9: Start the clients, changing uid/groups
	 	 *	   setting up environment and running the session
	 	 */
		if (StartClient (&verify, d, &clientPid)) {
			Debug ("Client Started\n");
			/*
		 	 * Step 13: Wait for session to end,
		 	 */
			for (;;) {
				pid = wait ((waitType *) 0);
				if (pid == clientPid)
					break;
			}
		} else {
			LogError ("session start failed\n");
		}
	} else {
		/*
		 * when terminating the session, nuke
		 * the child and then run the reset script
		 */
		AbortClient (clientPid);
	}
	/*
	 * Step 15: run system-wide reset file
	 */
	source (&verify, d->reset);
	SessionExit (OBEYTERM_DISPLAY);
}

LoadXloginResources (d)
struct display	*d;
{
	char	cmd[1024];

	if (d->resources[0] && access (d->resources, 4) == 0) {
		if (d->authorization && d->authFile && d->authFile[0]) {
			sprintf (cmd, "XAUTHORITY=%s %s -display %s -load %s",
					d->authFile,
					d->xrdb, d->name, d->resources);
		} else {
			sprintf (cmd, "%s -display %s -load %s",
					d->xrdb, d->name, d->resources);
		}
		Debug ("Loading resource file: %s\n", cmd);
		system (cmd);
	}
}

/*ARGSUSED*/
DeleteXloginResources (d, dpy)
struct display	*d;
Display		*dpy;
{
	XDeleteProperty(dpy, RootWindow (dpy, 0), XA_RESOURCE_MANAGER);
}

static jmp_buf syncJump;

static void
syncTimeout ()
{
	longjmp (syncJump, 1);
}

SecureDisplay (d, dpy)
struct display	*d;
Display		*dpy;
{
	Debug ("SecureDisplay %s\n", d->name);
	signal (SIGALRM, syncTimeout);
	if (setjmp (syncJump)) {
		LogError ("WARNING: display %s could not be secured\n",
				d->name);
		SessionExit (ABORT_DISPLAY);
	}
	alarm ((unsigned) d->grabTimeout);
	Debug ("Before XGrabServer %s\n", d->name);
	XGrabServer (dpy);
	if (XGrabKeyboard (dpy, DefaultRootWindow (dpy), True, GrabModeAsync,
			   GrabModeAsync, CurrentTime) != GrabSuccess)
 	{
		alarm (0);
		signal (SIGALRM, SIG_DFL);
		LogError ("WARNING: keyboard on display %s could not be secured\n",
				d->name);
		SessionExit (ABORT_DISPLAY);
	}
	Debug ("XGrabKeyboard succeeded %s\n", d->name);
	alarm (0);
	signal (SIGALRM, SIG_DFL);
	pseudoReset (dpy);
	Debug ("done secure %s\n", d->name);
}

UnsecureDisplay (d, dpy)
struct display	*d;
Display		*dpy;
{
	Debug ("Unsecure display %s\n", d->name);
	XUngrabServer (dpy);
	XSync (dpy, 0);
}

SessionExit (status)
{
	exit (status);
}

StartClient (verify, d, pidp)
struct verify_info	*verify;
struct display		*d;
int			*pidp;
{
	char	**f, *home, *getEnv ();
	char	*failsafeArgv[2];
	int	pid;

	SetUserAuthorization (d, verify);
	if (verify->argv) {
		Debug ("StartSession %s: ", verify->argv[0]);
		for (f = verify->argv; *f; f++)
			Debug ("%s ", *f);
		Debug ("; ");
	}
	if (verify->userEnviron) {
		for (f = verify->userEnviron; *f; f++)
			Debug ("%s ", *f);
		Debug ("\n");
	}
	switch (pid = fork ()) {
	case 0:
		CleanUpChild ();
#ifdef NGROUPS

		setgid (verify->groups[0]);
		setgroups (verify->ngroups, verify->groups);
#else
		setgid (verify->gid);
#endif
		setuid (verify->uid);
		home = getEnv (verify->userEnviron, "HOME");
		if (home)
			if (chdir (home) == -1) {
				LogError ("No home directory %s for user %s, using /\n",
					  home, getEnv (verify->userEnviron, "USER"));
				chdir ("/");
			}
		if (verify->argv) {
			Debug ("executing session %s\n", verify->argv[0]);
			execve (verify->argv[0], verify->argv, verify->userEnviron);
			LogError ("Session execution failed %s\n", verify->argv[0]);
			Debug ("exec failed\n");
		} else {
			LogError ("Session has no command/arguments\n");
		}
		failsafeArgv[0] = d->failsafeClient;
		failsafeArgv[1] = 0;
		execve (failsafeArgv[0], failsafeArgv, verify->userEnviron);
		exit (1);
	case -1:
		Debug ("StartSession, fork failed\n");
		LogError ("can't start session for %d, fork failed\n", d->name);
		return 0;
	default:
		Debug ("StartSession, fork suceeded %d\n", pid);
		*pidp = pid;
		return 1;
	}
}

static jmp_buf	tenaciousClient;

static void
waitAbort ()
{
	longjmp (tenaciousClient, 1);
}

#ifdef SYSV
#define killpg(pgrp, sig) kill(-(pgrp), sig)
#endif /* SYSV */

extern int  errno;

AbortClient (pid)
int	pid;
{
	int	sig = SIGTERM;
#ifdef __STDC__
	volitile int	i;
#else
	int	i;
#endif
	int	retId;
	for (i = 0; i < 4; i++) {
		if (killpg (pid, sig) == -1) {
			switch (errno) {
			case EPERM:
				LogError ("xdm can't kill client\n");
			case EINVAL:
			case ESRCH:
				return;
			}
		}
		if (!setjmp (tenaciousClient)) {
			(void) signal (SIGALRM, waitAbort);
			(void) alarm ((unsigned) 10);
			retId = wait ((waitType *) 0);
			(void) alarm ((unsigned) 0);
			(void) signal (SIGALRM, SIG_DFL);
			if (retId == pid)
				break;
		} else
			signal (SIGALRM, SIG_DFL);
		sig = SIGKILL;
	}
}

int
source (verify, file)
struct verify_info	*verify;
char			*file;
{
	char	*args[4];
	int	pid;
	extern int	errno;
	waitType	result;

	Debug ("source %s\n", file);
	if (file[0] && access (file, 1) == 0) {
		switch (pid = fork ()) {
		case 0:
			CleanUpChild ();
			if (!(args[0] = getEnv (verify->systemEnviron, "SHELL")))
				args[0] = "/bin/sh";
			args[1] = "-c";
			args[2] = file;
			args[3] = 0;
			Debug ("interpreting %s with %s\n", args[2], args[0]);
			execve (args[0], args, verify->systemEnviron);
			LogError ("can't execute system shell %s\n", args[0]);
			exit (1);
		case -1:
			Debug ("fork failed\n");
			LogError ("can't fork to execute %s\n", file);
			return 1;
			break;
		default:
			while (wait (&result) != pid)
				;
			break;
		}
		return waitVal (result);
	}
	return 0;
}
