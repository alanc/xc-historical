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
>>ASSERTION Bad B 1
>>### Alloc
>># This is not testable
When the server fails to allocate a required resource, then a
.S BadAlloc
error occurs.
>>STRATEGY
Report UNTESTED
>>CODE

	report("There is no reliable method of testing that a BadAlloc error is generated");
	tet_result(TET_UNTESTED);
