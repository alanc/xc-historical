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
>>TITLE XClipBox CH10

XClipBox(r, rect_return)
Region		r;
XRectangle	*rect_return;
>>ASSERTION Good A
A call to xname returns in the
.A rect_return
argument the smallest rectangle enclosing the region
.A r .
>>EXTERN
Status
checknotclear(disp, d, fg)
Display		*disp;
Drawable	d;
unsigned long	fg;
{
XImage			*im;
unsigned long		pix;
unsigned int		width;
unsigned int		height;
int			x;
int			y;

	getsize(Dsp, d, &width, &height);
	
        if ((im = XGetImage(disp, d, 0, 0, width, height, AllPlanes, ZPixmap)) == (XImage*) NULL) {
                delete("XGetImage failed");
                return(False);
        }

	for(y=0; y<height; y++)		
		for(x=0; x<width; x++)
			if(XGetPixel(im, x, y) == fg)
				return(True);
	return(False);
}
>>STRATEGY
Create a region using XPolygonRegion.
Create a gc using XCreateGC.
Create a window using XCreateWindow.
Set the clip mask of the gc to the region.
Fill the entire drawable with W_FG using XFillRectangle.
Verify that the drawable is not completely W_BG.
Set the foreground of the gc to W_BG.
Obtain the smallest rectangle enclosing the polygon using xname.
Draw the returned rectangle in W_BG using XFillRectangle.
Verify that the drawable is completely W_BG

Create a window using XCreateWindow.
Fill the entire drawable with W_FG using XFillRectangle and the first gc.
Verify that the drawable is not completely W_BG.
Draw the smallest enclosing rectangle offset by +1+0 in W_BG using XFillRectangle and the second gc.
Verify that the drawable is not completely W_BG.

Create a window using XCreateWindow.
Fill the entire drawable with W_FG using XFillRectangle and the first gc.
Verify that the drawable is not completely W_BG.
Draw the smallest enclosing rectangle offset by -1+0 in W_BG using XFillRectangle and the second gc.
Verify that the drawable is not completely W_BG.

Create a window using XCreateWindow.
Fill the entire drawable with W_FG using XFillRectangle and the first gc.
Verify that the drawable is not completely W_BG.
Draw the smallest enclosing rectangle offset by +0+1 in W_BG using XFillRectangle and the second gc.
Verify that the drawable is not completely W_BG.

Create a window using XCreateWindow.
Fill the entire drawable with W_FG using XFillRectangle and the first gc.
Verify that the drawable is not completely W_BG.
Draw the smallest enclosing rectangle offset by +0-1 in W_BG using XFillRectangle and the second gc.
Verify that the drawable is not completely W_BG.
>>CODE
static XPoint   points[] = { {10, 10},  {74, 10},  {75, 30},  {65, 60},  {65, 30}, {75, 60},
                         {30, 50},  {30, 74},  {60, 74},   {60, 5},   {70, 5},  {70, 75}, {10, 85} };
Region		reg;
XRectangle	rect;
XVisualInfo	*vi;
Window		win;
GC		gc;
GC		gc2;
unsigned int		width;
unsigned int		height;

	reg = XPolygonRegion( points, NELEM(points), WindingRule);
	resetvinf(VI_WIN);
	nextvinf(&vi);
	win = makewin(Dsp, vi);
	gc = makegc(Dsp, win);
	gc2 = makegc(Dsp, win);
	XSetRegion(Dsp, gc, reg);
	XSetForeground(Dsp, gc, W_FG);
	XSetForeground(Dsp, gc2, W_BG);
	getsize(Dsp, win, &width, &height);
	XFillRectangle(Dsp, win, gc, 0,0, width, height);	

	if(checknotclear(Dsp, win, W_FG) == 0) {
		delete("XFillRectangle did not set any pixel to foreground.");
		return;
	} else
		CHECK;

	r = reg;
	rect_return = &rect;
	XCALL;

	XDestroyRegion(reg);

	XFillRectangle(Dsp, win, gc2, rect.x, rect.y, rect.width, rect.height);	

	if(checknotclear(Dsp, win, W_FG) != 0 ) {
		report("With a region used as the clip-mask in a GC,");
		report("%s() did not return a sufficiently large rectangle.", TestName);
		FAIL;
		return;
	} else
		CHECK;

	win = makewin(Dsp, vi);
	XFillRectangle(Dsp, win, gc, 0,0, width, height);	

	if(checknotclear(Dsp, win, W_FG) == 0) {
		delete("XFillRectangle did not set any pixel to foreground.");
		return;
	} else
		CHECK;
	XFillRectangle(Dsp, win, gc2, rect.x+1, rect.y, rect.width, rect.height);	

	if(checknotclear(Dsp, win, W_FG) == 0 ) {
		report("With a region used as the clip-mask in a GC,");
		report("%s() did not return a sufficiently small rectangle.in the x-direction.", TestName);
		FAIL;
	} else
		CHECK;

	win = makewin(Dsp, vi);
	XFillRectangle(Dsp, win, gc, 0,0, width, height);	

	if(checknotclear(Dsp, win, W_FG) == 0) {
		delete("XFillRectangle did not set any pixel to foreground.");
		return;
	} else
		CHECK;
	XFillRectangle(Dsp, win, gc2, rect.x-1, rect.y, rect.width, rect.height);	

	if(checknotclear(Dsp, win, W_FG) == 0 ) {
		report("With a region used as the clip-mask in a GC,");
		report("%s() did not return a sufficiently small rectangle in the x-direction.", TestName);
		FAIL;
	} else
		CHECK;

	win = makewin(Dsp, vi);
	XFillRectangle(Dsp, win, gc, 0,0, width, height);	

	if(checknotclear(Dsp, win, W_FG) == 0) {
		delete("XFillRectangle did not set any pixel to foreground.");
		return;
	} else
		CHECK;

	XFillRectangle(Dsp, win, gc2, rect.x, rect.y+1, rect.width, rect.height);	

	if(checknotclear(Dsp, win, W_FG) == 0 ) {
		report("With a region used as the clip-mask in a GC,");
		report("%s() did not return a sufficiently small rectangle in the y-direction.", TestName);
		FAIL;
	} else
		CHECK;

	win = makewin(Dsp, vi);
	XFillRectangle(Dsp, win, gc, 0,0, width, height);	

	if(checknotclear(Dsp, win, W_FG) == 0) {
		delete("XFillRectangle did not set any pixel to foreground.");
		return;
	} else
		CHECK;

	XFillRectangle(Dsp, win, gc2, rect.x, rect.y-1, rect.width, rect.height);	

	if(checknotclear(Dsp, win, W_FG) == 0 ) {
		report("With a region used as the clip-mask in a GC,");
		report("%s() did not return a sufficiently small rectangle in the y-direction.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(10);
