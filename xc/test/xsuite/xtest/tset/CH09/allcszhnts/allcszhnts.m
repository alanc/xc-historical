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
>>TITLE XAllocSizeHints CH09
XSizeHints *
XAllocSizeHints()
>>ASSERTION Good A
A call to xname allocates and returns a pointer to a
.S XSizeHints 
structure, which can be freed with
.S XFree ,
in which each component is set to zero.
>>STRATEGY
Allocate an XSizeHints structure with XAllocSizeHints.
Verify NULL is not returned.
Verify that each component of the structure is set to zero.
Release the allocated memory using XFree.
>>CODE
XSizeHints	*shints;

	shints = XAllocSizeHints();

	if (shints == (XSizeHints *)NULL) {
		delete("XAllocSizeHints returned NULL.");
		return;
	} else
		CHECK;

	if(shints->flags != 0L) {
		report("The flags component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->x != 0) {
		report("The x component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->y != 0) {
		report("The y component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->width != 0) {
		report("The width component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->height != 0) {
		report("The height component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->min_width != 0) {
		report("The min_width component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->min_height != 0) {
		report("The min_height component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->max_width != 0) {
		report("The max_width component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->max_height != 0) {
		report("The max_height component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->width_inc != 0) {
		report("The width_inc component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->height_inc != 0) {
		report("The height_inc component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if((shints->min_aspect.x != 0) || (shints->min_aspect.y != 0)){
		report("The min_aspect components of the XSizeHints structure were not zero.");
		FAIL;
	} else
		CHECK;

	if((shints->max_aspect.x != 0) || (shints->max_aspect.y != 0)){
		report("The max_aspect components of the XSizeHints structure were not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->base_width != 0) {
		report("The base_width component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->base_height != 0) {
		report("The base_height component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;

	if(shints->win_gravity != 0) {
		report("The win_gravity component of the XSizeHints structure was not zero.");
		FAIL;
	} else
		CHECK;


	XFree((char *) shints);
	CHECKPASS(17);

>>ASSERTION Good B 1
When insufficient memory is available, then
a call to xname returns NULL.
>># Kieron	Completed	Review
