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
>>TITLE XLastKnownRequestProcessed CH02
unsigned long
XLastKnownRequestProcessed(display)
Display	*display = Dsp;
>>ASSERTION Good B 1
A call to xname returns the serial number of the last request known to have
been processed by the server over the connection specified by the
.A display
argument .
>>STRATEGY
Obtain the serial number of the last request processed by the server using xname.
Obtain the serial number of the next request to be sent using XNextRequest.
Verify that the two serial numbers are not the same.
>>CODE
unsigned long	sno, nno;

	sno = XCALL;
	nno = XNextRequest(display);
	report("Last processed serial number was %lu, next serial number is %lu.", sno, nno);
	if(sno == nno) {
		report("The next serial number (%lu) was not different to the current serial number.", nno);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
