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
>>TITLE XDisplayMotionBufferSize CH08
unsigned long
XDisplayMotionBufferSize(display)
Display *display = Dsp;
>>EXTERN
#include <stdio.h>
>>ASSERTION Good C
If
the implementation supports a more complete
history of pointer motion than is reported by event notification:
a call to xname
returns the approximate maximum number of elements in the motion buffer.
>>STRATEGY
If motion buffer not supported, call UNSUPPORTED.
Call XDisplayMotionBufferSize.
Verify value returned by XDisplayMotionBufferSize.
>>CODE
unsigned long bufsize;

/* If motion buffer not supported, call UNSUPPORTED. */
	if (config.displaymotionbuffersize == 0) {
		report("Motion buffer not supported.");
		UNSUPPORTED;
		return;
	}
	else
		CHECK;
/* Call XDisplayMotionBufferSize. */
	bufsize = XCALL;
/* Verify value returned by XDisplayMotionBufferSize. */
	if (bufsize != config.displaymotionbuffersize) {
		report("Returned %d, expected %d", bufsize, config.displaymotionbuffersize);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good C
If
the implementation does not support a more complete
history of pointer motion than is reported by event notification:
a call to xname
returns zero.
>>STRATEGY
If motion buffer supported, call UNSUPPORTED.
Call XDisplayMotionBufferSize.
Verify value returned by XDisplayMotionBufferSize.
>>CODE
unsigned long bufsize;

/* If motion buffer supported, call UNSUPPORTED. */
	if (config.displaymotionbuffersize != 0) {
		report("Motion buffer supported.");
		UNSUPPORTED;
		return;
	}
	else
		CHECK;
/* Call XDisplayMotionBufferSize. */
	bufsize = XCALL;
/* Verify value returned by XDisplayMotionBufferSize. */
	if (bufsize != 0) {
		report("Returned %d, expected %d", bufsize, 0);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
