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
>>TITLE XXorRegion CH10

XXorRegion(sra, srb, dr_return)
Region	sra;
Region	srb;
Region	dr_return;
>>ASSERTION Good A
A call to xname computes the difference between the union and intersection of
the regions
.A sra
and
.A srb
and stores the result in
.A dr_return .
>>STRATEGY
Create regions R1 R2 R3 Ru Ri Rx using XCreateRegion.
Set R1 to a polygon using XPolygonRegion.
Set R2 to a polygon which partially intersects R1 using XPolygonRegion.
Set Ru to the union of R1 and R2 using XUnionRegion.
Set Ri to the intersection of R1 and R2 using XIntersectRegion.
Set R3 to the difference between Ru and Ri using XSubtractUnion.
Set Rx to the difference between the union and intersection of R1 and R2 
  using xname.
Verify that Rx is the same as R3 using XEqualRegion.
>>CODE
Region		R1;
Region		R2;
Region		R3;
Region		Ru;
Region		Ri;
Region		Rx;
static XPoint	inter1[] = { {25,5}, {15,10}, {10,25}, {20,35}, {30,30}, {35,20}, {30,10} };
static XPoint	inter2[] = { {25,20}, {37,22}, {40,30}, {35,37}, {30,40}, {25,38}, {18,32}, {20,25} };

	R1 = makeregion();
	R2 = makeregion();
	R3 = makeregion();
	Ru = makeregion();
	Ri = makeregion();
	Rx = makeregion();
	
	if(isdeleted()) return;

	R1 = XPolygonRegion( inter1, NELEM(inter1), WindingRule );
	R2 = XPolygonRegion( inter2, NELEM(inter2), WindingRule );

	XUnionRegion(R1, R2, Ru);
	XIntersectRegion(R1, R2, Ri);
	XSubtractRegion(Ru, Ri, R3);	

	sra = R1;
	srb = R2;
	dr_return = Rx;
	XCALL;

	if(XEmptyRegion(Rx) == True) {
		report("%s() returned empty region.", TestName);
		FAIL;
	} else
		CHECK;


	if(XEqualRegion(R3, Rx) != True) {
		report("%s() did not return the difference between the union and intersection of its arguments.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
