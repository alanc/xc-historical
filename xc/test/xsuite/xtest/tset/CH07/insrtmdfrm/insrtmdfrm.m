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
>>TITLE XInsertModifiermapEntry CH07
XModifierKeymap *
xname
XModifierKeymap	*modmap = Modmap;
KeyCode	keycode_entry;
int 	modifier;
>>EXTERN

static XModifierKeymap	*Modmap;

>>ASSERTION Good A
A call to xname adds the specified KeyCode,
.A keycode_entry ,
to the set
that controls the specified
.A modifier
and returns a pointer to the modified
.S XModifierKeymap
structure.
>>STRATEGY
Create new modifiermap.
Insert several entries into the map with xname.
Verify by direct inspection that entries have been added.
>>CODE
struct	kcmod {
	KeyCode	kc;
	int 	mod;
	int 	found;
};
static	struct	kcmod	kcmod[] = {
	{0x34, ShiftMapIndex},
	{0x56, Mod3MapIndex},
	{0x76, ControlMapIndex},
	{0x77, ControlMapIndex},
	{0xF6, Mod3MapIndex},
};
int 	i;
int 	set;
extern	int 	NS_modifier;

	modmap = XNewModifiermap(0);
	if (modmap == 0) {
		delete("Could not create modifiermap");
		return;
	}

	for (i = 0; i < NELEM(kcmod); i++) {
		keycode_entry = kcmod[i].kc;
		modifier = kcmod[i].mod;

		modmap = XCALL;

		kcmod[i].found = 0;
	}

	for (i = 0; i < NELEM(kcmod); i++) {
		/* Look in the map */
		for (set = 0; set < modmap->max_keypermod; set++) {
			if (modmap->modifiermap[kcmod[i].mod*modmap->max_keypermod+set]==kcmod[i].kc)
				kcmod[i].found++;
		}
	}

	for (i = 0; i < NELEM(kcmod); i++) {
		if (kcmod[i].found)
			CHECK;
		else {
			report("Keycode 0x%x, modifier %s not added to map",
				kcmod[i].kc, modifiername(kcmod[i].mod));
			FAIL;
		}
	}

	CHECKPASS(NELEM(kcmod));
