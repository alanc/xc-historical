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
>>TITLE XDestroyRegion CH10

XDestroyRegion(r)
Region	r;
>>ASSERTION Bad B 1
A call to xname deallocates the storage associated with the region
.A r .
>>STRATEGY
Create a gc using XCreateGC.
For 20 iterations:
   Create a region using XPolygonRegion.
   Set the gc clip mask to the region using XSetRegion.
   Destroy the region using xname.
>>CODE
int		i;
int		loopcount = 20;
GC		gc;
static int	rule[] = { EvenOddRule, WindingRule };
static XPoint	points[] = { 	{20,20}, {35,10}, {55,10}, {80,20}, {90,35}, {85,50}, {75,65}, {50,70}, {30,65}, {20,60}, {15,40},
				{75,60}, {60,45}, {55,60}, {30,55}, {35,35}, {15,40}, {20,18}, {45,10}, {70,15}, {55,30}, {80,45},
				{85,40}, {30,20}, {20,55}, {30,69}, {55,35}, {85,44}, {10,43}, {14,66} };

	gc = makegc(Dsp, DRW(Dsp));
	for(i=0; i<loopcount; i++) {
		r = XPolygonRegion(points, NELEM(points), rule[i % 2]);
		XSetRegion(Dsp, gc, r);
		XCALL;
		CHECK;
	}

	CHECKUNTESTED(loopcount);
