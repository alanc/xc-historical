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
>>TITLE XBlackPixelOfScreen CH02
unsigned long
XBlackPixelOfScreen(screen)
Screen	*screen = XDefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the black pixel value of 
>># comments subsequent to the review period resulted in the extra words ...
the default colourmap of
the screen
.A screen .
>>STRATEGY
Obtain the black pixel value of the default screen using xname.
Verify that the value is that given in parameter XT_BLACK_PIXEL.
>>CODE
unsigned long	pixl;

	pixl = XCALL;
	if(pixl != config.black_pixel) {
		report("%s() returns incorrect value for black pixel", 
						TestName);
		report("Expected value 0x%lx; Observed value 0x%lx", 
						config.black_pixel, pixl);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
