/*
 * xdm - display manager daemon
 *
 * $XConsortium: session.c,v 1.32 90/03/29 11:35:41 keith Exp $
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
# include <stdio.h>
# include <ctype.h>

extern int  errno;

static int			clientPid;
static struct greet_info	greet;
static struct verify_info	verify;

static jmp_buf	abortSession;

static SIGVAL
catchTerm ()
{
    longjmp (abortSession, 1);
}

static jmp_buf	pingTime;

static SIGVAL
catchAlrm ()
{
    longjmp (pingTime, 1);
}

SessionPingFailed (d)
    struct display  *d;
{
    if (clientPid > 1)
    {
    	AbortClient (clientPid);
    	source (&verify, d->reset);
    }
    SessionExit (d, RESERVER_DISPLAY);
}

extern void	exit ();

/*
 * We need our own error handlers because we can't be sure what exit code Xlib
 * will use, and our Xlib does exit(1) which matches REMANAGE_DISPLAY, which
 * can cause a race condition leaving the display wedged.  We need to use
 * RESERVER_DISPLAY for IO errors, to ensure that the manager waits for the
 * server to terminate.  For other X errors, we should give up.
 */

static
IOErrorHandler (dpy)
    Display *dpy;
{
    extern char *sys_errlist[];
    extern int sys_nerr;
    char *s = ((errno >= 0 && errno < sys_nerr) ? sys_errlist[errno]
						: "unknown error");

    LogError("fatal IO error %d (%s)\n", errno, s);
    exit(RESERVER_DISPLAY);
}

static int
ErrorHandler(dpy, event)
    Display *dpy;
    XErrorEvent *event;
{
    LogError("X error\n");
    if (XmuPrintDefaultErrorMessage (dpy, event, stderr) == 0) return 0;
    exit(UNMANAGE_DISPLAY);
}

ManageSession (d)
struct display	*d;
{
    int			pid, code;
    Display		*dpy, *InitGreet ();

