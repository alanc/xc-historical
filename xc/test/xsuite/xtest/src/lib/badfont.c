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

#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include	"pixval.h"

/*
 * Return a bad font ID by loading a font and then unloading it.
 * Note the font ID may be realocated and so become good again by the
 * time it is used.  If the font cannot be found then return 0 - this
 * is a bad font ID anyway.
 */
Font
badfont(disp)
Display	*disp;
{
XFontStruct	*fsp;
Font	font;


	fsp = XLoadQueryFont(disp, "xtfont0");
	if (fsp)
	{
		font = fsp->fid;
		XFreeFont(disp, fsp);
	} else
		font = 0;

	return(font);
}
