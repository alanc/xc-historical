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
>>SET   macro
>>TITLE XBitmapPad CH02
int
XBitmapPad(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname returns the quantum of a scanline which is 8, 16, or 32.
>>STRATEGY
Obtain the bitmap pad from an XImage structure using XGetImage.
Obtain the bitmap pad using xname.
Verify the bitmap pad values are the same.
>>CODE
int	bpad;
XImage	*xi;

	bpad = XCALL;	
	xi = XGetImage(display, DRW(display), 0,0, 1,1, XAllPlanes(), ZPixmap);

	if( (xi->bitmap_pad != 8) && (xi->bitmap_pad != 16) && (xi->bitmap_pad != 32) ) {
		report("%s() returned invalid value %d.", TestName, xi->bitmap_pad);
		FAIL;
	} else 
		if(xi->bitmap_pad != bpad) {
			report("%s() returned %d instead of %d.", TestName, bpad, xi->bitmap_pad);
			FAIL;
		} else
			PASS;

	XFree((char *) xi);
