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
>>TITLE XUnloadFont CH06
void

Display	*display = Dsp;
Font	font;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good A
When another resource or resource ID references the
.A font ,
then a call to xname
deletes the association between the
.A font
resource ID 
and the specified font.
>>STRATEGY
Load a font.
Call XUnloadFont.
Verify that the font ID is no longer usable.
>>CODE
GC		gc;

	gc = makegc(display, DRW(display));
	if (isdeleted())
		return;
	font = XLoadFont(display, "xtfont0");

	XCALL;

	/* Try to use the font */
	XSetErrorHandler(error_status);
	reseterr();
	XSetFont(display, gc, font);
	XDrawString(display, DRW(display), gc, 30, 30, "abc", 3);
	XSync(display, 0);
	switch (geterr()) {
	case Success:
		report("font ID was still usable");
		FAIL;
		break;
	case BadFont:
		PASS;
		break;
	default:
		delete("Unexpected error in draw string");
		break;
	}
	XSetErrorHandler(unexp_err);
	
>>ASSERTION Good B 3
When no other resource or resource ID references the
.A font ,
then a call to xname
deletes the association between the
.A font
resource ID 
and the specified font 
and the font itself will be freed.
>>ASSERTION Bad A
.ER BadFont bad-font
>># HISTORY kieron Completed	Reformat and tidy to ca pass
