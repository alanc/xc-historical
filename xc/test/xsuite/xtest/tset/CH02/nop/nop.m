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
>>TITLE XNoOp CH02

XNoOp(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname sends a
.S NoOperation
protocol request to the server.
>>STRATEGY
Obtain the serial number of the next request using XNextRequest.
Issue a NoOperation protocol request using XNoOp.
Obtain the serial number of the next request using XNextRequest.
Verify that the serial number has changed.
>>CODE
int	nr, lr;

	lr = XNextRequest(display);
	_startcall(display);
	XNoOp(display);
	_endcall(display);

	if( geterr() != Success) {
		report("Got %s, Expecting Success", errorname(geterr()));
                FAIL;
        } else
		CHECK;

	nr = XNextRequest(display);

	if(nr == lr) {
		report("%s() did not cause the next serial number to change.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(2);
