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
 * Functions to cycle through all the visuals that are supported
 * on the screen under test.
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	"pixval.h"
#include	<string.h>

Display	*Dsp;

static	int 	Visindex;
static	int 	Nvis;
static	XVisualInfo *Vinfop;

static	int 	Depthind;
static	int 	*Depths;
static	int 	Ndepths;

int 	CurVinf;

static	void	iddebug();

/*
 * Start again at the beginning of the list of visual classes.
 *
 * The callee must ensure that the alternate screen is supported if
 * VI_ALT_PIX or VI_ALT_WIN are used.
 */
void
resetvinf(flags)
int 	flags;
{
XVisualInfo	vi;

	if (flags == 0)
		flags = VI_WIN_PIX;

	if ((flags & (VI_WIN_PIX|VI_ALT_WIN_PIX)  == 0 ) ||
	    ((flags & ~(VI_WIN_PIX|VI_ALT_WIN_PIX) ) != 0)) {
		printf("Programming error detected in resetvinf\n");
		exit(1);
	}

	if (((flags & VI_WIN_PIX) != 0) && ((flags & VI_ALT_WIN_PIX) != 0)) {
		printf("Illegal flag combination detected in resetvinf\n");
		exit(1);
	}

	CurVinf = 1;

	/*
	 * Do pixmaps only.  Useful for debuging maybe.  Allows you to test
	 * some aspects of a X Terminal that is being used.
	 */
	if (config.debug_pixmap_only)
		flags &= ~(VI_WIN|VI_ALT_WIN);

	/*
	 * Do not test pixmaps.  Useful during testing maybe.
	 */
	if (config.debug_window_only)
		flags &= ~(VI_PIX|VI_ALT_PIX);

	Visindex = 0;
	if (Vinfop)
		XFree((char*)Vinfop);
	Vinfop = (XVisualInfo *)0;

	Depthind = 0;
	if (Depths)
		XFree((char*)Depths);
	Depths = (int *)0;

	Nvis = 0;
	Ndepths = 0;

	if (flags & (VI_WIN|VI_ALT_WIN)) {
		vi.screen = (flags & VI_WIN)?
				DefaultScreen(Dsp) : config.alt_screen;
		Vinfop = XGetVisualInfo(Dsp, VisualScreenMask, &vi, &Nvis);

		/*
		 * For debuging purposes only consider a subset of the available
		 * visuals.
		 */
		if (config.debug_visual_ids)
			iddebug();

		if (Nvis == 0) {
			/*
			 * Report something to make sure that a problem is noted.
			 */
			delete("No visuals found");
		}
	}

	if (flags & (VI_PIX|VI_ALT_PIX)) {
		Depths = XListDepths(Dsp,
			(flags & VI_PIX)?
				DefaultScreen(Dsp) : config.alt_screen,
			&Ndepths);
		if (Depths == (int*)0) {
			delete("Call to XListDepths failed");
		}
		if (Ndepths < 1) {
			delete("less than 1 depth found in XListDepths");
		}
	}

	if (config.debug_default_depths) {
		if (Nvis > 1)
			Nvis = 1;
		if (Ndepths > 1)
			Ndepths = 1;
	}

}

/*
 * Get the next visual info structure.
 * This will either corespond to a visual supported by the default
 * screen, or (if visual is NULL) to a depth for which pixmaps are supported.
 * Returns False if there is one, otherwise True.
 */
int
nextvinf(visp)
XVisualInfo 	**visp;
{
static	XVisualInfo	vi;

	CurVinf++;

	/*
	 * This will cycle through the visuals first, then the pixmap depths.
	 */
	if (Vinfop) {
		if (Visindex < Nvis) {
			*visp = &Vinfop[Visindex++];
			trace("--- Running test with visual class %s, depth %d",
			    displayclassname((*visp)->class), (*visp)->depth);
			return(True);
		} else {
			XFree((char*)Vinfop);
			Vinfop = (XVisualInfo *)0;
		}
	}
	if (Depths) {
		if (Depthind < Ndepths) {
			vi.depth = Depths[Depthind++];
			vi.visual = 0;
			*visp = &vi;
			trace("--- Running test with pixmap depth %d", vi.depth);
			return(True);
		} else {
			XFree((char*)Depths);
			Depths = (int*)0;
		}
	}

	return(False);
}

/*
 * Returns the number of times that nextvinf will succeed. Only valid
 * after a call to resetvinf().
 */
int
nvinf()
{
	return(Nvis+Ndepths);
}

/*
 * Return a drawable which is either a window with a default visual
 * or a pixmap with a default depth.  This is used where it not
 * desired to loop over all types and depths.
 */
Drawable
defdraw(disp, type)
Display *disp;
int 	type;
{
XVisualInfo	*vp;
Drawable	d;

	resetvinf(type);
	nextvinf(&vp);

	d = makewin(disp, vp);

	return(d);
}

/*
 * Create a window with a default visual type.  This is only used
 * where it is not neccessary to loop over all types.
 */
Window
defwin(disp)
Display	*disp;
{
XVisualInfo	*vp;

	resetvinf(VI_WIN);
	nextvinf(&vp);

	return makewin(disp, vp);
}

/*
 * If we only want to consider a subset of visuals then this routine
 * is called to filter out all the irrelevent ones.
 */
static void
iddebug()
{
XVisualInfo	*viptmp;
char	*idlist;
int 	id;
int 	i;
int 	nreal = Nvis;

	/*
	 * Copy the original (this is realy only being done this way
	 * to avoid mixing free and XFree.)
	 */
	viptmp = (XVisualInfo*)malloc(nreal * sizeof(XVisualInfo));
	if (viptmp == 0)
		return;
	for (i = 0; i < nreal; i++)
		viptmp[i] = Vinfop[i];

	/*
	 * Copy back any visual that is wanted, keeping count of found ones
	 * as we go along.  Note that the list may contain visualids for
	 * different screens.
	 */
	Nvis = 0;
	idlist = config.debug_visual_ids;
	while (idlist) {
		id = atov(idlist); /* Allow hex/octal/decimal values */
		for (i = 0; i < nreal; i++) {
			if (viptmp[i].visualid == id)
				Vinfop[Nvis++] = viptmp[i];
		}

		idlist = strchr(idlist, ',');
		if (idlist)
			idlist++;
	}
	free(viptmp);
}
