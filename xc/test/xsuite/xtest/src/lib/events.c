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
/*LINTLIBRARY*/

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	<unistd.h>

/*
 * Return in x_root_return and y_root_return the specified coordinates 
 * transformed from the coordinate space of the given window to that
 * of the specified window.  It is assumed that the two windows are on
 * the same screen.
 *
 * Can be used in conjunction with ROOTCOORDSET macro.
 */
void
rootcoordset(display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return)
Display	*display;
Window	src_w;
Window	dest_w;
int	src_x;
int	src_y;
int	*dest_x_return;
int	*dest_y_return;
{
	Window	window;

	(void) XTranslateCoordinates(display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, &window);
}

/*
 * Set the serial member of the specified event to NextRequest.
 */
void
serialset(display, event)
Display	*display;
XEvent	*event;
{
	event->xany.serial = NextRequest(display);
}

/*
 * Compare serial field of two events.  Return True if they compare
 * equal, else return False.
 */
Bool
serialtest(good, ev)
XEvent	*good;
XEvent	*ev;
{
	if (ev->xany.serial != good->xany.serial) {
		report("Checking event type %s", eventname(good->type));
		report("found error in %s field, was %ld, expecting %ld","serial",ev->xany.serial, good->xany.serial);
		return(False);
	}
	return(True);
}
