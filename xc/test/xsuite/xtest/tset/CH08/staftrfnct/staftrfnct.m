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
>>TITLE XSetAfterFunction CH08
int ((*)())()
XSetAfterFunction(display, procedure)
Display *display = Dsp;
int (*procedure)() = afterfunction;
>>EXTERN
static	int	counter = 0;

static int
afterfunction(display)
Display *display;
{
	trace("In afterfunction(), screen number %d", XDefaultScreen(display));
	return(counter++);
}
static int
_afterfunction(display)
Display *display;
{
	trace("In _afterfunction(), screen number %d", XDefaultScreen(display));
	return(--counter);
}
>>ASSERTION Good A
A call to xname
sets the after function to
.A procedure .
>>STRATEGY
Create window.
Verify that afterfunction was not called.
Call XSetAfterFunction to set the after function to afterfunction.
Create window.
Verify that the after function was called.
>>CODE
int	oldcounter;

	oldcounter = counter;
/* Create window. */
	(void) mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Verify that afterfunction was not called. */
	if (counter != oldcounter) {
		delete("After function already set to afterfunction.");
		return;
	}
	else
		CHECK;
/* Call XSetAfterFunction to set the after function to afterfunction. */
	(void) XCALL;
/* Create window. */
	(void) mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Verify that the after function was called. */
	if (counter == oldcounter) {
		report("After function not called.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good A
A call to xname
returns the previous after function.
>>STRATEGY
Call XSetAfterFunction to set after function to afterfunction.
Call XSetAfterFunction to set after function to _afterfunction.
Verify that XSetAfterFunction returned afterfunction.
Call XSetAfterFunction to set after function to afterfunction.
Verify that XSetAfterFunction returned _afterfunction.
>>CODE
int	(*proc)();

/* Call XSetAfterFunction to set after function to afterfunction. */
	procedure = afterfunction;
	(void) XCALL;
/* Call XSetAfterFunction to set after function to _afterfunction. */
	procedure = _afterfunction;
	proc = XCALL;
/* Verify that XSetAfterFunction returned afterfunction. */
	if (proc != afterfunction) {
		report("Returned 0x%x, expected 0x%x", proc, afterfunction);
		FAIL;
	}
	else
		CHECK;
/* Call XSetAfterFunction to set after function to afterfunction. */
	procedure = afterfunction;
	proc = XCALL;
/* Verify that XSetAfterFunction returned _afterfunction. */
	if (proc != _afterfunction) {
		report("Returned 0x%x, expected 0x%x", proc, _afterfunction);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good B 6
>>#NOTE Category B, reason 6 (unreasonable amount of test development time).
After a call to xname,
all Xlib functions that generate protocol requests call
.A procedure
before returning.
