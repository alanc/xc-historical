/*
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, MIT, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, MIT, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: getvers.m,v 1.5 94/01/29 15:20:56 rws Exp $
 */

>>TITLE XGetExtensionVersion XINPUT
XExtensionVersion *

Display	*display = Dsp;
char *name = "XInputExtension";
>>EXTERN
extern ExtDeviceInfo Devs;
>>ASSERTION Good B 3
A successful call to xname with returns the extension version.
>>STRATEGY
Call xname.
>>CODE
XExtensionVersion *vers;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	vers = XCALL;
	if (geterr()!=Success)
	    FAIL;
	else
	    PASS;

>>ASSERTION Good B 3
A successful call to xname with returns an extension version
with a "present" field set to True, and a major and minor version
greater than or equal to XI_Initial_Release.
>>STRATEGY
Call xname.
>>CODE
XExtensionVersion *vers;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	vers = XCALL;
	if (geterr()!=Success)
	    FAIL;
	else
	    if (vers->present != True)
		{
		report("%s: Input Device Extension is not present.",TestName);
	        FAIL;
		}
	    else if (vers->major_version >= XI_Initial_Release_Major &&
	             vers->minor_version >= XI_Initial_Release_Minor)
		PASS;
	    else
		{
		report("%s: Wrong version was reported.",TestName);
	        FAIL;
		}

