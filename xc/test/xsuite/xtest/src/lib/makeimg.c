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

#include	"stdlib.h"
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	"pixval.h"

/*
 * Creates a general purpose image that can be used within the
 * test suite.  The image is cleared to W_BG.
 */
XImage *
makeimg(disp, vp, format)
Display	*disp;
XVisualInfo	*vp;
int	format;
{
XImage	*im;
unsigned int width = I_STDWIDTH;
unsigned int height = I_STDHEIGHT;
unsigned int depth = vp->depth;
unsigned int n;

	switch (format)
	{
	case XYBitmap:
		depth = 1;
		break;
	case XYPixmap:
		break;
	case ZPixmap:
		break;
	default:
		delete("Unknown format in makeimg: %d", format);
		return((XImage *) 0);
	}

	im = XCreateImage(disp, vp->visual, depth, format, 0, (char *) 0, width, height, BitmapPad(disp), 0);

	/* this should be more than enough memory */
	n = (format == ZPixmap ? 1 : depth) * im->bytes_per_line * im->height;
	im->data = (char *) malloc(n);
	if (im->data == (char *) 0) {
		delete("Memory allocation failed in makeimg: %d bytes", n);
		return((XImage *) 0);
	}

	/* register Image */
	regid(disp, (union regtypes *)&im, REG_IMAGE);

	/* set all pixel values to W_BG */
	dsetimg(im, W_BG);

	return(im);
}

/*
 * Set every pixel in an image to pixel.
 */
void
dsetimg(ximage, pixel)
XImage	*ximage;
unsigned long pixel;
{
	int x;
	int y;

	for (x = 0; x < ximage->width; x++)
		for (y = 0; y < ximage->height; y++)
			(void) XPutPixel(ximage, x, y, pixel);
}

/*
 * Scribble on the image.  A series of vertical lines are drawn starting
 * at (0,0) then at (5,0) (10,0) etc.  pixel specifies the pixel value
 * assumed by the lines.
 */
void
patternimg(ximage, pixel)
XImage	*ximage;
unsigned long pixel;
{
	int x;
	int y;

	for (x = 0; x < ximage->width; x += 5)
		for (y = 0; y < ximage->height; y++)
			(void) XPutPixel(ximage, x, y, pixel);
}
