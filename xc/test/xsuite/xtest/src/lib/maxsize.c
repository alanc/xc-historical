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
#include	"xtestlib.h"
#include	"pixval.h"

/* func. to give min of 3 vars */
static
int min3(a, b, c)
	int a,b,c;
{
	if (a < b)
		b = a;
	if (b < c)
		c = b;
	return c;
}

/*
 * Get the number of cells in a colourmap given the vp.
 */
int
maxsize(vp)
	XVisualInfo	*vp;
{
	int	 	r,g,b;
	unsigned long 	mask;

	if (vp->class != DirectColor && vp->class != TrueColor)
		return vp->colormap_size;

	for(r=0, mask=vp->red_mask; mask; mask >>= 1) {
		if (mask & 0x1)
			r++;
	}
	for(g=0, mask=vp->green_mask; mask; mask >>= 1) {
		if (mask & 0x1)
			g++;
	}
	for(b=0, mask=vp->blue_mask; mask; mask >>= 1) {
		if (mask & 0x1)
			b++;
	}
	return (1 << min3(r,g,b));
}
