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
>>TITLE XSynchronize CH08
int ((*)())()
XSynchronize(display, onoff)
Display *display = Dsp;
Bool onoff;
>>EXTERN
static int
afterfunction(display)
Display *display;
{
	return((int) display);
}
>>ASSERTION Good B 1
>>#NOTE Untestable.
A call to xname
with
.A onoff
set to
.S True
turns on synchronous behavior.
>>ASSERTION Good B 1
>>#NOTE Untestable.
A call to xname
with
.A onoff
set to
.S False
turns off synchronous behavior.
>>ASSERTION Good A
A call to xname
with
.A onoff
set to
.S True
sets the after function to a non-NULL value.
>>STRATEGY
Call XSynchronize with onoff set to True.
Call XSetAfterFunction to get value of old after function.
Verify that XSetAfterFunction returned non-NULL.
>>CODE
int	(*proc)();

/* Call XSynchronize with onoff set to True. */
	onoff = True;
	(void) XCALL;
/* Call XSetAfterFunction to get value of old after function. */
	proc = XSetAfterFunction(display, afterfunction);
/* Verify that XSetAfterFunction returned non-NULL. */
	if (proc == (int (*)()) NULL) {
		report("Returned NULL, expected non-NULL.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(1);
>>ASSERTION Good A
A call to xname
with
.A onoff
set to
.S False
sets the after function to NULL.
>>STRATEGY
Call XSynchronize with onoff set to False.
Call XSetAfterFunction to get value of old after function.
Verify that XSetAfterFunction returned NULL.
>>CODE
int	(*proc)();

/* Call XSynchronize with onoff set to False. */
	onoff = False;
	(void) XCALL;
/* Call XSetAfterFunction to get value of old after function. */
	proc = XSetAfterFunction(display, afterfunction);
/* Verify that XSetAfterFunction returned NULL. */
	if (proc != (int (*)()) NULL) {
		report("Returned non-NULL, expected NULL.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(1);
>>ASSERTION Good A
A call to xname
returns the previous after function.
>>STRATEGY
Call XSetAfterFunction to set after function to afterfunction.
Call XSynchronize with onoff set to False.
Verify that XSynchronize returned afterfunction.
Call XSetAfterFunction to set after function to afterfunction.
Call XSynchronize with onoff set to True.
Verify that XSynchronize returned afterfunction.
>>CODE
int	(*proc)();

/* Call XSetAfterFunction to set after function to afterfunction. */
	(void) XSetAfterFunction(display, afterfunction);
/* Call XSynchronize with onoff set to False. */
	onoff = False;
	proc = XCALL;
/* Verify that XSynchronize returned afterfunction. */
	if (proc != afterfunction) {
		report("Did not return previous after function.");
		FAIL;
	}
	else
		CHECK;
/* Call XSetAfterFunction to set after function to afterfunction. */
	(void) XSetAfterFunction(display, afterfunction);
/* Call XSynchronize with onoff set to True. */
	onoff = True;
	proc = XCALL;
/* Verify that XSynchronize returned afterfunction. */
	if (proc != afterfunction) {
		report("Did not return previous after function.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
