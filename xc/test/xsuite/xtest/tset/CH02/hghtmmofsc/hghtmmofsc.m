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
>>TITLE XHeightMMOfScreen CH02
int
XHeightMMOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the height in millimeters of the screen
.A screen .
>>STRATEGY
Obtain the height of the screen in millimeters using xname.
Verify that the value is that given in parameter XT_HEIGHT_MM.
>>CODE
int	hmm;

	hmm = XCALL;
	if(hmm != config.height_mm) {
		report("%s() returns incorrect value for screen height", 
						TestName);
		report("Expected value 0x%lx; Observed value 0x%lx", 
						config.height_mm, hmm);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
