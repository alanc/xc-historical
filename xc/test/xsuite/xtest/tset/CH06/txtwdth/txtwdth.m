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
>>TITLE XTextWidth CH06
int

XFontStruct	*font_struct;
char	*string;
int 	count;
>>ASSERTION Good A
A call to xname returns the sum of the character-width metrics of all
characters in the 8-bit character string,
.A string ,
as rendered in the font referenced by
.A font_struct .
>>STRATEGY
The known good font information structures are used so that these tests
  are isolated from XLoadQueryFont.
Make a string consisting of all characters from 0 to 255.
Call XTextWidth.
Verify by direct calculation from the metrics.
>>CODE
extern	struct	fontinfo	fontinfo[];
extern	int 	nfontinfo;
int 	i;
int 	width;
int 	calcwidth;
char	buf[256];

	for (i = 0; i < 256; i++)
		buf[i] = i;
	string = buf;
	count  = 256;

	for (i = 0; i < nfontinfo; i++) {
		font_struct = fontinfo[i].fontstruct;

		width = XCALL;

		calcwidth = txtwidth(font_struct, (unsigned char *)string, count);

		if (width != calcwidth) {
			report("Font %s - width was %d, expecting %d", fontinfo[i].name,
				width, calcwidth);
			FAIL;
		} else
			CHECK;
	}
	CHECKPASS(nfontinfo);
>># HISTORY kieron Completed    Reformat and tidy to ca pass
