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
>>TITLE XGetInputFocus CH07
void

Display	*display = Dsp;
Window	*focus_return = &fretwin;
int 	*revert_to_return = &rtret;
>>EXTERN

static Window	fretwin;
static int 	rtret;

>>ASSERTION Good A
A call to xname
returns the focus window,
.S PointerRoot ,
or
.S None
to
.A focus_return
and the current focus revert state to
.A revert_to_return .
>>STRATEGY
Set focus state.
Call xname to get focus state.
Verify that it was the state that was set.
>>EXTERN
#define	REVERT_TO	RevertToPointerRoot

>>SET startup focusstartup
>>SET cleanup focuscleanup
>>CODE
Window	win;

	win = defwin(display);

	XSetInputFocus(display, win, REVERT_TO, CurrentTime);

	XCALL;

	if (*focus_return != win) {
		report("Incorrect window was returned");
		FAIL;
	} else
		CHECK;

	if (*revert_to_return != REVERT_TO) {
		report("The revert_to_return argument was incorrect");
		report("  was %d, expecting %d", *revert_to_return, REVERT_TO);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
