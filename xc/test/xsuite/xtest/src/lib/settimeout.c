/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */

#include	"xtest.h"
#include	<signal.h>

static int 	Alarm_set;

#ifdef OLDSIGNALS
static void	(*osig)();
#else
static struct	sigaction	oact;
#endif

static void
alrm_exit(a)
int a;
{
	exit(TIMEOUT_EXIT);
}

/*
 * Set up a timeout to cause the process to exit in to seconds.  This should
 * only be called from a child process normally.
 */
void
settimeout(to)
int 	to;
{
#ifndef OLDSIGNALS
struct	sigaction	act;
#endif

	Alarm_set = 1;

#if OLDSIGNALS
	osig = signal(SIGALRM, alrm_exit);
#else
	act.sa_handler = alrm_exit;
	act.sa_flags = 0;
	(void) sigemptyset(&act.sa_mask);

	if (sigaction(SIGALRM, &act, &oact) == -1)
		delete("Could not set signal handler in settimeout");
#endif
	(void) alarm((unsigned long)to);
}

/*
 * Clear a previously set timeout.  Return the amount of time remaining.
 */
unsigned long
cleartimeout()
{
unsigned long	timeleft;

	if (Alarm_set == 0)
		return 0;

	timeleft = alarm(0);
#if OLDSIGNALS
	(void) signal(SIGALRM, osig);
#else
	if (sigaction(SIGALRM, &oact, (struct sigaction *)0) == -1)
		delete("Could not reset signal handler in cleartimeout");
#endif
	return(timeleft);
}

