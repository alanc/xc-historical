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
>>TITLE XGetKeyboardControl CH07
void

Display	*display = Dsp;
XKeyboardState	*values_return = &Ksvals;
>>EXTERN

static XKeyboardState	Ksvals;

>>ASSERTION Good A
A call to xname returns the current control values for the keyboard to
.A values_return .
>>STRATEGY
Set some keyboard values with XChangeKeyboardControl.
Call xname to get values.
Verify values are as set.
>>CODE
XKeyboardControl	kset;
XKeyboardState	oldkb;

	/*
	 * Actually first we save the original values.
	 */
	values_return = &oldkb;
	XCALL;

	kset.key_click_percent = 21;
	kset.bell_percent = 12;
	kset.bell_pitch = 402;
	kset.bell_duration = 222;

	XChangeKeyboardControl(display, KBKeyClickPercent|KBBellPercent|KBBellPitch|KBBellDuration, &kset);

	if (isdeleted())
		return;

	values_return = &Ksvals;
	XCALL;

	if (values_return->key_click_percent == kset.key_click_percent)
		CHECK;
	else {
		report("Value of key_click_percent was %d, expecting %d",
			values_return->key_click_percent, kset.key_click_percent);
		FAIL;
	}
	if (values_return->bell_percent == kset.bell_percent)
		CHECK;
	else {
		report("Value of bell_percent was %d, expecting %d",
			values_return->bell_percent, kset.bell_percent);
		FAIL;
	}
	if (values_return->bell_pitch == kset.bell_pitch)
		CHECK;
	else {
		report("Value of bell_pitch was %d, expecting %d",
			values_return->bell_pitch, kset.bell_pitch);
		FAIL;
	}
	if (values_return->bell_duration == kset.bell_duration)
		CHECK;
	else {
		report("Value of bell_duration was %d, expecting %d",
			values_return->bell_duration, kset.bell_duration);
		FAIL;
	}

	CHECKPASS(4);

	/*
	 * Attempt to restore the original values.
	 */
	kset.key_click_percent = oldkb.key_click_percent;
	kset.bell_percent = oldkb.bell_percent;
	kset.bell_pitch = oldkb.bell_pitch;
	kset.bell_duration = oldkb.bell_duration;

	XChangeKeyboardControl(display, KBKeyClickPercent|KBBellPercent|KBBellPitch|KBBellDuration, &kset);

	XSync(display, False);
