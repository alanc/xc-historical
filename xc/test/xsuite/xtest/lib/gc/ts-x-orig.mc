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
/*
 * The symbols stipple_* and tile_* that do not occur in this file
 * are defined in fill-style.mc which is always included along with
 * this one.
 */
>>ASSERTION Good A
The tile/stipple origin coordinates
.M ts-x-origin
and
.M ts-y-origin
are interpreted relative to the
origin of the destination drawable specified in the graphics
operation.
>>STRATEGY
Create a stipple pixmap.
Set stipple component of GC to pixmap using XSetStipple.
For various tile/stipple origins:
	Set tile/stipple origin using XSetTSOrigin.
	Set fill-style to FillStippled using XSetFillStyle.
	Clear drawable.
	Do graphics operation.
	Pixmap verify.
Create a tile with depth of drawable.
Set tile component of GC to tile using XSetTile.
For various tile/stipple origins:
	Set tile/stipple origin using XSetTSOrigin.
	Set fill-style to FillTiled using XSetFillStyle.
	Clear drawable.
	Do graphics operation.
	Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	tsostip;
Pixmap	tsotile;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		tsostip = XCreateBitmapFromData(A_DISPLAY, A_DRAWABLE
			, (char*)stipple_bits, stipple_width, stipple_height
			);
		XSetStipple(A_DISPLAY, A_GC, tsostip);
		XFreePixmap(A_DISPLAY, tsostip);
		XSetFillStyle(A_DISPLAY, A_GC, FillStippled);

		XSetTSOrigin(A_DISPLAY, A_GC, 9, 17);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XSetTSOrigin(A_DISPLAY, A_GC, 0xd8f2, 0x4321);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XSetTSOrigin(A_DISPLAY, A_GC, -3, 7);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		/* Now Tiles */
		tsotile = XCreatePixmapFromBitmapData(A_DISPLAY, A_DRAWABLE
			, (char*)tile_bits, tile_width, tile_height
			, W_FG, W_BG
			, vp->depth
			);
		XSetTile(A_DISPLAY, A_GC, tsotile);
		XFreePixmap(A_DISPLAY, tsotile);
		XSetFillStyle(A_DISPLAY, A_GC, FillTiled);

		XSetTSOrigin(A_DISPLAY, A_GC, 9, 17);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XSetTSOrigin(A_DISPLAY, A_GC, 0xd8f2, 0x4321);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XSetTSOrigin(A_DISPLAY, A_GC, -3, 7);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
	}

	CHECKPASS(6*nvinf());