    Debug ("ManageSession %s\n", d->name);
    (void)XSetIOErrorHandler(IOErrorHandler);
    (void)XSetErrorHandler(ErrorHandler);
    SetTitle(d->name, (char *) 0);
    /*
     * Step 5: Load system default Resources
     */
    LoadXloginResources (d);
    dpy = InitGreet (d);
    for (;;) {
	/*
	 * Greet user, requesting name/password
	 */
	code = Greet (d, &greet);
	if (code != 0)
	{
	    CloseGreet (d);
	    SessionExit (d, code);
	}
	/*
	 * Verify user
	 */
	if (Verify (d, &greet, &verify))
		break;
	else
		FailedLogin (d, &greet);
    }
    DeleteXloginResources (d, dpy);
    CloseGreet (d);
    Debug ("Greet loop finished\n");
    /*
     * Run system-wide initialization file
     */
    if (source (&verify, d->startup) != 0)
    {
	Debug ("Startup program %s exited with non-zero status\n",
		d->startup);
	SessionExit (d, OBEYSESS_DISPLAY);
    }
    clientPid = 0;
    if (!setjmp (abortSession)) {
	signal (SIGTERM, catchTerm);
	/*
	 * Start the clients, changing uid/groups
	 *	   setting up environment and running the session
	 */
	if (StartClient (&verify, d, &clientPid)) {
	    Debug ("Client Started\n");
	    /*
	     * Wait for session to end,
	     */
	    for (;;) {
		if (d->pingInterval)
		{
		    if (!setjmp (pingTime))
		    {
			signal (SIGALRM, catchAlrm);
			alarm (d->pingInterval * 60);
			pid = wait ((waitType *) 0);
			alarm (0);
		    }
		    else
		    {
			alarm (0);
		    	if (!PingServer (d, (Display *) NULL))
			    SessionPingFailed (d);
		    }
		}
		else
		{
		    pid = wait ((waitType *) 0);
		}
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
     * run system-wide reset file
     */
    Debug ("Source reset program %s\n", d->reset);
    source (&verify, d->reset);
    SessionExit (d, OBEYSESS_DISPLAY);
}

LoadXloginResources (d)
struct display	*d;
{
    char	*args[4];
    char	**env = 0, **setEnv();

    if (d->resources[0] && access (d->resources, 4) == 0) {
	env = setEnv (env, "DISPLAY", d->name);
	if (d->authFile)
	    env = setEnv (env, "XAUTHORITY", d->authFile);
	args[0] = d->xrdb;
	args[1] = "-load";
	args[2] = d->resources;
	args[3] = NULL;
	Debug ("Loading resource file: %s\n", d->resources);
	(void) runAndWait (args, env);
	freeEnv (env);
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

static SIGVAL
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
	SessionExit (d, RESERVER_DISPLAY);
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
	SessionExit (d, RESERVER_DISPLAY);
    }
    Debug ("XGrabKeyboard succeeded %s\n", d->name);
    alarm (0);
    signal (SIGALRM, SIG_DFL);
    pseudoReset (dpy);
    if (!d->grabServer)
    {
	XUngrabServer (dpy);
	XSync (dpy, 0);
    }
    Debug ("done secure %s\n", d->name);
}

UnsecureDisplay (d, dpy)
struct display	*d;
Display		*dpy;
{
    Debug ("Unsecure display %s\n", d->name);
    if (d->grabServer)
	XUngrabServer (dpy);
    XSync (dpy, 0);
}

SessionExit (d, status)
    struct display  *d;
{
    /* make sure the server gets reset after the session is over */
    if (d->serverPid >= 2 && d->resetSignal)
	kill (d->serverPid, d->resetSignal);
    else
	ResetServer (d);
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
	SetUserAuthorization (d, verify);
	home = getEnv (verify->userEnviron, "HOME");
	if (home)
		if (chdir (home) == -1) {
			LogError ("No home directory %s for user %s, using /\n",
				  home, getEnv (verify->userEnviron, "USER"));
			chdir ("/");
		}
	if (verify->argv) {
		Debug ("executing session %s\n", verify->argv[0]);
		execute (verify->argv, verify->userEnviron);
		LogError ("Session execution failed %s\n", verify->argv[0]);
	} else {
		LogError ("Session has no command/arguments\n");
	}
	failsafeArgv[0] = d->failsafeClient;
	failsafeArgv[1] = 0;
	execute (failsafeArgv, verify->userEnviron);
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

static SIGVAL
waitAbort ()
{
	longjmp (tenaciousClient, 1);
}

#ifdef SYSV
# include	<ctype.h>
#define killpg(pgrp, sig) kill(-(pgrp), sig)
#endif /* SYSV */

AbortClient (pid)
int	pid;
{
    int	sig = SIGTERM;
#ifdef __STDC__
    volatile int	i;
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
    char	*args[2];
    if (file && file[0]) {
	Debug ("source %s\n", file);
	args[0] = file;
	args[1] = NULL;
	return runAndWait (args, verify->systemEnviron);
    }
    return 0;
}

int
runAndWait (args, environ)
    char	**args;
    char	**environ;
{
    int	pid;
    extern int	errno;
    waitType	result;

    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();
	execute (args, environ);
	LogError ("can't execute %s\n", args[0]);
	exit (1);
    case -1:
	Debug ("fork failed\n");
	LogError ("can't fork to execute %s\n", args[0]);
	return 1;
	break;
    default:
	while (wait (&result) != pid)
		;
	break;
    }
    return waitVal (result);
}

execute (argv, environ)
char	**argv;
char	**environ;
{
    /* make stdout follow stderr to the log file */
    dup2 (2,1);
    execve (argv[0], argv, environ);
    /*
     * In case this is a shell script which hasn't been
     * made executable (or this is a SYSV box), do
     * a reasonable thing
     */
    if (errno != ENOENT) {
	char	program[1024], *e, *p, *optarg;
	FILE	*f;
	char	**newargv, **av;
	int	argc;

	/*
	 * emulate BSD kernel behaviour -- read
	 * the first line; check if it starts
	 * with "#!", in which case it uses
	 * the rest of the line as the name of
	 * program to run.  Else use "/bin/sh".
	 */
	f = fopen (argv[0], "r");
	if (!f)
	    return;
	if (fgets (program, sizeof (program) - 1, f) == NULL)
 	{
	    fclose (f);
	    return;
	}
	fclose (f);
	e = program + strlen (program) - 1;
	if (*e == '\n')
	    *e = '\0';
	if (!strncmp (program, "#!", 2)) {
	    p = program + 2;
	    while (*p && isspace (*p))
		++p;
	    optarg = p;
	    while (*optarg && !isspace (*optarg))
		++optarg;
	    if (*optarg) {
		*optarg = '\0';
		do
		    ++optarg;
		while (*optarg && isspace (*optarg));
	    } else
		optarg = 0;
	} else {
	    p = "/bin/sh";
	    optarg = 0;
	}
	Debug ("Shell script execution: %s (optarg %s)\n",
		p, optarg ? optarg : "(null)");
	for (av = argv, argc = 0; *av; av++, argc++)
		;
	newargv = (char **) malloc ((argc + (optarg ? 3 : 2)) * sizeof (char *));
	if (!newargv)
	    return;
	av = newargv;
	*av++ = p;
	if (optarg)
	    *av++ = optarg;
	while (*av++ = *argv++)
	    ;
	execve (newargv[0], newargv, environ);
    }
}
