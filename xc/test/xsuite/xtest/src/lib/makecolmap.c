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
#include	"stdlib.h"
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Make a colourmap for the screen associated with the Default
 * root window.
 */

Colormap
makecolmap(disp, visual, alloc)
Display *disp;
Visual *visual;
int alloc;
{
	Colormap colormap;

	colormap = XCreateColormap(disp, DRW(disp), visual, alloc);
	regid(disp, (union regtypes *) &colormap, REG_COLORMAP);
	return(colormap);
}


/*
 * return floor of log to base two.
 * if x is zero returns -1 ( lg(x) != -1 for any integer x)
 */
int lg(x)
	unsigned long x;
{
	int count = -1;

	for(; x != 0; x>>=1, count++);
	return count;
}

/*
 * return a sting representing the argument value as a binary number.
 */
char *bitstr(val, spare)
	unsigned long val;
	char spare[];
{
	int i;

	for(i=0; i< sizeof(unsigned long)<<3; i++,val <<= 1)
		if(val & ((unsigned long)1<<((sizeof(unsigned long)<<3) -1)))
			spare[i] = '1';
		else
			spare[i] = '0';
	spare[i] = (char)0;
	return(spare);
}
