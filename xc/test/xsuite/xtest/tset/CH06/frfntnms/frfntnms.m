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
>>TITLE XFreeFontNames CH06
void

char	**list;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good B 3
>># As resolved by MIT, this test is now done using 
>># the return value from XListFonts and XListFontsWithInfo.
>>#
>># Some X11R4 implementations may give memory faults when the return 
>># value of XListFontsWithInfo is passed to XFreeFontNames, but this 
>># is an implementation fault which should be fixed in X11R5.
>>#
>># DPJ Cater	5/4/91
When 
.A list
is a list of font names 
returned by a call to XListFonts,
then a call to xname
frees
.A list
and the font names specified by
.A list .
>>STRATEGY
Get list of names with XListFonts.
Call XFreeFontNames to free list of names.
Verify that no error occurred.
Get list of names with XListFontsWithInfo.
Call XFreeFontNames to free list of names.
Verify that no error occurred.
Result is UNTESTED, unless an error should occur.
>>CODE
int 	count;
XFontStruct	*info;

	list = XListFonts(Dsp, "xtfont?", 4, &count);
	if (list == NULL) {
		delete("XListFonts failed");
		return;
	}

	XCALL;

	if (geterr() == Success)
		CHECK;
	else {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	}

	list = XListFontsWithInfo(Dsp, "xtfont?", 4, &count, &info);
	if (list == NULL) {
		delete("XListFontsWithInfo failed");
		return;
	}

	XCALL;

	if (geterr() == Success)
		CHECK;
	else {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	}

	CHECKUNTESTED(2);

>># HISTORY kieron Completed	Reformat and tidy to ca pass
