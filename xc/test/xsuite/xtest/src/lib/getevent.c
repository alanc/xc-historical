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
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Check if there are events on the queue and if so return the
 * first one.
 * Returns the number of events that were in the queue on calling.
 */
int
getevent(disp, event)
Display	*disp;
XEvent	*event;
{
int 	len;

	if ((len = XQLength(disp)) == 0)
		return(0);

	XNextEvent(disp, event);

	debug(2, "Event %s", eventname(event->type));

	return(len);
}
