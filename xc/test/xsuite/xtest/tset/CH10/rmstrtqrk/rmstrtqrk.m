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
>>TITLE XrmStringToQuark CH10
XrmQuark

char *string;
>>SET startup rmstartup
>>ASSERTION Good A
A call to xname returns a quark allocated to represent
.A string .
>>STRATEGY
Call xname to allocate a quark for a string.
Call XrmQuarkToString to obtain representation for the quark.
Verify the quark represents the string.
>>CODE
char *s = "stq_one";
XrmQuark ret;
char *rep;

/* Call xname to allocate a quark for a string. */
	string = s;
	ret = XCALL;

/* Call XrmQuarkToString to obtain representation for the quark. */
	rep = XrmQuarkToString(ret);

#ifdef TESTING
	rep = "barfed";
#endif

/* Verify the quark represents the string. */
	if((rep==(char *)NULL) || strcmp(s,rep)) {
		FAIL;
		report("%s did not allocate a quark representing the string",
			TestName);
		report("Returned quark was: %d", (int)ret);
		report("Expected representation: %s", s);
		report("Returned representation: %s",
			(rep==(char *)NULL?"NULL pointer":rep));
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good A
When a quark already exists for
.A string ,
then a call to xname returns that quark.
>>STRATEGY
Call xname to allocate a quark for a string.
Call xname to allocate a quark for the string again.
Verify that the quarks were the same.
>>CODE
char *s = "stq_two";
XrmQuark ret1, ret2;

/* Call xname to allocate a quark for a string. */
	string = s;
	ret1 = XCALL;

/* Call xname to allocate a quark for the string again. */
	ret2 = XCALL;

/* Verify that the quarks were the same. */
	if (ret1 != ret2) {
		FAIL;
		report("%s did not return the same quark to represent",
			TestName);
		report("the same string.");
		report("1st quark return: %d", (int) ret1);
		report("2nd quark return: %d", (int) ret2);
	} else
		CHECK;

	CHECKPASS(1);
