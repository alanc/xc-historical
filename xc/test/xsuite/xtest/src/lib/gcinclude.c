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
/*
 * Functions that have no general utility but are called in the 
 * included gc routines can go in here.  This can save a lot of code
 * being included in every file.
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"tet_api.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Find the first pixel that is set in a drawable.  Used
 * in the gc funtion tests.
 */
void
setfuncpixel(disp, drw, xp, yp)
Display	*disp;
Drawable	drw;
int 	*xp;
int 	*yp;
{
int 	x;
int 	y;
unsigned int 	width, height;
XImage	*imp;

	getsize(disp, drw, &width, &height);
	imp = savimage(disp, drw);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (XGetPixel(imp, x, y) == W_FG) {
				*xp = x;
				*yp = y;
				return;
			}
		}
	}
	/*
	 * No pixel has been found - this 'cant happen' so might as well
	 * flag the fact here.  The higher routines will also notice this
	 * anyway
	 */
	delete("No pixel set in drawable");
}
