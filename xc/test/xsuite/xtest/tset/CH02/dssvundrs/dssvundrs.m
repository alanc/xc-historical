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
