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
>>TITLE XAllocWMHints CH09
XWMHints *
XAllocWMHints()
>>ASSERTION Good A
A call to xname allocates and returns a pointer to a
.S XWMHints 
structure, which can be freed with XFree,
in which each component is set to zero.
>>STRATEGY
Allocate a XWMHints structure with XAllocWMHints.
Verify that the call did not return NULL.
Verify that each component of the allocated structure was set to 0.
Free the allocated structure using XFree.
>>CODE
XWMHints	*hints;

	hints = XCALL;

	if(hints == (XWMHints *) NULL) {
		delete("XAllocWMHints returned NULL.");
		return;
	} else
		CHECK;

	if( hints->flags != 0L ) {
		report("The flags component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->input != (Bool) 0 ) {
		report("The input component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->initial_state != (int) 0 ) {
		report("The initial_state component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_pixmap != (Pixmap) 0 ) {
		report("The icon_pixmap component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_window != (Window) 0 ) {
		report("The icon_window component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_x != (int) 0 ) {
		report("The icon_x component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_y != (int) 0 ) {
		report("The icon_y component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_mask != (Pixmap) 0 ) {
		report("The icon_mask component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->window_group != (XID) 0 ) {
		report("The window_group component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	XFree((char*)hints);

	CHECKPASS(10);

>>ASSERTION Bad B 1
When insufficient memory is available, then
a call to xname returns NULL.
>># Completed	Kieron	Review
