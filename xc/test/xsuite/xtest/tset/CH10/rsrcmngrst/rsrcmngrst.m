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
>>TITLE XResourceManagerString CH10
char *
XResourceManagerString(display)
Display	*display = Dsp;
>>EXTERN
#include	"Xatom.h"
>>ASSERTION Good A
A call to xname returns the value of the RESOURCE_MANAGER
property on the root window of screen zero at the time
.A display
was opened.
>>STRATEGY
Set the value of the RESOURCE_MANAGER property to "XTest.test.resource:value" using XChangeProperty.
Open display using XOpenDisplay.
Set the value of the RESOURCE_MANAGER property to "XTest.changed.resource:value" using XChangeProperty.
Obtain the value of the RESOURCE_MANAGER property at the time display was opened using xname.
Verify that the call returned "XTest.test.resource:value".
>>CODE
char	*pval1 = "XTest.test.resource:value";
char	*pval2 = "XTest.changed.resource:value";
char	*res;

	XChangeProperty (Dsp, RootWindow(Dsp, 0), XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *)pval1, 1+strlen(pval1));
	XSync(Dsp, False);
	display = opendisplay();
	XChangeProperty (display, RootWindow(display, 0), XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *)pval2, 1+strlen(pval2));

	res = XCALL;

	if( res == (char *) NULL) {
		report("%s() returned NULL.", TestName);
		FAIL;
	} else {
		CHECK;
		if(strcmp(res, pval1) != 0) {
			report("%s() returned \"%s\" instead of \"%s\".", TestName, res, pval1);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2);
