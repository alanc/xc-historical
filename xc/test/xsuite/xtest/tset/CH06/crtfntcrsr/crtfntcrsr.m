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
>>TITLE XCreateFontCursor CH06
Cursor
XCreateFontCursor(display, shape)
Display *display = Dsp;
unsigned int shape;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good B 1
When the
.A shape 
argument is
a defined glyph in the standard cursor font,
then a call to xname creates a
.S Cursor
with a black foreground and a white background
corresponding to the 
.A shape
argument
and returns the cursor ID.
>>STRATEGY
Get TET variable XT_FONTCURSOR_GOOD.
Call XCreateFontCursor with shape of this value.
Verify that XCreateFontCursor returns non-zero.
>>CODE
Cursor	qstat;

/* Get TET variable XT_FONTCURSOR_GOOD */
	/* UNSUPPORTED is not allowed */
	shape = config.fontcursor_good;
	if (shape == -1) {
		delete("A value of UNSUPPORTED is not allowed for XT_FONTCURSOR_GOOD");
		return;
	}

	trace("Shape used is %x", shape);
/* Call XCreateFontCursor with shape of this value, */
/* else call XCreateFontCursor with shape of zero. */
	
	qstat = XCALL;

/* Verify that XCreateFontCursor returns non-zero. */
	if (qstat == 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Bad A
When the
.A shape 
argument is not
a defined glyph in the standard cursor font,
then a
.S BadValue
error occurs.
>>STRATEGY
Get TET variable XT_FONTCURSOR_BAD.
Call XCreateFontCursor with shape of this value.
Verify that a BadValue error occurs.
>>CODE BadValue
Cursor	qstat;

/* Get TET variable XT_FONTCURSOR_BAD */
	shape = config.fontcursor_bad;
	if (shape == -1) {
		unsupported("There are no invalid cursor glyph values");
		return;
	}

	trace("Shape used is %x", shape);
/* Call XCreateFontCursor with shape of this value, */
/* else call XCreateFontCursor with shape of zero. */
	XCALL;

/* Verify that a BadValue error occurs. */
	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 1
.ER BadAlloc
>># HISTORY kieron Completed	Reformat to pass ca, plus correction
>># HISTORY peterc Completed	Wrote STRATEGY and CODE.
