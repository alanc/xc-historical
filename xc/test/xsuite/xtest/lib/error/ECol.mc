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
>>ASSERTION Bad A
>>### Color
When a colourmap argument does not name a valid colourmap, then a
.S BadColor
error occurs.
>>STRATEGY
Create a bad colourmap by creating and freeing a colourmap.
Call test function using bad colourmap as the colourmap argument.
Verify that a BadColor error occurs.
>>CODE BadColor

	A_COLORMAP = badcolormap(A_DISPLAY, DRW(A_DISPLAY));
	XCALL;
	if(geterr() == BadColor)
		PASS;
	else 
		FAIL;
