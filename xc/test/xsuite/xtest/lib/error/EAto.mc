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
>>ASSERTION Bad A
>>### Atom
When an atom argument does not name a valid xerrlist, then a
.S BadAtom
error occurs.
>>STRATEGY
Call test function using a value with the top bits set as the atom argument.
Verify that a BadAtom error occurs.
>>CODE BadAtom

	A_ATOM = ~0L;
	XCALL;

	if(geterr() == BadAtom)
		PASS;
	else
		FAIL;
