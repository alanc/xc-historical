#ifndef lint
static char rcs_id[] = "$Header: xrdb.c,v 11.6 87/12/21 12:05:15 jim Locked $";
#endif

/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission.
 */

/*
 * this program is used to load, or dump the resource manager database
 * in the server.
 *
 * Author: Jim Gettys, August 28, 1987
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <ctype.h>

#define MAXRDB 100000
#define DBOK 0

static resource_buffer[MAXRDB];		/* what a crock, should be dynamic */
/* note that there is an implicit limit of BUFSIZ characters per line
   down in SanityCheck */

char *SanityCheck();

char *ProgramName;

main (argc, argv)
    int argc;
    char **argv;
{
    Display * dpy;
    int i;
    char *displayname = NULL;
    char *filename = NULL;
    FILE *fp;
    int nbytes;
    char *bptr, *endptr;
    int printit = 0;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {	/* get args */
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
		case '\0':		/* - meaning use stdin */
		    filename = NULL;
		    break;
		case 'q':
		    printit = 1;
		    break;
		case 'd':		/* -d display */
		    if (++i >= argc) goto usage;
		    displayname = argv[i];
		    break;
		case 'g':		/* -g geometry, for consistency only */
		    if (++i >= argc) goto usage;
		    /* ignore */
		    break;
		default:
		  usage:
		    (void) fprintf (stderr, 
	    	     "usage:  %s [-q] [-d display] [-g geometry] [filename]\n",
			     ProgramName);
		    (void) fprintf (stderr, 
 "\nThe geometry argument is ignored as is for consistency only.  If no\n");
		    (void) fprintf (stderr,
		    "filename is given, the standard input will be read.\n");
		    exit (1);
	    }						/* end switch */
	} else {
	    if (index (arg, ':') != NULL) displayname = arg;	/* old style */
	    else filename = arg;
	} 						/* end if */
    }							/* end for */

    /* Open display  */
    if (!(dpy = XOpenDisplay (displayname))) {
	(void) fprintf (stderr, "%s: Can't open display '%s'\n",
		ProgramName, XDisplayName (displayname));
	exit (1);
    }

    if (printit == 1) {
	/* user wants to print contents */
	if (dpy->xdefaults)
	    fputs(dpy->xdefaults, stdout);
	}
    else {
	if (filename != NULL) {
		fp = freopen (filename, "r", stdin);
		if (fp == NULL) {
			fprintf(stderr, "%s: can't open file '%s'\n", 
				argv[0], filename);
			exit(1);
			}
		}
	nbytes = fread(resource_buffer, sizeof(char), MAXRDB, stdin);
	if ((bptr = SanityCheck (resource_buffer)) != DBOK) {
		fprintf(stderr, "%s: database fails sanity check \n'%s'\n", 
			argv[0], bptr);
		exit(1);
		}
	XChangeProperty (dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER,
		XA_STRING, 8, PropModeReplace, resource_buffer, nbytes);
	}

    XCloseDisplay(dpy);
    exit (0);
}

char *getline(buffer, buf)
	register char *buffer;
	register char *buf;
{
	register char c;
	while (*buffer != '\0') {
		c = *buffer++;
		if (c == '\n') {
			*buf = '\0';
			return (buffer);
			}
		if ( ! isspace(c)) *buf++ = c;
	}
	return (NULL);
}

/*
 * does simple sanity check on data base.  Lines can either be
 * commented, be all white space, or must contain ':'.
 */
char *SanityCheck (buffer)
	char *buffer;
{
    static char buf[BUFSIZ];
    register char *s;
    char *b = buffer;
    register char *i;
    while (1) {
	if ((b = getline(b, buf)) == NULL) return (DBOK);
	if (buf[0] == '#' || buf[0] == '\0') continue;
	if ((i = index (buf, ':')) == NULL) return buf;
	}
}
