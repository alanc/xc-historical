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
>>### Match wininputonly
When the window argument is an
.S InputOnly
window then a
.S BadMatch
error occurs.
>>STRATEGY
Create an InputOnly window.
Call test function using InputOnly window as the window argument.
Verify that a BadMatch error occurs.
>>CODE BadMatch

	seterrdef();

	A_WINDOW = iponlywin(A_DISPLAY);
#ifdef A_WINDOW2
	A_WINDOW2 = iponlywin(A_DISPLAY);
#endif

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
