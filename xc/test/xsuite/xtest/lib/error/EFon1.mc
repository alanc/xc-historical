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
>>### Font bad-font
When a font argument does not name a valid font, then a
.S BadFont
error occurs.
>>STRATEGY
Create a bad font by loading and freeing a font.
Call test function using bad font as the font argument.
Verify that a BadFont error occurs.
>>CODE BadFont

	seterrdef();

	A_FONT = (Font) badfont(A_DISPLAY);

	XCALL;

	if(geterr() == BadFont)
		PASS;
	else
		FAIL;
