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
>>TITLE XAddPixel CH10

XAddPixel(ximage, value)
XImage	*ximage;
long	value;
>>EXTERN
mpattern(display, d, w, h, dep)
Display		*display;
Drawable	d;
unsigned int	w;
unsigned int	h;
int		dep;
{
int		i;
int		j;
int		mod;
GC		gc;
unsigned long	val;

	gc = makegc(display, d);
	mod = 1<<dep;
	val = 0;
	for(j=0; j<h; j++)
		for(i=0; i<w; i++) {
			XSetForeground(display, gc, val);
			XDrawPoint(display, d, gc, i, j);
			val = (val + 1) % mod;
		}
}

int
mcheck(xi, w, h, dep)
XImage		*xi;
unsigned int	w;
unsigned int	h;
int		dep;
{
int		i;
int		j;
int		mod;
unsigned long	val;

	mod = 1<<dep;
	val = value;
	for(j=0; j<h; j++)
		for(i=0; i<w; i++) {
			if(XGetPixel(ximage, i, j) != val){
				return 0;
			}
			val = (val + 1) % mod;
		}
	return 1;
}
>>ASSERTION Good A
A call to xname adds the
.A value
argument to every pixel in the
.A ximage
argument.
>>STRATEGY
For all supported drawables:
   Create a drawable.
   Initialise the drawable's pixels.
   For ZPixmap and XYPixmap:
	   Obtain an ximage from the drawable using XGetImage.
	   Add the drawables depth - 1 to every image pixel using xname.
	   Verify that the ximage pixels all set correctly using XGetPixel.
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
		mpattern(Dsp, pm, width, height, vi->depth);

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, pm, 0,0, width, height, AllPlanes, fmats[i]);
			if( ximage == (XImage *) NULL ) {
				delete("XGetImage() returned NULL.");
				return;
			} else {

				value = (1 << vi->depth) - 1;
				XCALL;
				if(mcheck(ximage, width, height, vi->depth) == 0) {
					report("XImage structure was not correct.");
					FAIL;
				} else
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
		mpattern(Dsp, win, width, height, vi->depth);

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, win, 0,0, width, height, AllPlanes, fmats[i]);
			if(ximage == (XImage *) NULL) {
				delete("XGetImage() returned NULL.");
				return;
			} else {
				value = (1 << vi->depth) - 1;
				XCALL;
				if(mcheck(ximage, width, height, vi->depth) == 0) {
					report("XImage structure was not correct.");
					FAIL;
				} else
					CHECK;

				XDestroyImage(ximage);
			}
		}

	}

	CHECKPASS(2 * (npix + nvinf()));
