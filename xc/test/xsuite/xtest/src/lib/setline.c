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
#include	"pixval.h"

/*
 * Set the line width, without affecting anything else.
 */
void
setwidth(disp, gc, width)
Display	*disp;
GC		gc;
unsigned int 	width;
{
XGCValues	gcv;

	gcv.line_width = width;
	XChangeGC(disp, gc, GCLineWidth, &gcv);
}

/*
 * Set the capstyle without affecting anything else.
 */
void
setcapstyle(disp, gc, capstyle)
Display	*disp;
GC		gc;
int 	capstyle;
{
XGCValues	gcv;

	gcv.cap_style = capstyle;
	XChangeGC(disp, gc, GCCapStyle, &gcv);
}

/*
 * Set the line style without affecting anything else.
 */
void
setlinestyle(disp, gc, linestyle)
Display	*disp;
GC		gc;
int 	linestyle;
{
XGCValues	gcv;

	gcv.line_style = linestyle;
	XChangeGC(disp, gc, GCLineStyle, &gcv);
}

/*
 * Set the join style without affecting anything else.
 */
void
setjoinstyle(disp, gc, joinstyle)
Display	*disp;
GC		gc;
int 	joinstyle;
{
XGCValues	gcv;

	gcv.join_style = joinstyle;
	XChangeGC(disp, gc, GCJoinStyle, &gcv);
}
