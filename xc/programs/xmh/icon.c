#ifndef lint
static char rcs_id[] = "$Header: icon.c,v 1.2 87/07/31 09:35:48 weissman Exp $";
#endif lint
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

/* Icon.c - Handle icon pixmaps. */

#include "xmh.h"
#include "nomail.bit"
#include "newmail.bit"

#ifdef X11
static Pixmap MakePixmap(dpy, root, data, width, height) 
Display *dpy;
Drawable root;
short *data;
unsigned int width, height;
{
    XImage ximage;
    GC pgc;
    XGCValues gcv;
    Pixmap pid;

    pid = XCreatePixmap(dpy, root, width, height,
			(unsigned int) DefaultDepth(dpy, 0));
    gcv.foreground = BlackPixel(dpy, 0);
    gcv.background = WhitePixel(dpy, 0);
    pgc = XCreateGC(dpy, pid, GCForeground | GCBackground, &gcv);
    ximage.height = height;
    ximage.width = width;
    ximage.xoffset = 0;
    ximage.format = XYBitmap;
    ximage.data = (char *)data;
    ximage.byte_order = LSBFirst;
    ximage.bitmap_unit = 16; 
    ximage.bitmap_bit_order = LSBFirst;
    ximage.bitmap_pad = 16;
    ximage.bytes_per_line = (width+15)/16 * 2;
    ximage.depth = 1;

    XPutImage(dpy, pid, pgc, &ximage, 0, 0, 0, 0, width, height);
    XFreeGC(dpy, pgc);
    return(pid);
}
#endif


void IconInit()
{
#ifdef X11
    NoMailPixmap = MakePixmap(theDisplay, DefaultRootWindow(theDisplay),
			      nomail_bits, nomail_width, nomail_height);
    NewMailPixmap =MakePixmap(theDisplay, DefaultRootWindow(theDisplay),
			      newmail_bits, newmail_width, newmail_height);
#endif
}
