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
>>TITLE XRectInRegion CH10
int
XRectInRegion(r, x, y, width, height)
Region		r;
int		x;
int		y;
unsigned int	width;
unsigned int	height;
>>EXTERN
static XRectangle	medrect = { 10,10, 15,5 };
>>ASSERTION Good A
When the rectangle specified by the
.A x ,
.A y ,
.A width ,
and
.A height
arguments is entirely in the region
.A r ,
then a call to xname returns
.S RectangleIn .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a rectangle using XUnionRectWithRegion.
Verify that with a contained rectangle xname returns RectangleIn.
>>CODE
Region			R;
int			res;

	R = makeregion();
	if(isdeleted()) return;

	XUnionRectWithRegion(&medrect, R, R);
	r = R;
	x=10;	
	y=10;
	width=15;
	height=5;

	res = XCALL;

	if(res != RectangleIn) {
		report("%s() returned %d instead of RectangleIn (%d).", TestName, res, RectangleIn);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the rectangle specified by the
.A x ,
.A y ,
.A width ,
and
.A height
arguments is entirely out of the region
.A r ,
then a call to xname returns
.S RectangleOut .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a rectangle using XUnionRectWithRegion.
Verify that with a non-intersecting rectangle xname returns RectangleOut.
>>CODE
Region			R;
int			res;

	R = makeregion();
	if(isdeleted()) return;

	XUnionRectWithRegion(&medrect, R, R);
	r = R;
	x=35;	
	y=15;
	width=5;
	height=2;

	res = XCALL;

	if(res != RectangleOut) {
		report("%s() returned %d instead of RectangleOut (%d).", TestName, res, RectangleOut);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the rectangle specified by the
.A x ,
.A y ,
.A width ,
and
.A height
arguments is partly in the region
.A r ,
then a call to xname returns
.S RectanglePart .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a rectangle using XUnionRectWithRegion.
Verify that with an intersecting rectangle xname returns RectanglePart.
>>CODE
Region			R;
int			res;

	R = makeregion();
	if(isdeleted()) return;

	XUnionRectWithRegion(&medrect, R, R);
	r = R;
	x=20;	
	y=5;
	width=4;
	height=10;

	res = XCALL;

	if(res != RectanglePart) {
		report("%s() returned %d instead of RectanglePart (%d).", TestName, res, RectanglePart);
		FAIL;
	} else
		PASS;

