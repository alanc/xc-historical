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
>>TITLE XScreenCount CH02
int
XScreenCount(display)
Display	*display = Dsp;
>>#
>># COMMENT:	Could maybe assume numbering scheme for screen to be 0,1,2...
>>#		then try to open each one using XOpenDisplay with a modified
>>#		XT_DISPLAY value.
>>#
>># Cal 24/7/91
>># REPLY:	This is now done in the appropriate test for XOpenDisplay.
>># Dave 20/9/91
>>#
>>ASSERTION Good A
A call to xname returns the number of available screens on the server connection
specified by the
.A display
argument.
>>STRATEGY
Obtain the number of available screens using xname.
Verify that the number of screens is that given in parameter XT_SCREEN_COUNT.
>>CODE
int	scrn;
int	s = config.screen_count;

	if (s < 0) {
		delete("Parameter XT_SCREEN_COUNT not set.");
		return;
	}

	scrn = XCALL;
	if (scrn != s) {
		report("%s() returns %d available screen%s.", 
					TestName, scrn, scrn > 1 ? "s" : "");
		report("Expected %d available screen%s.", 
					s, s > 1 ? "s" : "");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
