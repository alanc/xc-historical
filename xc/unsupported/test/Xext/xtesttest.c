/* $XConsortium: xtesttest.c,v 1.2 92/01/27 11:41:22 rws Exp $ */
/*

Copyright 1992 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/extensions/XTest.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

char *ProgramName;

static void usage ()
{
    fprintf (stderr, "usage:  %s [-display dpy]\n", ProgramName);
    exit (1);
}

main (argc, argv)
    int argc;
    char **argv;
{
    char *displayname = NULL;
    Display *dpy;
    int i;    
    int event_base, error_base;
    int major, minor;
    unsigned long req;
    GC gc;
    XID gid;
    Window w;
    XSetWindowAttributes swa;
    int key, minkey, maxkey;
    XEvent ev;

    ProgramName = argv[0];
    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'd':			/* -display dpy */
		if (++i >= argc) usage ();
		displayname = argv[i];
		continue;
	    }
	}
	usage ();
    }

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\"\n",
		 ProgramName, XDisplayName(displayname));
	exit (1);
    }

    if (!XTestQueryExtension (dpy, &event_base, &error_base, &major, &minor)) {
	fprintf (stderr, 
	 "%s:  XTest extension not supported on server \"%s\"\n",
		 ProgramName, DisplayString(dpy));
	XCloseDisplay(dpy);
	exit (1);
    }
    printf ("XTest information for server \"%s\":\n",
	    DisplayString(dpy));
    printf ("  Major version:       %d\n", major);
    printf ("  Minor version:       %d\n", minor);
    printf ("  First event number:  %d\n", event_base);
    printf ("  First error number:  %d\n", error_base);

    
    swa.override_redirect = True;
    swa.cursor = XCreateFontCursor(dpy, XC_boat);
    swa.event_mask = KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask;
    w = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, 100, 100, 0, 0,
		      InputOnly, CopyFromParent,
		      CWEventMask|CWOverrideRedirect|CWCursor, &swa);
    XMapWindow(dpy, w);
    if (!XTestCompareCursorWithWindow(dpy, w, swa.cursor))
	printf("error: window cursor is not the expected one\n");
    XTestFakeMotionEvent(dpy, DefaultScreen(dpy), 10, 10, 0);
    if (!XTestCompareCurrentCursorWithWindow(dpy, w))
	printf("error: window cursor is not the displayed one\n");
    XUndefineCursor(dpy, w);
    if (!XTestCompareCursorWithWindow(dpy, w, None))
	printf("error: window cursor is not the expected None\n");
    XSync(dpy, True);
    XDisplayKeycodes(dpy, &minkey, &maxkey);
    key = XKeysymToKeycode(dpy, XK_a);
    if (!key)
	key = minkey;
    XTestFakeKeyEvent(dpy, key, True, 0);
    XNextEvent(dpy, &ev);
    if (ev.type != KeyPress ||
	ev.xkey.keycode != key ||
	ev.xkey.x_root != 10 ||
	ev.xkey.y_root != 10)
	printf("error: bad event received for key press\n");
    XTestFakeKeyEvent(dpy, key, False, 0);
    XNextEvent(dpy, &ev);
    if (ev.type != KeyRelease ||
	ev.xkey.keycode != key ||
	ev.xkey.x_root != 10 ||
	ev.xkey.y_root != 10)
	printf("error: bad event received for key release\n");
    XTestFakeButtonEvent(dpy, 1, True, 0);
    XNextEvent(dpy, &ev);
    if (ev.type != ButtonPress ||
	ev.xbutton.button != 1 ||
	ev.xbutton.x_root != 10 ||
	ev.xbutton.y_root != 10)
	printf("error: bad event received for button press\n");
    XTestFakeMotionEvent(dpy, DefaultScreen(dpy), 9, 8, 0);
    XNextEvent(dpy, &ev);
    if (ev.type != MotionNotify ||
	ev.xmotion.x_root != 9 ||
	ev.xmotion.y_root != 8)
	printf("error: bad event received for motion\n");
    XTestFakeButtonEvent(dpy, 1, False, 0);
    XNextEvent(dpy, &ev);
    if (ev.type != ButtonRelease ||
	ev.xbutton.button != 1 ||
	ev.xbutton.x_root != 9 ||
	ev.xbutton.y_root != 8)
	printf("error: bad event received for button release\n");
    gc = DefaultGC(dpy, DefaultScreen(dpy));
    req = NextRequest(dpy);
    XDrawPoint(dpy, w, gc, 0, 0);
    if (!XTestDiscard(dpy) || req != NextRequest(dpy))
	printf("error: XTestDiscard failed to discard an XDrawPoint\n");
    gid = XGContextFromGC(gc);
    XTestSetGContextOfGC(gc, 3L);
    if (XGContextFromGC(gc) != 3L)
	printf("error: XTestSetGContextOfGC failed\n");
    XTestSetGContextOfGC(gc, gid);
    if (XGContextFromGC(gc) != gid)
	printf("error: XTestSetGContextOfGC failed\n");
    XCloseDisplay (dpy);
    exit (0);
}
