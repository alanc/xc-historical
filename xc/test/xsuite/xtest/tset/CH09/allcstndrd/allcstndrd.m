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
>>TITLE XAllocStandardColormap CH09
XStandardColormap *
XAllocStandardColormap()
>>#
>># Added note to mention that the allocated structure can
>># be freed with XFree.
>>#
>># Cal 29/05/91
>>ASSERTION Good A
A call to xname allocates and returns a pointer to a
.S XStandardColormap
structure, which can be freed with
.S XFree
in which each component
is set to zero.
>>STRATEGY
Allocate a XStandardColormap using XAllocStandardColormap.
Verify that the function did not return NULL.
Verify that each component of the XStandardColormap structure was 0.
Release the allocated structure using XFree.
>>CODE
XStandardColormap	*scmp = (XStandardColormap *) NULL;

	scmp = XAllocStandardColormap();

	if(scmp == (XStandardColormap *) NULL) {
		delete("%s() returned NULL.", TestName);
		return;
	} else
		CHECK;

	if(scmp->colormap != (Colormap) 0) {
		report("The colormap component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->red_max != 0L) {
		report("The red_max component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->red_mult != 0L) {
		report("The red_mult component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->green_max != 0L) {
		report("The green_max component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->green_mult != 0L) {
		report("The green_mult component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->blue_max != 0L) {
		report("The blue_max component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->blue_mult != 0L) {
		report("The blue_mult component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->base_pixel != 0L) {
		report("The base_pixel component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->visualid != (VisualID) 0) {
		report("The visualid component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->killid != (XID) 0) {
		report("The killid component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	XFree((char*)scmp);

	CHECKPASS(11);

>>ASSERTION Bad B 1
When insufficient memory is available, then
a call to xname returns NULL.
>># Kieron	Action	Review
