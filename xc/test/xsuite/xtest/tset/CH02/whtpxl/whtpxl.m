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
>>TITLE XWhitePixel CH02
unsigned long
XWhitePixel(display, screen_number)
Display *display = Dsp;
int	screen_number = DefaultScreen(Dsp);
>>ASSERTION Good A
A call to xname returns the white pixel value for 
>># comments subsequent to the review period resulted in the extra words ...
the default colourmap of
the screen
.A screen_number .
>>STRATEGY
Obtain the value of the white pixel using xname.
Verify that the value is that given in parameter XT_WHITE_PIXEL.
>>CODE
unsigned long	pixl;

	pixl = XCALL;
	if(pixl != config.white_pixel) {
		report("%s() returns incorrect value for white pixel", 
						TestName);
		report("Expected value 0x%lx; Observed value 0x%lx", 
						config.white_pixel, pixl);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
