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
>>TITLE XSubImage CH10
XImage *
XSunImage(ximage, x, y, subimage_width, subimage_height)
XImage		*ximage;
int		x;
int		y;
unsigned int	subimage_width;
unsigned int	subimage_height;
>>EXTERN
static int mpattern(display, d, w, h, dep)
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
	for(j=0; j<h; j++)
		for(i=0; i<w; i++) {
			val = (i + j) % mod;
			XSetForeground(display, gc, val);
			XDrawPoint(display, d, gc, i, j);
		}
}

static int checksubimage(subim, x,y, w,h, dep)
XImage		*subim;
int		x;
int		y;
unsigned int	w;
unsigned int	h;
int 		dep;
{
int	i;
int	j;
int	a;
int	b;
int	mod;

	mod = 1<<dep;
	for(j=0, b=y; j<h; j++, b++)
		for(i=0, a=x; i<w; i++, a++)
			if(XGetPixel(subim, i,j) != (a+b) % mod) {
				return(0);
			}
	return 1;
}
>>ASSERTION Good A
When the rectangle specified by the
.A x ,
.A y ,
.A subimage_width
and
.A subimage_height
arguments is contained in the image
.A ximage ,
then a call to xname creates a copy of the subimage and
returns a pointer to the newly created subimage.
>>STRATEGY
For all supported drawables types:
   Create a drawable.
   Initialise the drawable.
   For XYPixmap and ZPixmap:
      Create an image using XGetImage.
      Obtain a subimage using xname.
      Verify that the pixels in the subimage are correctly set using XGetPixel.
>>CODE
XImage		*sxi;
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
		x = 3;
		y = 5;
		subimage_width = width - 2*x;
		subimage_height = height - 2*y;
		mpattern(Dsp, pm, width, height, vi->depth);

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, pm, 0,0, width, height, AllPlanes, fmats[i]);

			if(ximage == (XImage *) NULL) {
				delete("XGetImage() returned NULL.");
				fail++; /* Avoid path checking in CHECKPASS */
			} else {
				sxi = XCALL;
				if(sxi == (XImage *) NULL) {
					delete("%s() returned NULL.", TestName);
					fail++; /* Avoid path checking */
				} else {
					if(checksubimage(sxi, x,y, subimage_width, subimage_height, vi->depth) == 0) {
						report("Subimage structure was not correct.");
						FAIL;
					} else
						CHECK;

					XDestroyImage(sxi);
				}

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
		x = 3;
		y = 5;
		subimage_width = width - 2*x;
		subimage_height = height - 2*y;
		mpattern(Dsp, win, width, height, vi->depth);

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, win, 0,0, width, height, AllPlanes, fmats[i]);
			if(ximage == (XImage *) NULL) {
				delete("XGetImage() returned NULL.");
				fail++; /* Avoid path checking in CHECKPASS */
			} else {

				sxi = XCALL;
				if(sxi == (XImage *) NULL) {
					delete("%s() returned NULL.", TestName);
					fail++; /* Avoid path checking */
				} else {
					if(checksubimage(sxi, x,y, subimage_width, subimage_height, vi->depth) == 0) {
						report("Subimage structure was not correct.");
						FAIL;
					} else
						CHECK;

					XDestroyImage(sxi);
				}

				XDestroyImage(ximage);
			}
		}
	
	}

	CHECKPASS(2 * (npix + nvinf()));
