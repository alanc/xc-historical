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
>>TITLE XPutPixel CH10
int
XPutPixel(ximage, x, y, pixel)
XImage		*ximage;
int		x;
int		y;
unsigned long	pixel;
>>EXTERN
int bltimage(im, w, h, dep)
XImage		*im;
unsigned int	w;
unsigned int	h;
int		dep;
{
int		i;
unsigned long	mask;
unsigned long	rpixel;

	ximage = im;
	mask = (1<<dep) - 1;
	for(y=0; y<h; y+=10)
		for(x=0; x<w; x+=10) {
			for(i = 0; i <= (15 & mask); i++) {
				pixel =  mask & (i | i<<4 | i<<12 | i<<20);
				startcall(Dsp);
				if (isdeleted())
					return(0);
				ValueReturn = XPutPixel(ximage, x, y, pixel);
				if (ValueReturn == 0) {
					report("%s() returned 0", TestName);
					return(0);
				}
				endcall(Dsp);
				if (geterr() != Success) {
					report("Got %s, Expecting Success", errorname(geterr()));
					return(0);
				}
				if((rpixel = XGetPixel(ximage, x, y)) != pixel) {
					report("XGetPixel() returned %lx instead of %lx.", rpixel, pixel);
					return(0);
				}
			}
		}

	return(1);
}		
>>ASSERTION Good A
When the image
.A ximage
contains the coordinate
.A x,y ,
and the
.A pixel
argument is in normalised format, then a call to xname overwrites
.A x,y
with the value
.A pixel .
>>STRATEGY
For all supported drawables:
   Create a drawable.
   For XYPixmap and ZPixmap:
      Obtain an XImage structure using XGetImage.
      For a range of pixel values over the drawable's depth:
         Set pixels using xname.
         Verify that the pixels are correctly set using XGetPixel.
>>CODE
XVisualInfo	*vi;
int		npix;
unsigned int	width;
unsigned int	height;
Pixmap		pm;
Window		win;
int		i;
static int	fmats[2] = { XYPixmap, ZPixmap };
	

	for(resetvinf(VI_PIX); nextvinf(&vi);) {

		pm = makepixm(Dsp, vi);
		getsize(Dsp, pm, &width, &height);
		width = width > 17 ? 17 : width;
		height = height > 19  ? 19 : height;

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, pm, 0,0, width, height, AllPlanes, fmats[i]);
			if( ximage == (XImage *) NULL ) {
				delete("XGetImage() returned NULL.");
			} else {
				if(bltimage(ximage, width, height, vi->depth) == 0) 
					FAIL;
				else
					CHECK;

				XDestroyImage(ximage);
			}

		}
	}
	npix = nvinf();

	for(resetvinf(VI_WIN); nextvinf(&vi);) {

		win = makewin(Dsp, vi);
		getsize(Dsp, win, &width, &height);
		width = width > 17 ? 17 : width;
		height = height > 19  ? 19 : height;

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, win, 0,0, width, height, AllPlanes, fmats[i]);
			if(ximage == (XImage *) NULL) {
				delete("XGetImage() returned NULL.");
			} else {
				if(bltimage(ximage, width, height, vi->depth) == 0) 
					FAIL;
				else
					CHECK;

				XDestroyImage(ximage);
			}
		}

	}

	CHECKPASS(2 * (npix + nvinf()));
