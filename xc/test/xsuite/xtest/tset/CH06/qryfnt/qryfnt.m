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
>>TITLE XQueryFont CH06
XFontStruct *

Display	*display = Dsp;
XID 	font_ID;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN
extern	int 	checkfsp();
extern	struct	fontinfo fontinfo[];
extern	int 	nfontinfo;
>>ASSERTION Good A
>># Improved the wording a bit from that approved ....
>># When the font with font_ID
>># .A font_ID
When the
.A font_ID
argument
is a valid Font resource,
then a call to xname returns a pointer to an
.S XFontStruct 
structure which contains information on
the font
with font ID
.A font_ID .
>>STRATEGY
For each xtest font
  Load font with XLoadFont.
  Set font_ID to font
  Call XQueryFont.
  Verify returned XFontStruct with known good one.
>>CODE
XFontStruct	*fsp;
Font	font;
int 	i;

	for (i = 0; i < nfontinfo; i++) {
		trace("Loading font %s", fontinfo[i].name);
		font = XLoadFont(Dsp, fontinfo[i].name);
		if (isdeleted())
			return;

		font_ID = font;
		fsp = XCALL;
		if (checkfsp(fsp, fontinfo[i].fontstruct, *fontinfo[i].string))
			CHECK;
		else {
			report("Returned XFontStruct was incorrect");
			FAIL;
		}
	}
	CHECKPASS(nfontinfo);
>>ASSERTION Good A
When the
.A font_ID
argument
specifies a
.S GContext ,
then a call to xname returns a pointer to an
.S XFontStruct 
structure which contains information on
the font in the corresponding GC's
.M font
field.
>>STRATEGY
Load font with 
>>CODE
XFontStruct	*fsp;
Font	font;
GC		gc;
Drawable	d;
int 	i;

	d = defdraw(Dsp, VI_WIN_PIX);
	gc = makegc(Dsp, d);
	if (isdeleted())
		return;

	for (i = 0; i < nfontinfo; i++) {
		trace("Loading font %s", fontinfo[i].name);
		font = XLoadFont(Dsp, fontinfo[i].name);
		if (isdeleted())
			return;

		XSetFont(Dsp, gc, font);

		font_ID = XGContextFromGC(gc);
		fsp = XCALL;
		if (checkfsp(fsp, fontinfo[i].fontstruct, *fontinfo[i].string))
			CHECK;
		else {
			report("Returned XFontStruct was incorrect");
			FAIL;
		}
	}
	CHECKPASS(nfontinfo);
>>ASSERTION Good A
When the
.A font_ID
argument
does not name a valid GContext or Font resource,
then a call to xname returns
.S NULL .
>>STRATEGY
Obtain a bad font ID.
Call XQueryFont.
Verify that null is returned.
>>CODE
Font	font;
XFontStruct	*fsp;

	font_ID = badfont(Dsp);
	fsp = XCALL;

	if (fsp != NULL) {
		report("A non-NULL pointer was returned");
		FAIL;
	} else
		PASS;
		
>># The following has been removed, because it is not true. ..sr
>># >>ASSERTION Bad A
>># .ER BadFont bad-fontable
>># HISTORY kieron Completed	Reformat and tidy to ca pass
