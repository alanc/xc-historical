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

#include "stdlib.h"
#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include "pixval.h"

/*
 *
 * Create an invalid colourmap ID by creating a readonly colourmap
 * of the default visual type.
 *
 */

Colormap
badcolormap(disp, win)
	Display *disp;
	Window  win;
{
	Colormap cmap;

	cmap = XCreateColormap(disp, win, DefaultVisual(disp, DefaultScreen(disp)), AllocNone);
	XFreeColormap(disp, cmap);
	return cmap;
}
