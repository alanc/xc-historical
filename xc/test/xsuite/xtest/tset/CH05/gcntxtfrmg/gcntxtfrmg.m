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
>>TITLE XGContextFromGC CH05
GContext
XGContextFromGC(gc)
GC gc;
>>ASSERTION Good A
A call to xname returns the GContext resource ID for the
.A gc
argument.
>>STRATEGY
Create a GC
Obtain the GContext ID with XGContextFromGC
Obtain an XFontStruct from the gc with XQueryFont.
Verify that the fid component of the XFontStruct is the GContext ID.
>>CODE
XFontStruct *fontstruct;
GContext gcontext;

	gc = makegc(Dsp, DRW(Dsp));
	gcontext = XGContextFromGC(gc);	
	fontstruct = XQueryFont(Dsp, gcontext);
	
	if(fontstruct->fid != gcontext) {
		report("A call to XGContextFromGC returned a value which when passed to XQueryFont");
		report("did not return a correct GContext ID in fid field of XFontStruct");
		FAIL;
	} else
		PASS;


>>#HISTORY	Cal	Completed	Written in new style and format - 6/12/90.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writing code.
