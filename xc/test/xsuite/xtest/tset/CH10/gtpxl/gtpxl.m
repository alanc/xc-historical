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
>>TITLE XGetPixel CH10
unsigned long
XGetPixel(ximage, x, y)
XImage	*ximage;
int	x;
int	y;
>>ASSERTION Good A
When the image
.A ximage
contains coordinates
.A x, y ,
then a call to xname returns the normalised value of the pixel located at
.A x, y
in the image.
>>STRATEGY
For all supported drawables:
   Create a drawable.
   For a range of pixel values over the drawable's depth:
      Set a pixel in the drawable using XDrawPoint.
      For XYPixmap and ZPixmap:
         Obtain an XImage using XGetImage.
         Obtain the pixel value using xname.
         Verify that the values are the same.
>>CODE
XVisualInfo	*vi;
GC		gc;
int		npix;
Pixmap		pm;
Window		win;
int		i;
int		j;
unsigned long	pixel;
unsigned long	rpixel;
unsigned long	mask;
static int	fmats[2] = { XYPixmap, ZPixmap };

	x = 2;
	y = 1;

	for(resetvinf(VI_PIX); nextvinf(&vi);) {

		pm = makepixm(Dsp, vi);
		gc = makegc(Dsp, pm);
		mask = (1<<vi->depth) - 1;
		for(i = 0; i <= (15 & mask); i++) {
			pixel =  mask & (i | i<<4 | i<<12 | i<<20);
			XSetForeground(Dsp, gc, pixel);
			XDrawPoint(Dsp, pm, gc, x+1, y+2);

			for(j=0; j<2; j++) {
				ximage = XGetImage(Dsp, pm, 1,2, 2*x,2*y, AllPlanes, fmats[j]);
				if( ximage == (XImage *) NULL ) {
					delete("XGetImage() returned NULL.");
					return;
				} else {

					rpixel = XCALL;

					if(rpixel != pixel) {
						report("%s() returned 0x%lx instead of 0x%lx.", TestName, rpixel, pixel);
						FAIL;
					} else
						if(i == 0 && j == 0)
							CHECK;

					XDestroyImage(ximage);
				}

			}
		}

	}

	npix = nvinf();

	for(resetvinf(VI_WIN); nextvinf(&vi);) {

		win = makewin(Dsp, vi);
		gc = makegc(Dsp, win);
		mask = (1<<vi->depth) - 1;
		for(i = 0; i <= (15 & mask); i++) {
			pixel =  mask & (i | i<<4 | i<<12 | i<<20);
			XSetForeground(Dsp, gc, pixel);
			XDrawPoint(Dsp, win, gc, x+1, y+2);

			for(j=0; j<2; j++) {
				ximage = XGetImage(Dsp, win, 1,2, 2*x,2*y, AllPlanes, fmats[j]);
				if( ximage == (XImage *) NULL ) {
					delete("XGetImage() returned NULL.");
					return;
				} else {

					rpixel = XCALL;

					if(rpixel != pixel) {
						report("%s() returned 0x%lx instead of 0x%lx.", TestName, rpixel, pixel);
						FAIL;
					} else
						if(i == 0 && j == 0)
							CHECK;

					XDestroyImage(ximage);
				}

			}
		}

	}

	CHECKPASS(npix + nvinf());

