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
>>TITLE XSetWindowBackground CH03
void

Display *display = Dsp;
Window	w;
unsigned long	background_pixel = W_FG;
>>EXTERN
static Window	parent;
static struct	area	ap;

static void
inittp()
{
	tpstartup();

	ap.x = 50;
	ap.y = 60;
	ap.width = 20;
	ap.height= 20;
}
>>SET tpstartup inittp
>>ASSERTION Good A
A call to xname sets the background of the window to the pixel value
specified by
.A background_pixel .
>>STRATEGY
Create a window.
Set the background_pixel by calling xname.
Verify the background pixel was set to background_pixel using XGetPixel.
>>CODE
XEvent	event;

	parent = defdraw(display, VI_WIN);

	w = creunmapchild(display, parent, &ap);

	XCALL;

	XSelectInput(display, w, ExposureMask);	
	XMapWindow(display, w);
	XWindowEvent(display,w,ExposureMask, &event); /* Await exposure */

	if(getpixel(display, w, 1, 1) != W_FG) {
		report("%s did not set the background pixel to W_FG.", TestName);
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(1);
>>ASSERTION Good A
When the background is changed, then the
window contents are not changed.
>>STRATEGY
Create a window.
Set the background-pixmap
Map window over a plain background.
Change background pixel. 
Verify that background has not changed.
>>CODE
Pixmap	pm;
XEvent event;
XVisualInfo	*vp;
XSetWindowAttributes	attributes;

	for(resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makewin(display, vp);

		w = creunmapchild(display, parent, &ap);

		pm = maketile(display, w);
		XSetWindowBackgroundPixmap(display, w, pm);

		XSelectInput(display, w, ExposureMask);
		XMapWindow(display, w);
					/* Wait for window to be exposed */
		XWindowEvent(display,w,ExposureMask, &event);

		background_pixel = W_BG;
		XCALL;

		if ( isdeleted() )
			continue;

		if (checktile(display, w, NULL, 0, 0, pm))
			CHECK;
		else
			FAIL;

	}

	CHECKPASS(nvinf());

>>ASSERTION Bad A
.ER BadMatch wininputonly
>>ASSERTION Bad A
.ER BadWindow 
