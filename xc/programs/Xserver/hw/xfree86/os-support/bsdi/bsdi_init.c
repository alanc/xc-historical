/* $XConsortium: bsdi_init.c,v 1.2 94/10/12 20:46:00 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsdi/bsdi_init.c,v 3.0 1994/09/23 10:24:39 dawes Exp $ */
/*
 * Copyright 1992 by Rich Murphey <Rich@Rice.edu>
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Rich Murphey and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Rich Murphey and
 * David Wexelblat make no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * RICH MURPHEY AND DAVID WEXELBLAT DISCLAIM ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL RICH MURPHEY OR DAVID WEXELBLAT BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */


#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"

static Bool KeepTty = FALSE;

void xf86OpenConsole()
{
    int i, fd;

    if (serverGeneration == 1)
    {
	/* check if we're run with euid==0 */
	if (geteuid() != 0)
	{
	    FatalError("xf86OpenConsole: Server must be suid root\n");
	}

	if (!KeepTty)
	{
	    /*
	     * detaching the controlling tty solves problems of kbd character
	     * loss.  This is not interesting for CO driver, because it is 
	     * exclusive.
	     */
	    setpgrp(0, getpid());
	    if ((i = open("/dev/tty",O_RDWR)) >= 0)
	    {
		ioctl(i,TIOCNOTTY,(char *)0);
		close(i);
	    }
	}

	if ((xf86Info.consoleFd = open("/dev/kbd", O_RDWR|O_NDELAY,0)) < 0)
	{
	    FatalError("xf86OpenConsole: Cannot open /dev/kbd (%s)\n",
		       strerror(errno));
	}
	if ((xf86Info.screenFd = open("/dev/vga", O_RDWR|O_NDELAY,0)) < 0)
	{
	    FatalError("xf86OpenConsole: Cannot open /dev/vga (%s)\n",
		       strerror(errno));
	}

	xf86Config(FALSE); /* Read XF86Config */

	if (ioctl(xf86Info.consoleFd, PCCONIOCRAW, 0) < 0)
	{
	    FatalError("%s: PCCONIOCRAW failed (%s)\n", 
		       "xf86OpenConsole", strerror(errno));
	}
    }
    return;
}

void xf86CloseConsole()
{
    ioctl (xf86Info.consoleFd, PCCONIOCCOOK, 0);

    if (xf86Info.screenFd != xf86Info.consoleFd)
    {
	close(xf86Info.screenFd);
    }
    close(xf86Info.consoleFd);
    return;
}

int xf86ProcessArgument (argc, argv, i)
int argc;
char *argv[];
int i;
{
	/*
	 * Keep server from detaching from controlling tty.  This is useful 
	 * when debugging (so the server can receive keyboard signals.
	 */
	if (!strcmp(argv[i], "-keeptty"))
	{
		KeepTty = TRUE;
		return(1);
	}
	return(0);
}

void xf86UseMsg()
{
	ErrorF("-keeptty               ");
	ErrorF("don't detach controlling tty (for debugging only)\n");
	return;
}
