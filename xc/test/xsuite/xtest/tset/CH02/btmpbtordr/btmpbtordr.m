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
>>TITLE XBitmapBitOrder CH02
int
XBitmapBitOrder(display)
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
A call to xname returns the bitorder, either
.S LSBFirst
or 
.S MSBFirst ,
for a scanline unit.
>>STRATEGY
Obtain an XImage structure using XGetImage.
Obtain the bitmap bit order using xname.
Verify that the bitmap bit orders are the same.
>>CODE
int	border;
XImage	*xi;

	border = XCALL;	
	xi = XGetImage(display, DRW(display), 0,0, 1,1, XAllPlanes(), ZPixmap);

	if(border !=  xi->bitmap_bit_order) {
		report("%s() returned %s instead of %s.", TestName, sorder(border), sorder(xi->bitmap_bit_order));
		FAIL;
	} else
		PASS;

	XFree((char *) xi);
