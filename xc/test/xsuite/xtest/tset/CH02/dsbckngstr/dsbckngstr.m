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
>>TITLE XDoesBackingStore CH02
int
XDoesBackingStore(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>EXTERN
static char	*wm = "WhenMapped";
static char	*al = "Always";
static char	*nu = "NotUseful";
static char	er[9];
static char *
bs(bs)
int bs;
{
	switch(bs) {
	case WhenMapped :
		return wm;
	case Always :
		return al;
	case NotUseful :
		return nu;
	default :
		sprintf(er, "%d", bs);
		return er;
	}
}
>>ASSERTION Good A
A call to xname returns 
.S WhenMapped , 
.S NotUseful , 
or
.S Always 
to indicate whether the screen
.A screen
supports backing stores.
>>STRATEGY
Obtain the level of support for backing store using xname.
>>CODE
int	dbs;
int	cdbs;

	switch(config.does_backing_store) {
	case 0:
		cdbs = NotUseful;
		break;
	case 1:
		cdbs = WhenMapped;
		break;
	case 2:
		cdbs = Always;
		break;
	default:
		delete("XT_DOES_BACKING_STORE was not set to 0, 1 or 2");
		return;
	}

	dbs = XCALL;
	if(cdbs !=  dbs) {
		report("%s() returned %s (%d) instead of %s (%d).\n", TestName, bs(dbs), dbs, bs(cdbs), cdbs);
		FAIL;		
	} else
		PASS;
