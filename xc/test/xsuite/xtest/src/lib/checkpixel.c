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
 * Test pixel in a drawable.
 */

int
checkpixel(display,drawable,x,y,value)
Display *display;
Drawable drawable;
int x;
int y;
unsigned long value;
{
	XImage *image;
	int 	pix;

	image = XGetImage(display, drawable, x, y, 1, 1, AllPlanes, ZPixmap);
	pix = XGetPixel(image, 0, 0);
	XDestroyImage(image);
	return(pix == value);
}

/*
 * Return the value of a pixel.  Only efficient when only one (or a few)
 * pixels are of interest.
 */
unsigned long
getpixel(display, drawable, x, y)
Display *display;
Drawable drawable;
int 	x;
int 	y;
{
XImage *image;
unsigned long	pix;

	image = XGetImage(display, drawable, x, y, 1, 1, AllPlanes, ZPixmap);
	pix = XGetPixel(image, 0, 0);
	XDestroyImage(image);
	return(pix);
}

/*
 * Verify the values of a set of linear points.
 */

Status
checkpixels(display, drawable, x, y, dx, dy, len, value)
Display *display;
Drawable drawable;	
int x;
int y;
int	dx;
int dy;
int len;
unsigned long value;
{
	int i;
	
	for(i=0; i<len; i++, x += dx, y += dy)
		if(checkpixel(display, drawable, x, y, value) == 0)
		  return(False);

	return(True);
}
