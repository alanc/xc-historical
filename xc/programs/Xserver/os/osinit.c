/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Header: osinit.c,v 1.15 88/07/20 13:53:26 xguest Exp $ */
#include "os.h"
#include "opaque.h"
#undef NULL
#include <dbm.h>
#undef NULL
#include <stdio.h>
#ifndef MAXPATHLEN
/*
 * just to get MAXPATHLEN.  Define it elsewhere if you need to
 * avoid these files.
 */
#include <sys/types.h>
#include <sys/param.h>
#endif

#ifndef ADMPATH
#define ADMPATH "/usr/adm/X%smsgs"
#endif

int	havergb = 0;
extern char *display;

OsInit()
{
    static Bool been_here = FALSE;
    char fname[MAXPATHLEN];

    /* hack test to decide where to log errors */

    if (!been_here) {
	if (write (2, fname, 0)) 
	{
	    long t; 
	    char *ctime();
	    fclose(stdin);
	    fclose(stdout);
	    sprintf (fname, ADMPATH, display);
	    if (!freopen (fname, "a+", stderr))
		freopen ("/dev/null", "w", stderr);
#ifdef macII
	    {
	    static char buf[BUFSIZ];
	    setvbuf (stderr, buf, _IOLBF, BUFSIZ);
	    }
#else
	    setlinebuf(stderr);
#endif
	    time (&t);
	    fprintf (stderr, "start %s", ctime(&t));
	}

	if (getpgrp (0) == 0)
	    setpgrp (0, getpid ());

	been_here = TRUE;
    }

    if(!havergb)
        if(dbminit (rgbPath) == 0)
	    havergb = 1;
        else
	    ErrorF( "Couldn't open RGB_DB '%s'\n", rgbPath );
}
