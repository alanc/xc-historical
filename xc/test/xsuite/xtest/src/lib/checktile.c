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
 * Check that the area is filled with the given tile pm.
 * a NULL area pointer means check the whole window.
 * xorig and yorig specify the tiling origin which is otherwise
 * assumed to start at the origin of the drawable (not the area)
 * The width and height if zero default to the rest of the drawable.
 */
Status
checktile(disp, d, ap, xorig, yorig, pm)
Display	*disp;
Drawable d;
struct	area	*ap;
int 	xorig;
int 	yorig;
Pixmap	pm;
{
XImage	*im;
XImage	*pmim;
unsigned int 	pmwidth;
unsigned int 	pmheight;
register int 	x;
register int 	y;
unsigned long	ppix;
unsigned long	ipix;
int 	ret;
struct	area	area;

	if (ap == 0) {
		ap = &area;
		ap->x = 0;
		ap->y = 0;
		getsize(disp, d, &area.width, &area.height);
	}
	if (ap->width == 0) {
		getsize(disp, d, &ap->width, (unsigned*)0);
		ap->width -= xorig;
	}
	if (ap->height == 0) {
		getsize(disp, d, (unsigned*)0, &ap->height);
		ap->height -= yorig;
	}

	getsize(disp, pm, &pmwidth, &pmheight);
	im = XGetImage(disp, d, ap->x, ap->y, ap->width, ap->height,
		AllPlanes, ZPixmap);
	pmim = XGetImage(disp, pm, 0, 0, pmwidth, pmheight, AllPlanes, ZPixmap);

	xorig = ap->x - xorig;
	while (xorig < 0)
		xorig += pmwidth;
	yorig = ap->y - yorig;
	while (yorig < 0)
		yorig += pmheight;

	ret = True;
	for (y = 0; y < ap->height; y++) {
		for (x = 0; x < ap->width; x++) {
			ppix = XGetPixel(pmim, (xorig+x)%pmwidth, (yorig+y)%pmheight);
			ipix = XGetPixel(im, x, y);
			if (ppix != ipix) {
				report("Bad pixel in tiled area at (%d, %d)", x, y);
				ret = False;
				goto bad;
			}
		}
	}

bad:
	XDestroyImage(im);
	XDestroyImage(pmim);

	return(ret);
}


#define tile_width	7
#define tile_height	7
static	char	tile_bits[] = {
	0x4c, 0x52, 0x04, 0x08,
	0x52, 0x4c, 0x61,
};

/*
 * Make a 7x7 tile and fill it with a pattern.
 */
Pixmap
maketile(disp, d)
Display	*disp;
Drawable	d;
{
Pixmap	pm;
unsigned int 	depth;

	depth = getdepth(disp, d);

	pm = XCreatePixmapFromBitmapData(disp, d, tile_bits,
		tile_width, tile_height, W_FG, W_BG, depth);

	return(pm);
}

