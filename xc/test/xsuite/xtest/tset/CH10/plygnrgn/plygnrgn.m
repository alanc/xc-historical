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
>>TITLE XPolygonRegion CH10
Region
XPolygonRegion(points, n, fill_rule)
XPoint	*points;
int	n = 99;
int	fill_rule;
>>EXTERN
XPoint	poly1[] = { {20,20}, {35,10}, {55,10}, {80,20}, {90,35}, {85,50}, {75,65}, {50,70}, {30,65}, {20,60}, {15,40} };
XPoint	poly2[] = { {75,60}, {60,45}, {55,60}, {30,55}, {35,35}, {15,40}, {20,18}, {45,10}, {70,15}, {55,30}, {80,45} };
XPoint	poly3[] = { {85,40}, {30,20}, {20,55}, {30,69}, {55,35}, {85,44}, {10,43}, {14,66} };
/*
 * The same polygon used in XFillPolygon tests.
 */
XPoint   compshape[] = { {10, 10},  {80, 10},  {100, 30},  {65, 60},  {65, 30}, {85, 60},
                         {30, 50},  {30, 80},  {60, 80},   {60, 5},   {70, 5},  {70, 100}, {10, 100},
};

struct prec {
	XPoint	*points;
	int	size;
} poly[] = { 	{poly1, NELEM(poly1)},
		{poly2, NELEM(poly2)},
		{compshape, NELEM(compshape)}	};

>>ASSERTION Good A
A call to xname returns a region 
comprising the interior of the
filled polygon as described by the 
.A n
points
specified by the
.A points
argument, filled with the
.A fill_rule
argument.
>>STRATEGY
For a convex, concave and complex polygon :
   For fill_rule = EvenOddRule, WindingRule :
      Create a polygon region using xname.
      Create a gc using XCreateGC.
      Set the foreground of the GC to W_FG ^ W_BG.
      Create a drawable with background W_BG using XCreateWindow.
      Fill the polygon on the drawable using XFillPolygon.
      Set the GC clipmask to the region using XSetRegion.
      Set the graphics function of the GC to GXxor using XSetFunction.
      Fill the drawable using XFillRectangle.
      Verify that the entire drawable is W_BG.
>>CODE
int     loop;
XImage		im;
XVisualInfo	*vi;
Region	reg;
Window	win;
GC	gc;
unsigned int    width;
unsigned int    height;
int x;

	resetvinf(VI_WIN);
	nextvinf(&vi);

	for(loop=0; loop < NELEM(poly); loop++) {
	
		win = makewin(Dsp, vi);
		gc = makegc(Dsp, win);
		fill_rule = EvenOddRule;
		XSetFillRule(Dsp, gc, fill_rule);
		XSetFillStyle(Dsp, gc, FillSolid);
		XSetFunction(Dsp, gc, GXxor);
		XSetForeground(Dsp, gc, W_BG^W_FG);
		points = poly[loop].points;
		n = poly[loop].size;
		reg = XCALL;

		getsize(Dsp, win, &width, &height);
		XFillPolygon(Dsp, win, gc, points, n, Complex, CoordModeOrigin);
		if( checkarea(Dsp, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL|CHECK_DIFFER)){
			delete("XFillPolygon() did set any pixels to non-background values.");
			return;
		} else
			CHECK;
		XSetRegion(Dsp, gc, reg);
		XFillRectangle(Dsp, win, gc, 0,0, width, height);	

		if( checkarea(Dsp, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL) == 0){
			report("When used as a clip-mask in a GC,");
			report("the polygon region did not produce the expected clipping.");
			report("Results depend on the correctness of XFillPolygon().");
			FAIL;
		} else
			CHECK;
		XDestroyRegion(reg);

		win = makewin(Dsp, vi);
		gc = makegc(Dsp, win);
		fill_rule = WindingRule;
		XSetFillRule(Dsp, gc, fill_rule);
		XSetFillStyle(Dsp, gc, FillSolid);
		XSetFunction(Dsp, gc, GXxor);
		XSetForeground(Dsp, gc, W_BG^W_FG);
		reg = XCALL;

		getsize(Dsp, win, &width, &height);
		XFillPolygon(Dsp, win, gc, points, n, Complex, CoordModeOrigin);
		if( checkarea(Dsp, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL|CHECK_DIFFER)){
			delete("XFillPolygon() did set any pixels to non-background values.");
			return;
		} else
			CHECK;
		XSetRegion(Dsp, gc, reg);
		XFillRectangle(Dsp, win, gc, 0,0, width, height);	

		if( checkarea(Dsp, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL) == 0){
			report("When used as a clip-mask in a GC,");
			report("the polygon region did not produce the expected clipping.");
			report("Results depend on the correctness of XFillPolygon().");
			FAIL;
		} else
			CHECK;
		XDestroyRegion(reg);
	}

	CHECKPASS(NELEM(poly) * 4);
