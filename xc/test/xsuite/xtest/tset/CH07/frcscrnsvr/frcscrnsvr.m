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
>>TITLE XForceScreenSaver CH07
void

Display	*display = Dsp;
int 	mode = ScreenSaverReset;
>>ASSERTION Good B 3
When the specified
.A mode
is
.S ScreenSaverActive
and the screen saver is currently deactivated,
then a call to xname
activates the screen saver even after the screen saver has been disabled
with a timeout of zero.
>>STRATEGY
Touch test only.
>>CODE

	mode = ScreenSaverActive;

	XCALL;

	untested("There is no reliable test method - touch test for ScreenSaverActive");
>>ASSERTION Good B 3
When the specified
.A mode
is
.S ScreenSaverReset
and the screen saver is currently enabled and activated,
then a call to xname
deactivates the screen saver
and the activation timer is reset to its initial state
as though device input had been received.
>>STRATEGY
Touch test for ScreenSaverReset only.
>>CODE

	mode = ScreenSaverReset;

	XCALL;

	untested("There is no reliable test method - touch test for ScreenSaverReset");
>>ASSERTION Bad A
.ER BadValue mode ScreenSaverActive ScreenSaverReset
