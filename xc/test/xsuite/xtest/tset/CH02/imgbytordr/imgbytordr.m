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
>>TITLE XImageByteOrder CH02
int
XImageByteOrder(display)
Display	*display = Dsp;
>>EXTERN
static	char	*msbstr = "MSBFirst";
static	char	*lsbstr = "LSBFirst";
static	char 	errstr[9];

static
char *
sorder(order)
int 	order;
{
	switch (order) {

	case MSBFirst:
		return msbstr;
	case LSBFirst:
		return lsbstr;
	default:
		sprintf(errstr, "%d", order);
		return errstr;
	}
}
>>ASSERTION Good A
A call to xname returns the byte order, either
.S LSBFirst
or 
.S MSBFirst ,
for images for each scanline unit in 
XY format or for each pixel value in Z format.
>>STRATEGY
Obtain an XImage structure using XGetImage.
Obtain the byte order using xname.
Verify that the byte orders are the same.
>>CODE
int	iorder;
XImage	*xi;

	iorder = XCALL;	
	xi = XGetImage(display, DRW(display), 0,0, 1,1, XAllPlanes(), ZPixmap);

	if(iorder !=  xi->byte_order) {
		report("%s() returned %s instead of %s.", TestName, sorder(iorder), sorder(xi->byte_order));
		FAIL;
	} else
		PASS;

	XFree((char *) xi);
