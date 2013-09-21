/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: dssvundrs.m,v 1.8 94/04/17 21:02:24 rws Exp $
 */
>>SET   macro
>>TITLE XDoesSaveUnders CH02
Bool
XDoesSaveUnders(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>EXTERN
static char	*ts = "True";
static char	*fs = "False";
static char	es[9];

static char *
tfs(tf)
int	tf;
{
	switch(tf) {
	case True:
		return ts;
	case False:
		return fs;
	default:
		sprintf(es, "%d", tf);
		return es;
	}
}
>>ASSERTION Good C
If the screen
.A screen
supports save unders:
A call to xname returns True.
Otherwise:
A call to xname returns False.
>>STRATEGY
Determine whether the screen supports save unders using xname.
>>CODE
Bool dsu;

	dsu = XCALL;
	if(config.does_save_unders == 1)
		if(dsu != True) {
			report("%s() returned %s instead of True (%d).", TestName, tfs(dsu), True);
			FAIL;
		} else
			CHECK;
	else
		if(dsu != False) {
			report("%s() returned %s instead of False (%d).", TestName, tfs(dsu), False);
			FAIL;
		} else
			CHECK;

	CHECKPASS(1);
