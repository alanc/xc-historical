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
 * Create, map and wait for a child window covering the
 * specified area of the parent window.
 * If either width or height are zero then the child extends to the left
 * or bottom edge of the parent window.
 * If ap is NULL then the child will exactly cover the parent window.
 */
Window
crechild(disp, w, ap)
Display	*disp;
Window	w;
struct	area	*ap;
{
Window	child;
XEvent	event;
XWindowAttributes	atts;

	child = creunmapchild(disp, w, ap);

	if (isdeleted())
		return None; /* avoid waiting for events that won't happen. */

	XSync(disp, True);
	XSelectInput(disp, child, ExposureMask);
	XMapWindow(disp, child);

	XGetWindowAttributes(disp, child, &atts);
	if (XPending(disp) && atts.map_state == IsViewable)
		XWindowEvent(disp, child, ExposureMask, &event);

	XSelectInput(disp, child, NoEventMask);

	return(child);
}

/*
 * Create, without mapping, a child window covering the
 * specified area of the parent window.
 * If either width or height are zero then the child extends to the left
 * or bottom edge of the parent window.
 * If ap is NULL then the child will exactly cover the parent window.
 */
Window
creunmapchild(disp, w, ap)
Display	*disp;
Window	w;
struct	area	*ap;
{
Window	child;
struct area ar;

	if(ap == (struct area *) 0) {	
		ar.width = ar.height = ar.x = ar.y = 0;
		ap = &ar;
	}

	/*
	 * If either width or height is 0, then use size of parent window.
	 * This ensures that the window covers the rest of the
	 * parent without sticking off the edge.
	 */
	if (ap->width == 0) {
		getsize(disp, w, &ap->width, (unsigned int*)0);
		ap->width -= ap->x;
	}
	if (ap->height == 0) {
		getsize(disp, w, (unsigned int*)0, &ap->height);
		ap->height -= ap->y;
	}

	child = XCreateSimpleWindow(disp, w, ap->x, ap->y, ap->width, ap->height, 0, W_FG, W_BG);

	return(child);
}
