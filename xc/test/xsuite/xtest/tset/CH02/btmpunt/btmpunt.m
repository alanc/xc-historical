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
>>TITLE XBitmapUnit CH02
int
XBitmapUnit(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname returns an integer 8, 16, or 32  which is less than the 
bitmap pad value and which is the size in bits of a scanline unit belonging to bitmaps.
>>STRATEGY
Obtain the bitmap unit from a XImage structure using XGetImage.
Obtain the bitmap unit using xname.
Verify that the values are the same.
>>CODE
int	bunt;
int	pad;
XImage	*xi;

	bunt = XCALL;	

	if( (bunt != 8) && (bunt != 16) && (bunt != 32) ) {
		report("%s() returned illegal value %d.", TestName, bunt);
		FAIL;
	} else
		CHECK;

	pad = XBitmapPad(display);

	if( bunt > pad ) {
		report("%s() returned %d which is not less than the bitmap pad of %d.", TestName, bunt, pad);
		FAIL;
	} else
		CHECK;

	xi = XGetImage(display, DRW(display), 0,0, 1,1, XAllPlanes(), ZPixmap);

	if(xi->bitmap_unit != bunt) {
		report("%s() returned %d instead of %d.", TestName, bunt, xi->bitmap_unit);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

	XFree((char *) xi);
