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
>>TITLE XAllocIconSize CH09
XIconSize	*XAllocIconSize()
>>ASSERTION Good A
A call to xname allocates and returns a pointer to a
.S XIconSize 
structure in which each component is set to zero.
>>STRATEGY
Allocate a XIconSize structure using XAllocIconSize.
Verify that the function did not return NULL.
Verify that each component of the structure is 0.
Release the allocated memory using XFree.
>>CODE
XIconSize	*isize;

	isize = XCALL;

	if(isize == (XIconSize *) NULL) {
		delete("%s() returned NULL.", TestName);
		return;
	} else
		CHECK;

	if(isize->min_width != 0) {
		report("The min_width component of the XIconSize structure was %d instead of zero", isize->min_width);
		FAIL;
	} else
		CHECK;

	if(isize->min_height != 0) {
		report("The min_height component of the XIconSize structure was %d instead of zero", isize->min_height);
		FAIL;
	} else
		CHECK;

	if(isize-> max_width != 0) {
		report("The max_width component of the XIconSize structure was %d instead of zero", isize-> max_width);
		FAIL;
	} else
		CHECK;

	if(isize->max_height != 0) {
		report("The max_height component of the XIconSize structure was %d instead of zero", isize->max_height);
		FAIL;
	} else
		CHECK;

	if(isize->width_inc != 0) {
		report("The width_inc component of the XIconSize structure was %d instead of zero", isize->width_inc);
		FAIL;
	} else
		CHECK;

	if(isize-> height_inc != 0) {
		report("The height_inc component of the XIconSize structure was %d instead of zero", isize->height_inc);
		FAIL;
	} else
		CHECK;

	CHECKPASS(7);

>>ASSERTION Bad B 1
When insufficient memory is available, then
a call to xname returns NULL.
>># Kieron	Action	Review
