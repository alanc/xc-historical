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
>>TITLE XFree CH02
>>ASSERTION Good A
There are no specific assertions in section 2 for xname.
Assertions for other Xlib functions refer to xname
(as in the Xlib specification).
>>STRATEGY
Put out a message explaining that there are no assertions in 
section 2 for xname, and that assertions for other Xlib functions 
refer to xname.
>>CODE

	report("There are no specific assertions in section 2 for %s.", 
								TestName);
	report("Assertions for other Xlib functions refer to %s.",
								TestName);
	tet_result(TET_NOTINUSE);
