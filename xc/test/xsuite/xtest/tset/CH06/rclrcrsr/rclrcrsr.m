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
>>TITLE XRecolorCursor CH06
void
XRecolorCursor(display, cursor, foreground_color, background_color)
Display *display = Dsp;
Cursor cursor;
XColor *foreground_color = mkcolor(1);
XColor *background_color = mkcolor(0);
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN

/*
 * mkcolor() -	return a pointer to a color structure.
 *		flag indicates whether or not color is foreground
 */
static XColor *
mkcolor(flag)
{
	static	XColor	fore;
	static	XColor	back;
	static	int	first = 1;

	if (first)
	{
		first = 0;

		fore.pixel = BlackPixel(display, DefaultScreen(display));
		XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), &fore);
		back.pixel = WhitePixel(display, DefaultScreen(display));
		XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), &back);
	}
	return(flag ? &fore : &back);
}
>>ASSERTION Good B 1
A call to xname changes the color of the specified cursor,
.A cursor ,
to the specified
.A foreground_color
and
.A background_color .
>>STRATEGY
Create cursor.
Call XRecolorCursor to change foreground to W_BG and
background to W_FG.
>>CODE

/* Create cursor. */
	cursor = makecur(display);

/* Call XRecolorCursor to change foreground to W_BG and */
/* background to W_FG. */

	XCALL;

	if (geterr() != Success)
		FAIL;
	else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When the cursor is being displayed on a screen, then
the change is visible immediately.
>>ASSERTION Bad A
.ER BadCursor 
>># HISTORY kieron Completed    Reformat and tidy to ca pass
>># HISTORY peterc Completed Wrote STRATEGY and CODE
