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
>>TITLE XGetFontPath CH06
char	**

Display	*display = Dsp;
int 	*npaths_return = &npaths;
>>EXTERN
int 	npaths;
>>ASSERTION Good A
>># NOTE kieron		names are impl. dependent but should match those set
>>#			by XSetFontPath....
A call to xname
allocates and returns an array of strings containing the search path
for font lookup and returns the number of strings in the
.A npaths_return
argument.
>>STRATEGY
Touch test - the ability to read back the path that was set is checked
  in XSetFont.
Call XGetFontPath.
Verify that return is non-NULL.
Verify that npaths_return is non-zero.
Verify that there are at least that many strings.
>>CODE
char	**paths;
int 	i;

	/*
	 * Assuming that the path is set to something here.
	 */
	paths = XCALL;
	if (paths == NULL) {
		report("return value was NULL");
		FAIL;
	} else
		CHECK;

	if (npaths == 0) {
		report("npaths_return was 0");
		FAIL;
	} else
		CHECK;

	for (i = 0; i < npaths; i++) {
		trace("got path component '%s'", paths[i]);
	}

	CHECKPASS(2);


>># HISTORY kieron Completed	Reformat and tidy to ca pass
