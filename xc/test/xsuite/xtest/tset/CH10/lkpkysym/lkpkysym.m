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
>>TITLE XLookupKeysym CH10
KeySym
XLookupKeysym(key_event, index)
XKeyEvent	*key_event;
int		index = 0;
>>ASSERTION Good A
A call to xname returns the
.S KeySym
element specified by
.A index
of the 
.S KeyCode
vector specified by the
.M keycode
member of the
.A key_event
argument .
>>STRATEGY
Obtain the maximum and minimum KeyCodes using XDisplayKeycodes.
Obtain the number of KeySyms per KeyCode using XGetKeyboardMapping.
For each KeyCode and index:
   Obtain the corresponding KeySym using XKeycodeToKeysym.
   Obtain the corresponding KeySym using xname.
   Verify that the KeySyms are identical.

>>CODE
int		minkc, maxkc;
int		syms_per_kc;
KeyCode		kc;
KeySym		ks, res;
KeySym		*keysyms;
XKeyEvent	ke;
int		i;

	XDisplayKeycodes(Dsp, &minkc, &maxkc);
	keysyms = XGetKeyboardMapping(Dsp, minkc, 1, &syms_per_kc);
	XFree((char *) keysyms);
	key_event = &ke;
	ke.display = Dsp;

	for(i=minkc; i<=maxkc; i++)
		for(index = 0; index < syms_per_kc; index++) {
			ks = XKeycodeToKeysym(Dsp, i, index);
			ke.keycode = i;
			res = XCALL;
			if(res != ks){
				report("%s() returned KeySym %lu instead of %lu for KeyCode %lu with index %d.", TestName, (long) res, (long) ks, (long) i, index);
				FAIL;
			} else
				CHECK;
		}


	CHECKPASS( (1+maxkc-minkc) * syms_per_kc);

>>ASSERTION Good A
When no
.S KeySym
is defined for the
.M keycode
of the event
.A key_event ,
then a call to xname returns
.S NoSymbol .
>>STRATEGY
Obtain the maximum and minimum KeyCodes using XDisplayKeycodes.
Obtain the number of KeySyms per KeyCode using XGetKeyboardMapping.
For each index:
   Obtain the KeySym for a KeyCode greater than maximum KeyCode using xname.
   Verify that the KeySym returned is NoSymbol.
>>CODE
int		minkc, maxkc;
int		syms_per_kc;
KeyCode		kc;
KeySym		res;
KeySym		*keysyms;
XKeyEvent	ke;

	XDisplayKeycodes(Dsp, &minkc, &maxkc);
	keysyms = XGetKeyboardMapping(Dsp, minkc, 1, &syms_per_kc);
	XFree( (char *) keysyms);
	kc = maxkc + 1;

	if(kc == minkc) {
		delete("Cannot establish an invalid KeyCode.");
		return;
	} else
		CHECK;

	key_event = &ke;
	ke.display = Dsp;
	ke.keycode = kc;

	for(index=0; index < syms_per_kc; index++) {

		res = XCALL;

		if(res != NoSymbol) {
			report("%s() returned %lu instead of NoSymbol (%lu) for invalid KeyCode %lu with index %d.", TestName, (long) res, (long) NoSymbol, (long) kc, index);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(1 + syms_per_kc);
