#ifndef lint
static char *rcsid_xinit_c = "$Header: xinit.c,v 11.15 88/08/11 20:09:52 jim Exp $";
#endif /* lint */
#include <X11/copyright.h>

/* Copyright    Massachusetts Institute of Technology    1986	*/

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/resource.h>
#ifndef SYSV
#include <sys/wait.h>
#endif
#include <errno.h>
extern int sys_nerr;
#ifdef hpux
#include <sys/utsname.h>
#endif

#include <setjmp.h>

#ifdef macII
#define vfork() fork()
#endif /* macII */

#if defined(SYSV) && !defined(hpux)
#define vfork() fork()
#endif /* SYSV and not hpux */

#define	TRUE		1
#define	FALSE		0
#define	OK_EXIT		0
#define	ERR_EXIT	1
char hostname[100] = "unix";
char client_display[100];

char *bindir = BINDIR;
char *server_names[] = {
#ifdef vax				/* Digital */
    "Xqvss       Digital monochrome display on Microvax II or III series",
    "Xqdss       Digital color display on Microvax II or III series",
    "Xsm         Digital monochrome display on Vaxstation 2000",
    "Xsg         Digital color display on Vaxstation 2000",
#endif
#ifdef sun				/* Sun */
    "Xsun        Sun monochrome and color displays on Sun 2, 3, or 4 series",
#endif
#ifdef hpux				/* HP */
    "Xhp         HP monochrome and colors displays on 9000/300 series",
#endif
#ifdef apollo				/* Apollo */
    "Xapollo     Apollo monochrome and color displays",
#endif
#ifdef ibm				/* IBM */
    "Xibm        IBM AED, APA, 8514a, megapel, VGA displays on PC/RT",
#endif
#ifdef macII				/* MacII */
    "XmacII      Apple monochrome display on Macintosh II",
#endif
#ifdef M4310				/* Tektronix Pegasus */
    "Xpeg        Tektronix Pegasus display on 4310",
#endif
#if defined(vax) || defined(sun)	/* Parallax */
    "Xplx        Parallax color and video graphics controller",
#endif
    NULL};

char *default_server = "X";
char *default_display = ":0";		/* choose most efficient */
char *default_client[] = {"xterm", "=+1+1", "-n", "login", "-display", NULL};
char *server[100];
char *client[100];
char *displayNum;
char *program;
Display *xd;			/* server connection */
#ifndef SYSV
union wait	status;
#endif /* SYSV */
int serverpid = -1;
int clientpid = -1;
extern int	errno;

sigCatch(sig)
	int	sig;
{
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	Error("signal %d\n", sig);
	shutdown(serverpid, clientpid);
	exit(1);
}

#ifdef SYSV
sigAlarm(sig)
	int sig;
{
	signal (sig, sigAlarm);
}
#endif /* SYSV */

main(argc, argv)
int argc;
register char **argv;
{
	register char **sptr = server;
	register char **cptr = client;
	register char **ptr;
	int pid, i;

	program = *argv++;
	argc--;

#ifndef UNIXCONN
#ifdef hpux
	/* Why not use gethostname()?  Well, at least on my system, I've had to
	 * make an ugly kernel patch to get a name longer than 8 characters, and
	 * uname() lets me access to the whole string (it smashes release, you
	 * see), whereas gethostname() kindly truncates it for me.
	 */
	{
	struct utsname name;

	uname(&name);
	strcpy(hostname, name.nodename);
	}
#else
	gethostname(hostname, sizeof(hostname));
#endif
#endif /* UNIXCONN */
	/*
	 * copy the client args.
	 */
	if (argc == 0 || (**argv != '/' && **argv != '.' && !isalpha(**argv))) {
		for (ptr = default_client; *ptr; )
			*cptr++ = *ptr++;
		strcpy(client_display, hostname);
		strcat(client_display, default_display);
		*cptr++ = client_display;
#ifdef sun
		/* 
		 * If running on a sun, and if WINDOW_PARENT isn't defined, 
		 * that means SunWindows isn't running, so we should pass 
		 * the -C flag to xterm so that it sets up a console.
		 */
		if ( getenv("WINDOW_PARENT") == NULL )
		    *cptr++ = "-C";
#endif /* sun */
	}
	while (argc && strcmp(*argv, "--")) {
		*cptr++ = *argv++;
		argc--;
	}
	*cptr = NULL;
	if (argc) {
		argv++;
		argc--;
	}

	/*
	 * Copy the server args.
	 */
	if (argc == 0 || (**argv != '/' && **argv != '.' && !isalpha(**argv))) {
		*sptr++ = default_server;
	} else {
		*sptr++ = *argv++;
		argc--;
	}
	if (argc > 0 && (argv[0][0] == ':' && isdigit(argv[0][1])))
		displayNum = *argv;
	else
		displayNum = *sptr++ = default_display;
	while (--argc >= 0)
		*sptr++ = *argv++;
	*sptr = NULL;

	/*
	 * Start the server and client.
	 */
	signal(SIGQUIT, sigCatch);
	signal(SIGINT, sigCatch);
#ifdef SYSV
	signal(SIGALRM, sigAlarm);
#endif /* SYSV */
	if ((serverpid = startServer(server)) > 0
	 && (clientpid = startClient(client)) > 0) {
		pid = -1;
		while (pid != clientpid && pid != serverpid)
			pid = wait(NULL);
	}
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);

	shutdown(serverpid, clientpid);

	if (serverpid < 0 || clientpid < 0)
		exit(ERR_EXIT);
	exit(OK_EXIT);
}


/*
 *	waitforserver - wait for X server to start up
 */

