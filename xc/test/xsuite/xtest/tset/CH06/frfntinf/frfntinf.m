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
>>TITLE XFreeFontInfo CH06
void

char	**names;
XFontStruct	*free_info;
int 	actual_count;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good B 3
>># NOTE	kieron	Untestable, really. No font IDs or other server
>>#			resident stuff is affected.
>># NOTE	kieron	The names and info *must* have been allocated
>>#			in the same way as by XListFontsWithInfo. More
>>#			accurately the names *cannot* have been returned by
>>#			XListFontNames or the XFree's won't match the Xalloc's
>>#			and subsequent, untestable and unpredictable, nastiness
>>#			will occur.
When
.A names
is a list of font names and
.A free_info
is a pointer to the font information
both returned by a call to 
.S XListFontsWithInfo ,
then a call to xname frees
.A names ,
the font names specified by 
.A names
and
.A free_info .
>>STRATEGY
Get names and free_info with XListFontsWithInfo.
Call XFreeFontInfo.
Result is UNTESTED, unless an error should occur.
>>CODE

	names = XListFontsWithInfo(Dsp, "xtfont?", 4, &actual_count, &free_info);
	if (names == NULL) {
		delete("No xtest fonts found, check that they have been installed");
		return;
	} else
		CHECK;

	XCALL;

	CHECKUNTESTED(1);
>>ASSERTION Good B 3
When names is NULL and free_info is a pointer to the font information
structure returned by an Xlib routine, then a call to xname frees
the storage associated with free_info.
>>STRATEGY
Get font information with XLoadQueryFont.
Call XFreeFontInfo.
Result is UNTESTED, unless some error should occur.
>>CODE
XFontStruct	*fsp;

	fsp = XLoadQueryFont(Dsp, "xtfont0");
	if (fsp == NULL) {
		delete("Could not open xtfont0, check that xtest fonts are installed");
		return;
	}

	names = NULL;
	free_info = fsp;
	actual_count = 1;
	XCALL;

	tet_result(TET_UNTESTED);
>># HISTORY kieron Completed	Reformat and tidy to ca pass
