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
>>TITLE XDefaultGCOfScreen CH02
GC
XDefaultGCOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good B 1
A call to xname returns the ID of the default GC of the screen
.A screen .
>>STRATEGY
Obtain the ID of the default GC of the default screen.
>>CODE
GC	dgc;

	dgc = XCALL;
	report("Default GC has ID %lx", (long) dgc);
	CHECK;
	CHECKUNTESTED(1);

>>ASSERTION Good A
The default GC has the same depth as the root window of the screen
.A screen .
>>STRATEGY
Draw the point (0,0) using the default GC on the default root window.
Verify that a BadMatch error did not occur.
>>CODE
XWindowAttributes	atts;
GC			dgc;

	dgc = XCALL;

	startcall(Dsp);
	XDrawPoint(Dsp, RootWindowOfScreen(screen), dgc, 0, 0);
	endcall(Dsp);


	if(geterr() == BadMatch) {
		report("%s() returned a GC which was not of the correct depth.", TestName);
		FAIL;
	} else
		CHECK;

	if(geterr() != Success) {
		delete("XDrawPoint() failed with a %s error.", errorname(geterr()));
		return;
	} else
		CHECK;


	CHECKPASS(2);
