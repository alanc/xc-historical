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
>>TITLE XProtocolVersion CH02
int
XProtocolVersion(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname returns the major version number (11) of the X protocol associated with 
the display specified by the
.A display
argument.
>>STRATEGY
Obtain the protocol version of the connection using xname.
>>CODE
int	version;

	version = XCALL;
	if(version != config.protocol_version) {
		report("%s() returned %d instead of %d.",TestName, version, config.protocol_version);
		FAIL;
	} else
		PASS;
