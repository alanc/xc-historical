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
>>TITLE XAllPlanes CH02
unsigned long
XAllPlanes()
>>ASSERTION Good A
A call to xname returns an unsigned long value with all bits set to 1.
>>STRATEGY
Call XAllPlanes.
Verify result was ~0L.
>>CODE
unsigned long	result;

	result = XCALL;
	if ( result != ~0L ) {
		report("%s() returned %lu instead of %lu", result, ~0L);
		FAIL;
	} else
		PASS;

>>#
>># Note - the next assertion cannot be automatically derived from the 
>># previous one, using the >>SET macro command, because mc can only produce
>># automatically generated tests for macros which have arguments.
>># So, it is included here separately.
>># Note - the assertion is really for AllPlanes, not XAllPlanes.
>>#
>>ASSERTION Good A
AllPlanes returns an unsigned long value with all bits set to 1.
>>STRATEGY
Invoke AllPlanes.
Verify result was ~0L.
>>CODE
unsigned long	result;

	result = AllPlanes;
	if ( result != ~0L ) {
		report("%s() returned %lu instead of %lu", result, ~0L);
		FAIL;
	} else
		PASS;
