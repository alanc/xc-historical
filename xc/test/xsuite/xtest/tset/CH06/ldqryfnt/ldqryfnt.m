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
>>TITLE XLoadQueryFont CH06
XFontStruct *

Display	*display = Dsp;
char	*name = "xtfont0";
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good A
When the 
.A name
argument names an existing font, 
then a call to xname loads the font named
.A name ,
and returns a pointer to an
.S XFontStruct 
structure which contains information on the font.
>>STRATEGY
For each xtest font:
  Load and query font.
  Compare returned XFontStruct with known good structures.
>>EXTERN
extern	int 	checkfsp();
extern	struct	fontinfo fontinfo[];
extern	int 	nfontinfo;
>>CODE
XFontStruct	*fsp;
int 	i;


	for (i = 0; i < nfontinfo; i++) {
		name = fontinfo[i].name;
		trace("Loading %s", name);
		fsp = XCALL;
		if (checkfsp(fsp, fontinfo[i].fontstruct, *fontinfo[i].string))
			CHECK;
		else
			FAIL;
	}

	CHECKPASS(nfontinfo);

>>ASSERTION Good A
The font ID returned by a call to xname is usable 
on any GC created for any screen of the display.
>>STRATEGY
For each visual supported for the default screen:
  Load and query font "xtfont1".
  Create window.
  Create GC for window.
  Set font component in GC to loaded font.
  Draw string with single character.
  Pixmap verify.
Note: this tests the GC's for the default screen. 
To test for other screens, re-run the test suite with XT_DISPLAY set 
to number of required screen.
>>CODE
XVisualInfo	*vp;
Font	font;
XFontStruct	*fsp;
Drawable d;
GC	gc;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		name = fontinfo[1].name;

		fsp = XCALL;
		font = fsp->fid;

		if (geterr() != Success) {
			/* Already done after calling XLoadFont */
			report("font %s could not be loaded", name);
			report("Check that xtest fonts have been installed");
			FAIL;
		} else {
			d = makewin(display, vp);
			gc = makegc(display, d);
			XSetFont(display, gc, font);
			XDrawString(display, d, gc, 20, 20, "z", 1);
			PIXCHECK(display, d);
			dclear(display, d);
			XFreeFont(display, fsp);
		}
	}
	CHECKPASS(nvinf());


>>ASSERTION Good A
When the 
.A name
argument does not name an existing font, 
then xname returns NULL.
>>STRATEGY
Set name to a non-existant name.
Verify that NULL is returned.
>>CODE
XFontStruct	*fsp;

	name = config.bad_font_name;
	fsp = XCALL;

	if (fsp == NULL)
		PASS;
	else {
		report("NULL was not returned when the name was '%s'", name);
		FAIL;
	}
>>ASSERTION Good A
Upper and lower case characters in the
.A name
argument refer to the same font.
>>STRATEGY
Load font by name xtfont0.
Load font by name XtFoNt0.
Verify that this name also succeeds and that
the returned XFontStruct's are the same.
>>CODE
XFontStruct	*fsp1;
XFontStruct	*fsp2;

	name = "xtfont0";
	fsp1 = XCALL;
	if (fsp1 == NULL) {
		delete("Could not load font %s", name);
		return;
	}

	name = "XtFoNt0";
	fsp2 = XCALL;
	if (fsp2 == NULL) {
		report("Font could not be loaded with name %s", name);
		FAIL;
		return;
	}

	/*
	 * The checking of the copyright property is inhibited, so that
	 * this test can pass even if the first one doesn't.
	 */
	if (checkfsp(fsp2, fsp1, (char*)0))
		PASS;
	else {
		report("Different XFontStruct descriptions were returned");
		FAIL;
	}
>>ASSERTION Bad A
.ER BadAlloc 
>># HISTORY kieron Completed	Reformat and tidy to ca pass
