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

#include	"stdio.h"
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

extern	Display	*Dsp;

/*
 * Open the xtest fonts, and place their id's in the fonts array.
 * The nfonts argument limits the number of fonts that will be placed in
 * the array.
 */
void
openfonts(fonts, nfonts)
Font	fonts[];
int 	nfonts;
{
char	name[64];
int 	i;

	if (XT_NFONTS < nfonts)
		nfonts = XT_NFONTS;

	resetdelete();
	for (i = 0; i < nfonts; i++) {
		sprintf(name, "xtfont%d", i);

		fonts[i] = XLoadFont(Dsp, name);
		if (isdeleted()) {
			report("Could not open %s in openfonts", name);
			report("Check that xtest fonts are installed properly");
			cancelrest("Could not open all xtest fonts");
			return;
		}
	}
}

