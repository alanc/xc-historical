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
>>TITLE XAutoRepeatOff CH07
void

Display	*display = Dsp;
>>ASSERTION Good A
A call to xname turns off auto-repeat for the keyboard on the specified display.
>>STRATEGY
Get old auto repeat value.
Call xname.
Get new auto repeat value.
Verify that auto repeat is off.
Restore old value.
>>CODE
XKeyboardState	old, new;
XKeyboardControl	ctr;

	/*
	 * This is just a touch test really.
	 */
	XGetKeyboardControl(display, &old);

	XCALL;

	XGetKeyboardControl(display, &new);
	if (new.global_auto_repeat == AutoRepeatModeOff)
		CHECK;
	else {
		report("Auto-repeat was not turned off");
		FAIL;
	}

	ctr.auto_repeat_mode = old.global_auto_repeat;

	XChangeKeyboardControl(display, KBAutoRepeatMode, &ctr);

	CHECKPASS(1);
