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
>>TITLE XEmptyRegion CH10
Bool
XEmptyRegion(r)
Region	r;
>>ASSERTION Good A
When the region
.A r
is empty, then a call to xname returns
.S True .
>>STRATEGY
Create a region using XCreateRegion.
Verify that the region is empty using xname.
Compute the intersection of two disjoint regions.
Verify that the region is empty using xname.
>>CODE
Region			reg;
Bool			res;
Region			R1;
Region			R2;
static XRectangle	rect1 = {0,0, 7,13 };
static XRectangle	rect2 = {8, 14, 12,12};

	reg = makeregion();
	R1 = makeregion();
	R2 = makeregion();

	if(isdeleted()) return;

	r = reg;
	res = XCALL;

	if(res != True) {
		report("%s() did not return True", TestName);
		report("for a region obtain by a call to XCreateRegion().");
		FAIL;
	} else
		CHECK;

	XUnionRectWithRegion(&rect1, R1, R1);
	XUnionRectWithRegion(&rect2, R2, R2);
	XIntersectRegion(R1, R2, R1);

	r = R1;
	res = XCALL;

	if(res != True) {
		report("%s() did not return True", TestName);
		report("for a region obtained from the union of two disjoint regions.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When the region
.A r
is not empty, then a call to xname returns
.S False .
>>STRATEGY
Create a region using XCreateRegion.
Set the region using XUnionRectWithRegion.
Verify that the region is non-empty using xname.
>>CODE
Bool			res;
Region			R1;
static	XRectangle	rect1 = {0,0, 1,1 };

	R1 = makeregion();	
	if(isdeleted()) return;

	XUnionRectWithRegion(&rect1, R1, R1);
	r = R1;
	res = XCALL;

	if(res != False) {
		report("%s() did not return False", TestName);
		report("for a region obtain by a call to XUnionRectWithRegion().");
		FAIL;
	} else
		PASS;
