/*
 * xdm - display manager daemon
 *
 * $XConsortium: daemon.c,v 1.1 88/09/23 14:21:53 keith Exp $
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
#include <sys/ioctl.h>
#ifdef SYSV
#include <sys/ptyio.h>
#endif

char *root = "/";			/* have only one copy of string */
char *devtty = "/dev/tty";		/* ditto */

BecomeDaemon ()
{
    register int i;

    /*
     * fork so that the process goes into the background automatically. Also
     * has a nice side effect of having the child process get inherited by
     * init (pid 1).
     */

    if (fork ())			/* if parent */
      exit (0);				/* then no more work to do */

    /*
     * Close standard file descriptors and get rid of controlling tty
     */

    setpgrp (0, getpid());

    close (0); close (1); close (2);

    if ((i = open (devtty, O_RDWR)) >= 0) {	/* did open succeed? */
#ifdef SYSV
	int zero = 0;
	(void) ioctl (i, TIOCTTY, &zero);
#else
	(void) ioctl (i, TIOCNOTTY, 0);		/* detach */
#endif
	(void) close (i);
    }

    /*
     * Set up the standard file descriptors.
     */

    (void) open ("/", O_RDONLY);	/* root inode already in core */
    (void) dup2 (0, 1);
    (void) dup2 (0, 2);
}
