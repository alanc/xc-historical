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
>>TITLE XDeleteModifiermapEntry CH07
XModifierKeymap *
xname
XModifierKeymap *modmap;
KeyCode	keycode_entry;
int 	modifier;
>>ASSERTION Good A
A call to xname
deletes the specified KeyCode,
.A keycode_entry ,
from the set that controls the specified
.A modifier
and returns a pointer to the modified
.S XModifierKeymap
structure.
>>STRATEGY
Create modifiermap.
Insert some entries into it.
Call xname to delete entries.
Verify by direct inspection that the entries are gone.
>>CODE
int 	minkc, maxkc;
int 	kc;
int 	rm1, rm2;
int 	set;
int 	mod;
KeyCode	*kcp;
static int 	mods[] = {
	ShiftMapIndex,
	LockMapIndex,
	ControlMapIndex,
	Mod1MapIndex,
	Mod2MapIndex,
	Mod3MapIndex,
	Mod4MapIndex,
	Mod5MapIndex,
};

	XDisplayKeycodes(Dsp, &minkc, &maxkc);
	kc = minkc;

	modmap = XNewModifiermap(0);

	modmap = XInsertModifiermapEntry(modmap, ++kc, ControlMapIndex);
	rm1 = kc;
	modmap = XInsertModifiermapEntry(modmap, ++kc, ControlMapIndex);
	modmap = XInsertModifiermapEntry(modmap, ++kc, ControlMapIndex);
	modmap = XInsertModifiermapEntry(modmap, ++kc, Mod2MapIndex);
	modmap = XInsertModifiermapEntry(modmap, ++kc, Mod2MapIndex);
	rm2 = kc;
	modmap = XInsertModifiermapEntry(modmap, ++kc, Mod2MapIndex);
	modmap = XInsertModifiermapEntry(modmap, ++kc, Mod2MapIndex);

	/* Remove two entries */
	keycode_entry = rm1;
	modifier = ControlMapIndex;
	modmap = XCALL;

	keycode_entry = rm2;
	modifier = Mod2MapIndex;
	modmap = XCALL;

	kcp = modmap->modifiermap;
	for (mod = 0; mod < NELEM(mods); mod++) {
		for (set = 0; set < modmap->max_keypermod; set++) {
			if (kcp[0] == rm1) {
				report("KeyCode %d still in map", rm1);
				FAIL;
			} else
				CHECK;
			if (kcp[0] == rm2) {
				report("KeyCode %d still in map", rm2);
				FAIL;
			} else
				CHECK;

			kcp++;
		}
	}

	CHECKPASS(2 * modmap->max_keypermod * NELEM(mods));
