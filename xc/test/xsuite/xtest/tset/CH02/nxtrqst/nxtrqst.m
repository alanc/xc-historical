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
>>TITLE XNextRequest CH02
unsigned long
XNextRequest(display)
Display	*display = Dsp;
>>ASSERTION Good B 1
A call to xname returns the serial number of the next request over the
connection specified by the
.A display
argument.
>>STRATEGY
Obtain the serial number of the last request processed by the server using XLastKnownRequestProcessed.
Obtain the serial number of the next request to be sent using xname.
Verify that the two serial numbers are not the same.
>>CODE
unsigned long	sno, nno;

	sno = XLastKnownRequestProcessed(display);
	nno = XCALL;
	report("Last processed serial number was %lu, next serial number is %lu.", sno, nno);
	if(sno == nno) {
		report("The next serial number (%lu) was not different to the current serial number.", nno);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);

>>ASSERTION Good B 1
Serial numbers are maintained separately for each display connection.
>>STRATEGY
Open two displays using XOpenDisplay.
Obtain the next serial number for each display using xname.
Perform an XNoOp request on the first display.
Verify that the next serial number for that display is not the same.
Verify that the next serial number for the second display is the same.
>>#
>># COMMENT :	The mad posturing with _startcall and _endcall is to
>>#		avoid the XSync calls which are planted by XCALL as
>>#		these call increment the serial number.
>>#
>># Cal 23/07/91
>>#
>>CODE
unsigned long	nr1, nr2, nar1, nar2;
Display		*disp2;

	disp2 = opendisplay();

	_startcall(display);
	nr1 = XNextRequest(display);
	_endcall(display);
	if (geterr() != Success) {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	_startcall(disp2);
	nr2 = XNextRequest(disp2);
	_endcall(disp2);
	if (geterr() != Success) {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	XNoOp(display);

	_startcall(display);
	nar1 = XNextRequest(display);
	_endcall(display);
	if (geterr() != Success) {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	_startcall(disp2);
	nar2 = XNextRequest(disp2);
	_endcall(disp2);

	if (geterr() != Success) {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	if(nr1 == nar1) {
		delete("The next serial number (%lu) was the same after a XNoOp request.", nr1);
		return;
	} else
		CHECK;


	if(nr2 != nar2) {
		report("The next serial number changed without a request.");
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(6);







