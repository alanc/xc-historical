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
>>TITLE XSubtractRegion CH10

XSubtractRegion(sra, srb, dr_return)
Region	sra;
Region	srb;
Region	dr_return;
>>ASSERTION Good A
A call to xname subtracts the region
.A srb
from region
.A sra
and stores the result in region
.A dr_return .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a rectangle using XUnionRectWithRegion.
Subtract an encompassing region from itself using xname.
Verify that the result is the empty region using XEmptyRegion.
Subtract an enclosed region from and encompassing one using xname.
Verify that the result is not the empty region using XEmptyRegion.
Combine this result with the enclosed region.
Verify that the result is the original region.
>>CODE
Region			R1;
Region			R2;
Region			Rres;
static XRectangle	rect = { -43, -54, 122, 23};
static XRectangle	rect2 = { -44, -55, 123, 24};

	R1 = makeregion();
	R2 = makeregion();
	Rres = makeregion();

	if(isdeleted()) return;

	XUnionRectWithRegion(&rect, R1, R1);
	XUnionRectWithRegion(&rect2, R2, R2);
	
	sra = R1;
	srb = R2;
	dr_return = Rres;
	XCALL;

	if(XEmptyRegion(Rres) != True) {
		report("%s() did not return the empty region.", TestName);
		FAIL;
	} else
		CHECK;

	sra = R2;
	srb = R1;
	dr_return = Rres;
	XCALL;

	if(XEmptyRegion(Rres) == True) {
		report("%s() returned the empty region.", TestName);
		FAIL;
	} else
		CHECK;

	XXorRegion(R1,Rres, Rres);
	if(XEqualRegion(Rres,R2) == False) {
		report("%s() did not return expected region.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
