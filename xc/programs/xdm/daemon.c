/*
 * xdm - display manager daemon
 *
 * $XConsortium: daemon.c,v 1.12 94/02/04 11:35:58 gildea Exp $
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

#include <X11/Xos.h>

#if defined(SVR4) || defined(USG)
#include <termios.h>
#else
#include <sys/ioctl.h>
#endif
#ifdef __osf__
#define setpgrp setpgid
#endif
#ifdef hpux
#include <sys/ptyio.h>
#endif
#include <errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif
#include <sys/types.h>
#ifdef X_NOT_POSIX
#define Pid_t int
#else
#define Pid_t pid_t
#endif

extern void exit ();

BecomeOrphan ()
{
    Pid_t child_id;
    int stat;

    /*
     * fork so that the process goes into the background automatically. Also
     * has a nice side effect of having the child process get inherited by
     * init (pid 1).
     * Separate the child into its own process group before the parent
     * exits.  This eliminates the possibility that the child might get
     * killed when the init script that's running xdm exits.
     */

    child_id = fork();
    switch (child_id) {
    case 0:
	/* child */
	break;
    case -1:
	/* error */
	LogError("daemon fork failed, errno = %d\n", errno);
	break;

    default:
	/* parent */

#if defined(SVR4)
	stat = setpgid(child_id, child_id);
	/* This gets error EPERM.  Why? */
#else
#if defined(SYSV)
	stat = 0;	/* don't know how to set child's process group */
#else
	stat = setpgrp(child_id, child_id);
	if (stat != 0)
	    LogError("setting process grp for daemon failed, errno = %d\n",
		     errno);
#endif
#endif
	exit (0);
    }
}

BecomeDaemon ()
{
    register int i;

    /*
     * Close standard file descriptors and get rid of controlling tty
     */

#if defined(SYSV) || defined(SVR4)
    setpgrp ();
#else
    setpgrp (0, getpid());
#endif

    close (0); 
    close (1);
    close (2);

#ifndef SYSV386
    if ((i = open ("/dev/tty", O_RDWR)) >= 0) {	/* did open succeed? */
#if defined(USG) && defined(TCCLRCTTY)
	int zero = 0;
	(void) ioctl (i, TCCLRCTTY, &zero);
#else
#if (defined(SYSV) || defined(SVR4)) && defined(TIOCTTY)
	int zero = 0;
	(void) ioctl (i, TIOCTTY, &zero);
#else
	(void) ioctl (i, TIOCNOTTY, (char *) 0);    /* detach, BSD style */
#endif
#endif
	(void) close (i);
    }
#endif /* !SYSV386 */

    /*
     * Set up the standard file descriptors.
     */
    (void) open ("/", O_RDONLY);	/* root inode already in core */
    (void) dup2 (0, 1);
    (void) dup2 (0, 2);
}
