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
>>TITLE XrmQuarkToString CH10
char *

XrmQuark quark;
>>SET startup rmstartup
>>ASSERTION Good A
A call to xname returns a pointer to the string that corresponds to
the
.A quark .
>>STRATEGY
Call XrmStringToQuark to allocate a quark for a string.
Call xname to obtain the representation for the quark.
>>CODE
char *s="qts_one";
char *ret;

/* Call XrmStringToQuark to allocate a quark for a string. */
	quark = XrmStringToQuark( s );

/* Call xname to obtain the representation for the quark. */
	ret = XCALL;

	if (ret==(char *)NULL || strcmp(s,ret)) {
		FAIL;
		report("%s did not return the representation for the quark.",
			TestName);
		report("Expected representation: %s", s);
		report("Returned representation: %s",
			(ret==(char *)NULL?"NULL pointer":ret));
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good A
When no string exists for a
.A quark ,
then a call to xname returns
.S NULL .
>>STRATEGY
Create a unique quark which has no string representation.
Call xname to obtain the representation for the quark.
Verify that a NULL pointer was returned.
>>CODE
char *ret;

/* Create a unique quark which has no string representation. */
	quark = XrmUniqueQuark();

/* Call xname to obtain the representation for the quark. */
	ret = XCALL;

/* Verify that a NULL pointer was returned. */
	if (ret != (char *)NULL) {
		FAIL;
		report("%s returned unexpected value with a quark with",
			TestName);
		report("no string representation.");
		report("Expected value: NULL pointer");
		report("Returned value: '%s'", ret);
	} else
		CHECK;

	CHECKPASS(1);

