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
>>### Cursor
When a cursor argument does not name a valid xerrlist, then a
.S BadCursor
error occurs.
>>STRATEGY
Create cursor with shape XT_FONTCURSOR_GOOD in font cursor.
Free cursor.
Call test function using bad cursor as the cursor argument.
Verify that a BadCursor error occurs.
>>CODE BadCursor
unsigned int shape;

	seterrdef();

	/* UNSUPPORTED is not allowed */
	shape = config.fontcursor_good;
	if (config.fontcursor_good == -1) {
		delete("A value of UNSUPPORTED is not allowed for XT_FONTCURSOR_GOOD");
		return;
	}

	A_CURSOR = XCreateFontCursor(A_DISPLAY, shape);
	XFreeCursor(A_DISPLAY, A_CURSOR);

	XCALL;

	if (geterr() == BadCursor)
		PASS;
	else
		FAIL;
