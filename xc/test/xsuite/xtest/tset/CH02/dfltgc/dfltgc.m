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
>>TITLE XDefaultGC CH02
GC
XDefaultGC(display, screen_number)
Display	*display = Dsp;
int	screen_number = DefaultScreen(Dsp);
>>ASSERTION Good B 1
A call to xname returns the default graphics context ID for the root window
of the screen
.A screen_number .
>>STRATEGY
Obtain the ID of the default GC using xname.
>>CODE
GC	dgc;

	dgc = XCALL;
	report("Default GC has ID %lx", (long) dgc);
	CHECK;
	CHECKUNTESTED(1);
