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
>>SET   macro
>>TITLE XVendorRelease CH02
char *
XVendorRelease(display)
Display	*display = Dsp;
>>ASSERTION Good B 1
A call to xname returns a number related to a vendor's release of the X server.
>>STRATEGY
Obtain the value of the vendor's X server release using xname.
>>CODE
int	vr;

	vr = XCALL;
	if(vr != config.vendor_release) {
		report("%s() returned %d instead of %d.", TestName, vr, config.vendor_release);
		FAIL;
	} else
		PASS;
