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
>>### Value
>>STRATEGY
Obtain a sequence of values which are not in the list specified by VALUE_LIST.
For each value:
  Call test function with this value in the VALUE_ARG argument.
  Verify that a BadValue error occurs.
>>CODE BadValue
int 	i;
int 	n;
NOTMEMTYPE
long	vals[NM_LEN];

	seterrdef();

	n = NOTMEMBER(VALUE_LIST, NELEM(VALUE_LIST), vals);

	for (i = 0; i < n; i++) {

		debug(1, "Trying arg of %d", vals[i]);

		VALUE_ARG = vals[i];
		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}

	CHECKPASS(n);
