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
>>TITLE IsKeypadKey CH10

IsKeypadKey(keysym)
KeySym	keysym;
>>EXTERN
#define XK_LATIN1
#define XK_MISCELLANY
#include	"keysymdef.h"
#undef	XK_MISCELLANY
#undef XK_LATIN1
>>ASSERTION Good A
When the
.A keysym
argument is a keypad key, then
invocation of the xname macro
returns
.S True .
>>STRATEGY
For each keypad key KeySym:
   Verify that xname returns True.
>>CODE
static KeySym ks[] = {
			XK_KP_Space,
			XK_KP_Tab,
			XK_KP_Enter,
			XK_KP_F1,
			XK_KP_F2,
			XK_KP_F3,
			XK_KP_F4,
			XK_KP_Equal,
			XK_KP_Multiply,
			XK_KP_Add,
			XK_KP_Separator,
			XK_KP_Subtract,
			XK_KP_Decimal,
			XK_KP_Divide,
			XK_KP_0,
			XK_KP_1,
			XK_KP_2,
			XK_KP_3,
			XK_KP_4,
			XK_KP_5,
			XK_KP_6,
			XK_KP_7,
			XK_KP_8,
			XK_KP_9,
			0 };
KeySym	*ksp;
Bool	res;

	for(ksp = ks; *ksp; ksp++) {
		keysym = *ksp;
		res = XCALL;
		if(res != True) {
			char	*kstr = XKeysymToString(*ksp);

			report("%s() did not return True for KeySym XK_%s (value %lu).",
				TestName, kstr != (char *) NULL ? kstr : "<KeySym Undefined>", *ksp);
			FAIL;
		} else
			CHECK;

	}

	CHECKPASS(NELEM(ks) - 1);

>>ASSERTION Good A
When the
.A keysym
argument is not a keypad key, then
invocation of the xname macro returns
.S False .
>>STRATEGY
Verify that xname returns False for the KeySym XK_A.
>>CODE
Bool	res;

	keysym = XK_A;
	res = XCALL;
	if(res != False) {
		char	*kstr = XKeysymToString(keysym);

		report("%s() did not return False for KeySym XK_%s (value %lu).",
			TestName, kstr != (char *) NULL ? kstr : "<KeySym Undefined>", keysym);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
