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
>>TITLE XAllocClassHint CH09
XClassHint *
XAllocClassHint()
>>ASSERTION Good A
A call to xname allocates and returns a pointer to a
.S XClassHint 
structure in which each component is set to NULL.
>>STRATEGY
Allocate an XClassHint structure with XAllocClassHint.
Verify that each component of the structure is set to NULL.
>>CODE
XClassHint *chint;

	chint = XCALL;
	
	if(chint == (XClassHint *) NULL) {
		delete("XAllocClassHint() returned NULL.");
		return;
	} else
		CHECK;

	if(chint->res_name != NULL) {
		report("The res_name component of the XClassHints structure was not NULL.");
		FAIL;
	} else
		CHECK;

	if(chint->res_class != NULL) {
		report("The res_class component of the XClassHints structure was not NULL.");
		FAIL;
	} else
		CHECK;

	XFree((char*)chint);

	CHECKPASS(3);


>>ASSERTION Good B 1
When insufficient memory is available, then a call
to xname returns NULL.
>># Kieron	Action	Review
