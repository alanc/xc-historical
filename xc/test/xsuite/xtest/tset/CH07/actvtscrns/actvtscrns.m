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
>>TITLE XActivateScreenSaver CH07
void

Display	*display = Dsp;
>>ASSERTION Good B 3
When the screen saver is currently deactivated, then a call to xname
activates the screen saver.
>>STRATEGY
Touch test only.
>>CODE

	XCALL;

	untested("There is no reliable test method, but a touch test was performed");
>>ASSERTION Good B 3
When the screen saver is currently deactivated
and the screen saver has been disabled with a timeout
of zero, then a call to xname
activates the screen saver.
