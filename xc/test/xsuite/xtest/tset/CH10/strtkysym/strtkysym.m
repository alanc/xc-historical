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
>>TITLE XStringToKeysym CH10
KeySym
XStringToKeysym(string)
char	*string;
>>EXTERN
#define XK_MISCELLANY
#include "keysymdef.h"
#undef  XK_MISCELLANY
>>ASSERTION Good A
A call to xname returns the
.S KeySym
value for the
.S KeySym 
name specified by the
null-terminated
.A string
argument.
>>STRATEGY
Obtain the KeySym corresponding to the string "Escape" using XStringToKeysym.
Verify that the returned value is XK_Escape.
>>CODE
KeySym	ks;
char	*tstr = "Escape";

	string = tstr;
	ks = XCALL;
	if(ks != XK_Escape) {
		report("%s() returned KeySym %lu instead of XK_Escape (%lu) for \"%s\".",
			TestName, (long) ks, (long) XK_Escape, tstr);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the specified string does not match a valid KeySym, then
a call to xname returns
.S NoSymbol . 
>>STRATEGY
Obtain the KeySym matching the string "XTest_No_Sym" using XStringToKeysym.
Verify that the returned value is NoSymbol.
>>CODE
KeySym	ks;
char	*tstr = "XTest_No_Sym";

	string = tstr;
	ks = XCALL;
	if(ks != NoSymbol) {
		report("%s() returned KeySym %lu instead of NoSymbol (%lu) for \"%s\".",
			TestName, (long) ks, (long) NoSymbol, tstr);
		FAIL;
	} else
		PASS;

