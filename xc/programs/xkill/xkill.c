/*
 * xkill - simple program for destroying unwanted clients
 *
 * $XHeader$
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
 * Author:  Jim Fulton, MIT X Consortium; Dana Chee, Bellcore
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>

char *ProgramName;

XID parse_id(), get_window_id();

usage ()
{
    fprintf (stderr, "usage:  %s [-display displayname] [-id resource]\n",
	     ProgramName);
    fprintf (stderr, "\n");
    exit (1);
}

main (argc, argv)
    int argc;
    char *argv[];
{
    Display *dpy;
    char *displayname = NULL;
    XID id = None;
    int i;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'd':			/* -display displayname */
		if (++i >= argc) usage ();
		displayname = argv[i];
		continue;
	      case 'i':			/* -id resourceid */
		if (++i >= argc) usage ();
		id = parse_id (argv[i]);
		continue;
	      default:
		usage ();
	    }
	} else {
	    usage ();
	}
    }					/* end for */

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\"\n",
		 ProgramName, XDisplayName (displayname));
	exit (1);
    }

    if (id == None) {
	id = get_window_id (dpy);
    }

    if (id != None) {
	printf ("%s:  killing creator of resource 0x%lx\n", ProgramName, id);
	XSync (dpy, 0);			/* give xterm a chance */
	XKillClient (dpy, id);
	XSync (dpy, 0);
    }

    XCloseDisplay (dpy);
    exit (0);
}


XID parse_id (s)
    char *s;
{
    XID retval = None;
    char *fmt = "%ld";			/* since XID is long */

    if (s) {
	if (*s == '0') s++, fmt = "%lo";
	if (*s == 'x' || *s == 'X') s++, fmt = "%lx";
	sscanf (s, fmt, &retval);
    }
    return (retval);
}

XID get_window_id (dpy)
    Display *dpy;
{
    Status status;
    XEvent event;
    Cursor cursor;
    int screen;
    Window root;

    screen = DefaultScreen (dpy);
    root = RootWindow (dpy, screen);
    cursor = XCreateFontCursor (dpy, XC_pirate);
    if (cursor == None) {
	fprintf (stderr, "%s:  unable to create selection cursor\n",
		 ProgramName);
	XCloseDisplay (dpy);
	exit (1);
    }

    printf ("Select a window whose client you wish to kill....\n");
    XSync (dpy, 0);			/* give xterm a chance */

    status = XGrabPointer (dpy, root, False,
			   (ButtonPressMask | ButtonReleaseMask),
			   GrabModeSync, GrabModeAsync, None, cursor,
			   CurrentTime);
    if (status != GrabSuccess) {
	fprintf (stderr, "%s:  unable to grab cursor\n", ProgramName);
	XCloseDisplay (dpy);
	exit (1);
    }
    XAllowEvents (dpy, SyncPointer, CurrentTime);
    XWindowEvent (dpy, root, (ButtonPressMask | ButtonReleaseMask), &event);
    XUngrabPointer (dpy, CurrentTime);
    XFreeCursor (dpy, cursor);
    XSync (dpy, 0);

    return (event.xbutton.subwindow);
}

