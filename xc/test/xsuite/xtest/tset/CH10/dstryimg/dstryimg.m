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
>>TITLE XDestroyImage CH10
int
XDestroyImage(ximage)
XImage	*ximage;
>>ASSERTION Good B 1
A call to xname frees both the structure and image data specified by the
.A ximage
argument.
>>STRATEGY
For 20 iterations:
   Create an image using XGetImage on the root window.
   Access the region data using XAddPixel.
   Free the structure using xname.
   Create an image using XCreateImage.
   Free the structure using xname.
>>CODE
int 	i;
int	bp;
static  int	fmat[2] = {XYPixmap, ZPixmap};

	bp = BitmapPad(Dsp);

	for(i=0; i<20; i++) {

		ximage = XGetImage(Dsp, RootWindow(Dsp, 0), 0,0, 12,13, AllPlanes, fmat[ i%2 ]);
		if(ximage == (XImage *) NULL){
			delete("XGetImage() returned NULL.");
			return;
		} else
			if(i == 19)
				CHECK;
		
		XAddPixel(ximage, 1<<29 - 7);
		XCALL;

		ximage = XCreateImage(Dsp, DefaultVisual(Dsp, 0), 32, fmat [ 1 - i%2 ], 5, (char *) NULL, 13,23, 32, 0);

		if(ximage == (XImage *) NULL){
			delete("XCreateImage() returned NULL.");
			return;
		} else
			if(i == 19)
				CHECK;

		XCALL;

	} 

	CHECKUNTESTED(2);
