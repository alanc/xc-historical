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
>>### Name font
When the specified font does not exist, then a
.S BadName
error occurs.
>>STRATEGY
Call test function with non-existent font as the font name argument.
Verify that a BadName error occurs.
>>CODE BadName
Font	font;

	/* Note:  the argument has to be called 'name' */
	name = config.bad_font_name;

	font = XCALL;

	if (geterr() == BadName)
		PASS;
	else
		FAIL;	/* For safty, this should already have been issued */
