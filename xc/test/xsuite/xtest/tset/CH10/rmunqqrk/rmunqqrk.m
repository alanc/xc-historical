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
>>TITLE XrmUniqueQuark CH10
XrmQuark

>>SET startup rmstartup
>>ASSERTION Good A
A call to xname returns a quark that does not represent any string
known to the resource manager.
>>STRATEGY
Call xname to obtain a unique quark.
Verify the quark does not represent a string in the resource manager.
Call xname to obtain another quark.
Verify this is a distinct quark.
>>CODE
XrmQuark ret1, ret2;
char *str;

/* Call xname to obtain a unique quark. */
	ret1 = XCALL;

/* Verify the quark does not represent a string in the resource manager. */
	str = XrmQuarkToString( ret1 );
	if (str != (char *)NULL) {
		FAIL;
		report("%s did not return a quark not representing a string.",
			TestName);
		report("XrmQuarkToString Expected: NULL pointer");
		report("XrmQuarkToString Returned: '%s'", str);
	} else
		CHECK;

/* Call xname to obtain another quark. */
	ret2 = XCALL;

/* Verify this is a distinct quark. */
	if (ret1 == ret2) {
		FAIL;
		report("%s returned indistinct quarks on consecutive calls.",
			TestName);
		report("1st quark: %d", (int)ret1);
		report("2nd quark: %d", (int)ret2);
	} else
		CHECK;

	CHECKPASS(2);
