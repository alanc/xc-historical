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
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Dump out an image to the given file name.  A ascii form is used
 * with a simple run length encoding.
 */
/*ARGSUSED2*/
void
dumpimage(imp, name, ap)
XImage	*imp;
char	*name;
struct	area	*ap;	/* NOTUSED yet */
{
FILE	*fp;
int 	x, y;
unsigned long	pix, lastpix = 0L; /* Initialise just to keep lint quiet */
unsigned long 	count;
extern	int 	tet_thistest;
static	int 	lasttest;

	fp = fopen(name, (lasttest==tet_thistest)? "a": "w");
	if (fp == NULL) {
		report("Could not create image file %s", name);
		return;
	}
	lasttest = tet_thistest;

	fprintf(fp, "%d %d %d\n", imp->width, imp->height, imp->depth);

	count = 0;
	for (y = 0; y < imp->height; y++) {
		for (x = 0; x < imp->width; x++) {
			pix = XGetPixel(imp, x, y);
			/* count is 0 on the first time through */
			if (pix != lastpix || count == 0) {
				if (count == 1) {
					fprintf(fp, "%x\n", lastpix);
				} else if (count != 0) {
					fprintf(fp, "%x,%x\n", count, lastpix);
				}
				lastpix = pix;
				count = 1;
			} else {
				count++;
			}
		}
	}
	if (count == 1) {
		fprintf(fp, "%x\n", lastpix);
	} else if (count != 0) {
		fprintf(fp, "%x,%x\n", count, lastpix);
	}
	fclose(fp);
}

