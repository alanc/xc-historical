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

#include	"Xlib.h"
#include	"Xutil.h"
#include	"Xatom.h"
#include	"xtest.h"
#include	"xtestlib.h"
#include	"pixval.h"


/*
 * Get the current server time. Use a property attached to the root window
 * of the display called XT_TIMESTAMP and replaces it with 42 (32-bits).
 * The PropertyNotify event that's generated supplies the time stamp returned.
 * The event mask on the root window is restored to its initial state.
 * Returns CurrentTime on error (as well as deleting the test).
 */
Time
gettime(disp)
	Display	*disp;
{
	Window 	root;
	static Atom	prop = None;
	static int	data = 42;
	static char	*name = XT_TIMESTAMP;
	XWindowAttributes wattr;
	XEvent	ev;
	int	i;

	root = XDefaultRootWindow(disp);

	if (XGetWindowAttributes(disp, root, &wattr) == False) {
		delete("gettime: XGetWindowAttributes on root failed.");
		return CurrentTime;
	}

	if (prop==None && (prop=XInternAtom(disp, name, False)) == None) {
		delete("gettime: XInternAtom of '%s' failed.", name);
		return CurrentTime;
	}

	XSelectInput(disp, root, wattr.your_event_mask | PropertyChangeMask);

	XChangeProperty(disp, root, prop, XA_INTEGER, 32, PropModeReplace,
		(unsigned char *)&data, 1);

	for (i=0; i<10; i++, sleep(2)) {
		if (XCheckWindowEvent(disp, root, PropertyChangeMask, &ev))
			break;
	}

	if (i >= 10) {
		delete("gettime: Didn't receive expected PropertyNotify event");
		return CurrentTime;
	}

	XSelectInput(disp, root, wattr.your_event_mask);

	return ev.xproperty.time;
}

