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
>>TITLE XKeysymToString CH10
char *
XKeysymToString(keysym)
KeySym	keysym;
>>EXTERN
#define XK_MISCELLANY
#include "keysymdef.h"
#undef  XK_MISCELLANY
>>ASSERTION Good A
A call to xname returns 
as a null-terminated string
the name of the
.S KeySym
specified by the
.A keysym
argument.
>>STRATEGY
Obtain the string corresponding to the KeySym XK_BackSpace using xname.
Verify that the returned string was "BackSpace".
>>CODE
KeySym	ks = XK_BackSpace;
char	*value = "BackSpace";
char	*res;

	keysym = ks;
	res = XCALL;	

	if( res == (char *) NULL) {
		report("%s() returned NULL for KeySym XK_BackSpace.", TestName);
		FAIL;
	} else {
		CHECK;
		if(strcmp(res, value) != 0) {
			report("%s() returned \"%s\" instead of \"%s\" for KeySym XK_Backspace.",
				TestName, res, value);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2);

>>ASSERTION Good A	
When the
.S KeySym
specified by the
.A keysym
argument
is not defined, then a call to xname returns NULL.
>>STRATEGY
Obtain the string corresponding to the KeySym NoSymbol using xname.
Verify that the returned string was NULL
>>CODE
KeySym	ks = NoSymbol;
char	*res;

	keysym = ks;
	res = XCALL;	

	if( res != (char *) NULL) {
		report("%s() returned \"%s\" instead of NULL for KeySym NoSymbol.",
			TestName, res);
		FAIL;
	} else
		PASS;
