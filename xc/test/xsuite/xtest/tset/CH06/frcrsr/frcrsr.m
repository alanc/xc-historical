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
>>TITLE XFreeCursor CH06
void
XFreeCursor(display, cursor)
Display *display = Dsp;
Cursor cursor;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good B 1
When another resource ID references the 
.A cursor ,
then a call to xname
deletes the association between the
.A cursor
resource ID 
and the specified cursor.
>>STRATEGY
Create cursor and cursor2 as same cursors.
Create window.
Define cursor for window.
Call XFreeCursor with cursor.
Call XFreeCursor with cursor2.
>>CODE
Cursor cursor2;
Window w;
XVisualInfo *vp;
unsigned int shape;


	/* UNSUPPORTED is not allowed */
	shape = config.fontcursor_good;
	if (shape == -1) {
		delete("A value of UNSUPPORTED is not allowed for XT_FONTCURSOR_GOOD");
		return;
	}

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
/* Create cursor and cursor2 as same cursors. */
		cursor = XCreateFontCursor(display, shape);
		cursor2 = XCreateFontCursor(display, shape);

/* Create window. */
		w = makewin(display, vp);

/* Define cursor for window. */
		XDefineCursor(display, w, cursor);

/* Call XFreeCursor with cursor. */
		XCALL;
		if (geterr() == Success)
			CHECK;
		else
			FAIL;

/* Call XFreeCursor with cursor2. */
		cursor = cursor2;
		XCALL;

		if (geterr() == Success)
			CHECK;
		else
			FAIL;
	}
	
	CHECKUNTESTED(2*nvinf());
>>ASSERTION Good B 1
When no other resource ID references the 
.A cursor ,
then a call to xname
deletes the association between the
.A cursor
resource ID 
and the specified cursor,
and the cursor storage is freed.
>>STRATEGY
Create cursor and cursor2 as same cursors.
Call XFreeCursor with cursor.
Call XFreeCursor with cursor2.
>>CODE
Cursor cursor2;
unsigned int shape;

	/* UNSUPPORTED is not allowed */
	shape = config.fontcursor_good;
	if (shape == -1) {
		delete("A value of UNSUPPORTED is not allowed for XT_FONTCURSOR_GOOD");
		return;
	}

/* Create cursor and cursor2 as same cursors. */
	cursor = XCreateFontCursor(display, shape);
	cursor2 = XCreateFontCursor(display, shape);

/* Call XFreeCursor with cursor. */
	XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;

/* Call XFreeCursor with cursor2. */
	cursor = cursor2;
	XCALL;

	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	
	CHECKUNTESTED(2);
>>ASSERTION Bad A
.ER BadCursor 
>># HISTORY kieron Completed    Check format and pass ac
>>#HISTORY peterc Completed Wrote STRATEGY and CODE
