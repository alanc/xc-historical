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
>>TITLE XGetPointerControl CH07
void

Display	*display = Dsp;
int 	*accel_numerator_return = &num;
int 	*accel_denominator_return = &denom;
int 	*threshold_return = &thresh;
>>EXTERN

static int 	num;
static int 	denom;
static int 	thresh;

>>ASSERTION Good A
A call to xname returns the current acceleration multiplier
and acceleration threshold to
.A accel_numerator_return ,
.A accel_denominator_return
and
.A threshold_return .
>>STRATEGY
Set some values.
Call xname.
Verify values are as set.
>>CODE
int 	onum, odenom, othresh;
int 	val = 34;

	/* First get original values */
	accel_numerator_return = &onum;
	accel_denominator_return = &odenom;
	threshold_return = &othresh;
	XCALL;

	XChangePointerControl(display, True, True, val, val, val);
	if (isdeleted())
		return;

	accel_numerator_return = &num;
	accel_denominator_return = &denom;
	threshold_return = &thresh;
	XCALL;

	if (num == val)
		CHECK;
	else {
		report("accel_numerator_return was %d, expecting %d", num, val);
		FAIL;
	}
	if (denom == val)
		CHECK;
	else {
		report("accel_denominator_return was %d, expecting %d", denom, val);
		FAIL;
	}
	if (thresh == val)
		CHECK;
	else {
		report("threshold_return was %d, expecting %d", thresh, val);
		FAIL;
	}

	CHECKPASS(3);

	XChangePointerControl(display, True, True, onum, odenom, othresh);
	XSync(display, False);
