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
 * $XConsortium: lstinstlld.m,v 1.4 92/06/11 17:15:10 rws Exp $
 */
>>TITLE XListInstalledColormaps CH07
Colormap *
xname
Display	*display = Dsp;
Window	w;
int 	*num_return = &Num;
>>EXTERN

/* Variable to hold the return value in */
static	int 	Num;

>>ASSERTION Good A
A call to xname returns a list, that can be
freed with
.F XFree ,
of the currently installed colourmaps for the screen
of the specified window and the number of such colourmaps
in
.A num_return .
>>STRATEGY
Call xname.
Verify that the number of installed colour maps is between the min and
max limits.
Free return value with XFree.
>>CODE
Colormap	*cmp;
Screen	*screen;

	w = DRW(display);
	cmp = XCALL;

	if (cmp == NULL) {
		report("NULL was returned");
		FAIL;
		return;
	} else
		CHECK;

	screen = ScreenOfDisplay(display, DefaultScreen(display));
	if (*num_return < 1 ||
		*num_return > MaxCmapsOfScreen(screen)) {
		report("Number of installed colourmaps was outside range [1, MaxCmapsOfScreen]");
		FAIL;
	} else
		CHECK;

	XFree((char*)cmp);

	CHECKPASS(2);
>>ASSERTION Bad A
.ER BadWindow