waitforserver(serverpid)
	int	serverpid;
{
	int	ncycles	 = 120;		/* # of cycles to wait */
	int	cycles;			/* Wait cycle count */
	char	display[100];		/* Display name */

	strcpy(display, hostname);
	strcat(display, displayNum);
	for (cycles = 0; cycles < ncycles; cycles++) {
		if (xd = XOpenDisplay(display)) {
			return(TRUE);
		}
		else {
			if (!processTimeout(serverpid, 1, "server to start"))
				return(FALSE);
		}
	}

	return(FALSE);
}

/*
 * return TRUE if we timeout waiting for pid to exit, FALSE otherwise.
 */
processTimeout(pid, timeout, string)
	int	pid, timeout;
	char	*string;
{
	int	i = 0, pidfound = -1;
	static char	*laststring;

	for (;;) {
#ifdef SYSV
		alarm(1);
		if ((pidfound = wait(NULL)) == pid)
			break;
		alarm(0);
#else /* SYSV */
		if ((pidfound = wait3(&status, WNOHANG, NULL)) == pid)
			break;
#endif /* SYSV */
		if (timeout) {
			if (i == 0 && string != laststring)
				fprintf(stderr, "\nwaiting for %s ", string);
			else
				fprintf(stderr, ".");
			fflush(stderr);
		}
		if (timeout)
			sleep (1);
		if (++i > timeout)
			break;
	}
	laststring = string;
	return( pid != pidfound );
}

Error(fmt, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
	char	*fmt;
{
	extern char	*sys_errlist[];

	fprintf(stderr, "%s:  ", program);
	if (errno > 0 && errno < sys_nerr)
	  fprintf (stderr, "%s (errno %d):  ", sys_errlist[errno], errno);
	fprintf(stderr, fmt, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
}

Fatal(fmt, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
	char	*fmt;
{
	Error(fmt, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	exit(ERR_EXIT);
}

startServer(server)
	char *server[];
{
	int	serverpid;

	serverpid = vfork();
	switch(serverpid) {
	case 0:
		close(0);
		close(1);

		/*
		 * don't hang on read/write to control tty
		 */
#ifdef SIGTTIN
		(void) signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTTOU
		(void) signal(SIGTTOU, SIG_IGN);
#endif

		/*
		 * prevent server from getting sighup from vhangup()
		 * if client is xterm -L
		 */
		setpgrp(0,0);

		execvp(server[0], server);
		Error ("no server \"%s\" in PATH\n", server[0]);
		{
		    char **cpp;

		    fprintf (stderr,
"\nUse the -- option, or make sure that %s is in your path and\n",
			     bindir);
		    fprintf (stderr,
"that \"%s\" is a program or a link to the right type of server\n",
			     server[0]);
		    fprintf (stderr,
"for your display.  Possible server names include:\n\n");
		    for (cpp = server_names; *cpp; cpp++) {
			fprintf (stderr, "    %s\n", *cpp);
		    }
		    fprintf (stderr, "\n");
		}
		exit (ERR_EXIT);

		break;
	case -1:
		break;
	default:
		/*
		 * don't nice server
		 */
#ifdef PRIO_PROCESS
		setpriority( PRIO_PROCESS, serverpid, -1 );
#endif

		errno = 0;
		if (! processTimeout(serverpid, 0, "")) {
			serverpid = -1;
			break;
		}
		/*
		 * kludge to avoid race with TCP, giving server time to
		 * set his socket options before we try to open it
		 */
		sleep(5);

		if (waitforserver(serverpid) == 0) {
			Error("unable to connect to server\n");
			shutdown(serverpid, -1);
			serverpid = -1;
		}
		break;
	}

	return(serverpid);
}

startClient(client)
	char *client[];
{
	int	clientpid;

	if ((clientpid = vfork()) == 0) {
		setuid(getuid());
		setpgrp(0, getpid());
		execvp(client[0], client);
		Error ("no program named \"%s\" in PATH\n", client[0]);
		fprintf (stderr,
"\nSpecify a program on the command line or make sure that %s\n", bindir);
		fprintf (stderr,
"is in your path.\n");
		fprintf (stderr, "\n");
		exit (ERR_EXIT);
	}
	return (clientpid);
}

#ifdef SYSV
#define killpg(pgrp, sig) kill(-(pgrp), sig)
#endif /* SYSV */

static jmp_buf close_env;

static int ignorexio (dpy)
    Display *dpy;
{
    fprintf (stderr, "%s:  connection to server lost.\n", program);
    longjmp (close_env, 1);
    return;
}

static
shutdown(serverpid, clientpid)
	int	serverpid, clientpid;
{
	/* have kept display opened, so close it now */
	if (clientpid > 0) {
		XSetIOErrorHandler (ignorexio);
		if (! setjmp(close_env)) {
		    XCloseDisplay(xd);
		}

		/* HUP all local clients to allow them to clean up */
		errno = 0;
		if ((killpg(clientpid, SIGHUP) != 0) &&
		    (errno != ESRCH))
			Error("can't killpg(%d, SIGHUP) for client\n",
				clientpid);
	}

	if (serverpid < 0)
		return;
	errno = 0;
	if (kill(serverpid, SIGTERM) < 0) {
		if (errno == EPERM)
			Fatal("Can't kill server\n");
		if (errno == ESRCH)
			return;
	}
	if (! processTimeout(serverpid, 10, "server to terminate"))
		return;

	fprintf(stderr, "timeout... send SIGKILL");
	fflush(stderr);
	errno = 0;
	if (kill(serverpid, SIGKILL) < 0) {
		if (errno == ESRCH)
			return;
	}
	if (processTimeout(serverpid, 3, "server to die"))
		Fatal("Can't kill server\n");
}
