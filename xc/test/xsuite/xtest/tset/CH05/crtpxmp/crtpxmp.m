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
>>TITLE XCreatePixmap CH05
void
XCreatePixmap(display, d, width, height, depth)
Display *display = Dsp;
Drawable d = DRW(display);
unsigned int width = 13;
unsigned int height = 17;
unsigned int depth;
>>ASSERTION Good A
A call to xname creates a pixmap of width
.A width ,
height
.A height ,
and depth
.A depth
on the same screen as the drawable argument
.A d ,
and returns a pixmap ID.
>>STRATEGY
For each supported pixmap depth:
  Create a pixmap of height 13, width 17.
  Verify the depth, height and width with XGetGeometry.
>>CODE
XVisualInfo	*vp;
Status 		gstat;
Window		root_ret;
int		x_ret, y_ret;
unsigned int	w_ret, h_ret, bw_ret, bh_ret, dep_ret;
Pixmap		pmap;


	for(resetvinf(VI_PIX); nextvinf(&vp); ) {
		depth = vp->depth;
		d = DRW(display);
		pmap = XCALL;
		gstat = XGetGeometry(display, pmap, &root_ret, &x_ret, &y_ret, &w_ret, &h_ret, &bw_ret, &dep_ret);

		if(gstat != True) {
			delete("XGetGeometry did not return True for pixmap ID 0x%x",pmap);
			return;
		} else
			CHECK;

		if(w_ret != width) {
			report("XGetGeometry returned width of %u instead of %u for pixmap of depth %u", w_ret, width, depth);
			FAIL;
		} else 
			CHECK;

		if(h_ret != height) {
			report("XGetGeometry returned height of %u instead of %u for pixmap of depth %u", h_ret, height, depth);
			FAIL;
		} else 
			CHECK;

		if(dep_ret != depth) {
			report("XGetGeometry returned depth of %u instead of depth %u", dep_ret, depth);
			FAIL;
		} else 
			CHECK;

 	}

	CHECKPASS(4 * nvinf());

>>ASSERTION Good A
When the drawable argument
.A d
is an
.S InputOnly
window, then no error occurs.
>>STRATEGY
For each supported pixmap depth:
  Create an InputOnly window.
  Call XCreatePixmap with the window as the drawable argument.
  Verify that no error occurred.
>>CODE
XVisualInfo	*vp;

	for(resetvinf(VI_PIX); nextvinf(&vp); ) {
		d = iponlywin(display);
		depth = vp->depth;
		XCALL;

		if(geterr() != Success) {
			report("XCreatePixmap() produced error %s with an InputOnly Window as the drawable",errorname(geterr()));
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(nvinf());

>>ASSERTION Bad A
When either the
.A width
argument or 
.A height
argument is zero, then a
.S BadValue 
error occurs.
>>STRATEGY
For each supported pixmap depth:
  Create a pixmap with height = 17 and width = 0 with XCreatePixmap.
  Verify that a BadValue error occurred.
  Create a pixmap with height = 0 and width = 19 with XCreatePixmap.
  Verify that a BadValue error occurred.
>>CODE BadValue
XVisualInfo	*vp;
unsigned int	depth;

	for(resetvinf(VI_PIX); nextvinf(&vp); ) {
		depth = vp->depth;

		d = DRW(display); 
		height = 17;
		width = 0;
		XCALL;
		if(geterr() != BadValue) {
			report("When XCreatePixmap called with zero width");
			report("Got %s, Expecting BadValue error", 
							errorname(geterr()));
			FAIL;
		} else
			CHECK;

		width = 19;
		height = 0;
		XCALL;
		if(geterr() != BadValue) {
			report("When XCreatePixmap called with zero height");
			report("Got %s, Expecting BadValue error", 
							errorname(geterr()));
			FAIL;
		} else
			CHECK;
	}	

	CHECKPASS(nvinf() * 2);

>>ASSERTION Bad A
When the depth
.A depth
is not supported by the screen of the drawable
.A d,
then a
.S BadValue 
error occurs.
>>STRATEGY
Call XCreatePixmap with depth argument set to the 
	sum of all supported pixmap depths plus 1.
Verify that a BadValue error occurred.
>>CODE BadValue
XVisualInfo	*vp;
unsigned int	depth, nondepth;

	nondepth = 1;
	for(resetvinf(VI_PIX); nextvinf(&vp); )
		nondepth += vp->depth;	

	trace("Selected unsupported depth is %u", nondepth);
	d = DRW(display);
	depth = nondepth;
	XCALL;
	if(geterr() != BadValue) {
		report("When XCreatePixmap called with unsupported depth %d",
								depth);
		report("Got %s, Expecting BadValue error", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER Drawable
>>#HISTORY	Cal	Completed	Written in new format and style.
>>#HISTORY	Kieron	Action		<Have a look>
>>#HISTORY	Cal	Action		Writing code.
