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
 * $XConsortium: frpxmp.m,v 1.5 92/06/11 16:16:16 rws Exp $
 */
>>TITLE XFreePixmap CH05
void
XFreePixmap(display, pixmap)
Display *display = Dsp;
Pixmap  pixmap;
>>ASSERTION Good A
A call to xname removes the association between the pixmap ID
.A pixmap
and the specified pixmap.
>>STRATEGY
For all supported depths of pixmap:
   Create a pixmap.
   Create a gc using the pixmap as the drawable.
   Free the pixmap with XFreePixmap.
   Plot (0,0) in the pixmap.
   Verify that a BadDrawable error occurred.
>>CODE
XGCValues	gcv;
XVisualInfo	*vp;
GC		gc;

	for(resetvinf(VI_PIX); nextvinf(&vp); ) {
		pixmap = XCreatePixmap(display, DRW(display), 1, 1, vp->depth);
		gc = makegc(display, pixmap);
		XCALL;
		
		startcall(Dsp);
		XDrawPoint(display, pixmap, gc, 0, 0);
		endcall(Dsp);
		if(geterr() != BadDrawable) {
			report("Got %s instead of BadDrawable when drawing on a freed pixmap. ", errorname(geterr()));
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
The storage allocated to the pixmap is not recovered until all references to it
have been removed.
>>STRATEGY
Create a window.
Create a pixmap of the same dimensions as the window.
Pattern the pixmap.
Create a gc with the pixmap as the tile and the fill_mode set to FillTiled.
Free the pixmap with XFreePixmap.
Tile the entire window with XFillRectangle.
Verify that the tiled pattern matches the pixmap.
>>CODE
Window		win;
XVisualInfo	*vp;
XGCValues	gcv;
GC		gc, gc2;

	for(resetvinf(VI_WIN); nextvinf(&vp);) {
		win = makewin(display, vp);	
		pixmap = XCreatePixmap(display, DRW(display), W_STDWIDTH, W_STDHEIGHT, vp->depth);
		dset(display, pixmap, W_BG);
		pattern(display, pixmap);
	
		gcv.fill_style = FillTiled;
		gcv.tile = pixmap;
		gcv.foreground = W_FG;
		gcv.background = W_BG;

                /*
                 * Create the GC with the window of the same depth because
                 * the root window could be of a different depth.
                 */
		gc = XCreateGC(display, win, GCFillStyle|GCTile|GCForeground|GCBackground, &gcv);
		XCALL;
	
		XFillRectangle(display, win, gc, 0, 0, W_STDWIDTH+1, W_STDHEIGHT+1);		
		
		if( checkpattern(display, win, (struct area *) 0 ) != True) {
			report("Tiled pattern on window was not correct after");
			report("tile component in GC was freed by XFreePixmap");
			FAIL;
		} else
			CHECK;
	
	}
	CHECKPASS(nvinf());

>>ASSERTION Bad A	
.ER BadPixmap
>>#HISTORY	Cal	Completed	Written in new format and style.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writing code.
