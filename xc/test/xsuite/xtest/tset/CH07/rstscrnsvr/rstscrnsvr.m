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
>>TITLE XResetScreenSaver CH07
void

Display	*display = Dsp;
>>ASSERTION Good B 3
When the screen saver is enabled and activated, then a call to xname
deactivates the screen saver.
>>STRATEGY
Touch test for xname only.
>>CODE

	XCALL;

	untested("There is no reliable test method, but a touch test was performed");
>>ASSERTION Good B 3
When the screen saver is enabled,
then a call to xname resets the activation timer to its
initial state as though device input had been received.
