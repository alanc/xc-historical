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

#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

static int Nsupvis;
static unsigned long Supvismsk;

/*
 * issuppvis() takes a visual class as argument and returns true
 * if such a class is supported.
 * This function uses the XGetVisualInfo() function rather than
 * use the XT_VISUAL_CLASSES parameter.
 */
int
issuppvis(disp, vis)
Display	*disp;
int 	vis;
{
XVisualInfo	templ;
XVisualInfo	*vip;
int 	n;

	templ.class = vis;
	templ.screen = DefaultScreen(disp);
	vip = XGetVisualInfo(disp, VisualClassMask|VisualScreenMask, &templ, &n);
	if(vip != (XVisualInfo *) 0)
		XFree((char*)vip);
	return((n>0)? True: False);
}

/*
 * Takes a mask indicating a set of visuals, and returns
 * a mask indicating the subset that is supported.
 * If the mask is 0L then the mask shows all supported 
 * visuals.
 */
unsigned long
visualsupported(disp, mask)      
Display *disp;
unsigned long mask;
{
	unsigned long resultmask = 0;

	if(mask == 0L)
		mask =  ((1L<<DirectColor) | (1L<<PseudoColor) | (1L<<GrayScale)|
			 (1L<<TrueColor) | (1L<<StaticColor) | (1L<<StaticGray));

	if(mask & (1L<<PseudoColor))
		if(issuppvis(disp, PseudoColor))
			resultmask |= (1L<<PseudoColor);

	if(mask & (1L<<DirectColor))
		if(issuppvis(disp, DirectColor))
			resultmask |= (1L<<DirectColor);

	if(mask & (1L<<GrayScale))
		if(issuppvis(disp, GrayScale))
			resultmask |= (1L<<GrayScale);

	if(mask & (1L<<StaticGray))
		if(issuppvis(disp, StaticGray))
			resultmask |= (1L<<StaticGray);

	if(mask & (1L<<StaticColor))
		if(issuppvis(disp, StaticColor))
			resultmask |= (1L<<StaticColor);

	if(mask & (1L<<TrueColor))
		if(issuppvis(disp, TrueColor))
			resultmask |= (1L<<TrueColor);

	return resultmask;
}

void
resetsupvis(vismask)
unsigned long vismask;
{
	resetvinf(VI_WIN);
	Nsupvis = 0;
	Supvismsk = vismask;
}


int
nextsupvis(vi)
XVisualInfo **vi;
{
	while(nextvinf(vi))
		if(Supvismsk & (1L<<(*vi)->class)) {
			Nsupvis++;
			trace("--- Testing with supported visual class %s", displayclassname((*vi)->class));
			return True;
		}

	return False;
}

int
nsupvis()
{
	return Nsupvis;
}
