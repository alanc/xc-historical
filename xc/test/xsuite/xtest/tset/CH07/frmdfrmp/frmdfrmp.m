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
>>TITLE XFreeModifiermap CH07
void

XModifierKeymap	*modmap;
>>ASSERTION Good B 3
A call to xname frees the specified
.S XModifierKeymap
structure.
>>STRATEGY
Create modifier map.
Call xname to free it.
Touch test only.
>>CODE

	modmap = XNewModifiermap(0);

	XCALL;

	untested("There is no reliable test method, but a touch test was performed");
