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
#include	"pixval.h"


/*
 * Get the size of a drawable.  Just uses XGetGeometry but avoids all
 * the other information that you get with that.
 * Either of widthp or heightp can be NULL.
 */
void
getsize(disp, d, widthp, heightp)
Display	*disp;
Drawable	d;
unsigned int 	*widthp;
unsigned int 	*heightp;
{
unsigned int 	dummy;
Window 	root;
int 	x;
int 	y;
unsigned int 	border;
unsigned int 	depth;

	XGetGeometry(disp, d, &root, &x, &y,
		widthp? widthp: &dummy,
		heightp? heightp: &dummy, &border, &depth);
}

/*
 * Returns the depth of the given drawable.
 */
unsigned int
getdepth(disp, d)
Display	*disp;
Drawable	d;
{
Window 	root;
int 	x;
int 	y;
unsigned int 	width;
unsigned int 	height;
unsigned int 	border;
unsigned int 	depth;

	XGetGeometry(disp, d, &root, &x, &y, &width, &height,
		&border, &depth);

	return(depth);
}
