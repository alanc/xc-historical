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
 * Return a pixmap of depth other than 1 if such is supported,
 * otherwise return (pixmap) 0.
 *
 */
#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include	"xtestlib.h"
#include "Xutil.h"
#include	"pixval.h"

Pixmap
nondepth1pixmap(disp, d)
	Display	*disp;
	Drawable d;
{
	XVisualInfo *vp;

	for(resetvinf(VI_WIN); nextvinf(&vp);)
		if(vp -> depth != 1)
		  return(XCreatePixmap(disp,d,1,1,vp->depth));

	return ((Pixmap) 0);
}
