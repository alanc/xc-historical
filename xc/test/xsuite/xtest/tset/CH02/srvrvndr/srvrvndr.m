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
>>TITLE XServerVendor CH02
char *
XServerVendor(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname returns a pointer to a null-terminated string that
identifies the owner of the X server implementation.
>>STRATEGY
Obtain the server vendor string using xname.
Verify that the value is that given in parameter XT_SERVER_VENDOR.
>>CODE
char	*sv;

	if(!config.server_vendor) {
		delete("Parameter XT_SERVER_VENDOR not set.");
		return;
	}

	sv = XCALL;
	if(!sv) {
		report("%s() returned NULL", TestName);
		FAIL;
	} else if(strcmp(config.server_vendor, sv)) {
		report("%s() returns incorrect value for server vendor string", 
						TestName);
		report("Expected value \"%s\"", config.server_vendor);
		report("Observed value \"%s\"", sv);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
