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
>>TITLE XUnionRectWithRegion CH10

XUnionRectWithRegion(rectangle, src_region, dest_region_return)
XRectangle	*rectangle;
Region		src_region;
Region		dest_region_return;
>>ASSERTION Good A
A call to xname computes the union of the rectangle
.A rectangle
and the region
.A src_region
and stores the result in
.A dest_region_return .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a rectangle using xname.
Obtain the extents of the region using XClipBox.
Verify that the position and dimension of the region is the same as that of the rectangle.
>>CODE
static XRectangle	rect = { 7,9, 23,37 };
XRectangle		rrect;
Region 			R1;


	R1 = makeregion();
	rectangle = &rect;
	src_region = R1;
	dest_region_return = R1;
	XCALL;

	XClipBox(R1, &rrect);

	if( (rect.x != rrect.x) || (rect.y != rrect.y) || (rect.width != rrect.width) || (rect.height != rrect.height)) {
		report("%s() set a rectangle x - %d y - %d width - %d height - %d",
			 TestName,
			 rrect.x, rrect.y, rrect.width, rrect.height);
		report("instead of x - %d y - %d width - %d height - %d.",
			 rect.x, rect.y, rect.width, rect.height);

		FAIL;
	} else
		PASS;
