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
>>TITLE XFreeFontPath CH06
void

char	**list;
>>ASSERTION Good B 3
>># NOTE	kieron	Untestable.
When
.A list
is an array of strings allocated by a call to
.S XGetFontPath ,
then a call to xname frees
.A list 
and the strings specified by
.A list .
>>STRATEGY
Touch test.
Get a font path with XGetFontPath.
Call XFreeFontPath.
Result is UNTESTED.
>>CODE
int 	n;

	list = XGetFontPath(Dsp, &n);
	if (list == NULL || isdeleted()) {
		delete("Could not get font path to free");
		return;
	}

	XCALL;

	tet_result(TET_UNTESTED);
>># HISTORY kieron Completed	Reformat and tidy to ca pass
