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
>>TITLE XSetWindowColormap CH05
void
XSetWindowColormap(display, w, colormap)
Display *display = Dsp;
Window w = DRW(display);
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
>>ASSERTION Good A
A call to xname sets the colourmap of the window argument
.A w
to the
.A colormap
argument.
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap.
  Create a window with XCreateWindow.
  Set the colourmap of the window to the created colormap with XSetWindowColormap.
  Get the colourmap associated with the window with XGetWindowAttributes.
  Verify that the created colourmap and that associated with the window are the same.
>>CODE
XVisualInfo *vi;
XWindowAttributes watts;

	for(resetvinf(VI_WIN); nextvinf(&vi); ) {

		colormap = makecolmap(display, vi->visual, AllocNone);
		w = makewin(display, vi);
		XCALL;
		if( XGetWindowAttributes(display, w, &watts) == False ) {
			delete("XGetWindowAttributes() failed on visual class %s", displayclassname(vi->class));
			return;
		} else
			CHECK;

		if( watts.colormap != colormap ) {
			report("Colormap of window was not set for visual class %s", displayclassname(vi->class));
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Bad C
If more than one type of visual is supported:
When the
.A colormap
argument
does not have the same visual type as the window
specified by the window argument
.A w ,
then a
.S BadMatch 
error occurs.
>>STRATEGY
For each supported visual:
  Create a colormap not of that visual type.
  Create a window of that visual type.
  Verify that a call to XSetWindowColormap generates a BadMatch error.
>>CODE BadMatch
XVisualInfo *vi;
XWindowAttributes watts;
int i, nvisuals;
Visual **visuals;

	resetvinf(VI_WIN);
	if((nvisuals = nvinf()) <= 1) {
		unsupported("Only one visual type is supported");
		return;
	}

	visuals = (Visual **)malloc(nvisuals * sizeof(Visual *));
	if (visuals == 0) {
		delete("Could not allocate visuals");
		return;
	}
	for(resetvinf(VI_WIN), i=0 ; nextvinf(&vi); i++) {
		visuals[i] = vi->visual;
	}

	for(resetvinf(VI_WIN), i = 1; nextvinf(&vi); i++) {
		i = i % nvisuals;
		w = makewin(display, vi);
		colormap = makecolmap(display, visuals[i], AllocNone);
		XCALL;
		if(geterr() == BadMatch)
			CHECK;
	}

	CHECKPASS(nvinf());

>>ASSERTION Bad A
.ER BadColor
>>ASSERTION Bad A
.ER BadWindow
>>#HISTORY	Cal	Completed	Written in new format and style -4/12/90.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writting code.
