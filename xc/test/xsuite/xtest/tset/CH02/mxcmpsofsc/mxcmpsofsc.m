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
 * $XConsortium: mxcmpsofsc.m,v 1.6 92/06/11 16:00:33 rws Exp $
 */
>>SET   macro
>>TITLE XMaxCmapsOfScreen CH02
int
XMaxCmapsOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good B 3
A call to xname returns the maximum number of installed colourmaps supported by the screen
.A screen .
>>STRATEGY
Obtain the minimum allowed number of installed colourmaps
  using XMinCmapsOfScreen.
Verify that the minimum is greater than zero.
Obtain the maximum allowed number of installed colourmaps using xname.
Verify that the maximum is greater than or equal to the minimum.
Report UNTESTED (the assertion can only be partially tested).
>>CODE
int		maxm;
int		minm;

	minm = XMinCmapsOfScreen( screen );

	if (minm <= 0) {
		delete("XMinCmapsOfScreen() returned %d.", minm);
		return;
	} else
		CHECK;

	maxm = XCALL;

	if (maxm < minm) {
		report("%s() returns %d, but the minimum was %d",
			TestName, maxm, minm);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(2);
