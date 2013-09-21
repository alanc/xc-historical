/* $XConsortium: ppcPntWin.c,v 1.3 94/10/12 21:06:18 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga16/ibm/ppcPntWin.c,v 3.0 1994/05/04 15:03:30 dawes Exp $ */
/*
 * Copyright IBM Corporation 1987,1988,1989
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that 
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/


#define BANKING_MODS
#include "X.h"

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ppc.h"

#include "ibmTrace.h"
#include "OScompiler.h"	/* GJA */

/* NOTE: These functions only work for visuals up to 31-bits deep */
void ppcPaintWindowSolid();
void ppcPaintWindowTile();

void
ppcPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{

    register mfbPrivWin	*pPrivWin;
    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);

    TRACE(("ppcPaintWindow( pWin= 0x%x, pRegion= 0x%x, what= %d )\n",
							pWin,pRegion,what));

    switch (what) {
    case PW_BACKGROUND:
	switch (pWin->backgroundState) {
	case None:
	    return;
	case ParentRelative:
	    do {
		pWin = pWin->parent;
	    } while (pWin->backgroundState == ParentRelative);
	    (*pWin->drawable.pScreen->PaintWindowBackground)(pWin, pRegion,
							     what);
	    return;
	case BackgroundPixmap:
	    if (pPrivWin->fastBackground)
	    {
		ppcPaintWindowTile(pWin, pRegion, what);
		return;
	    }
	    break;
	case BackgroundPixel:
	    ppcPaintWindowSolid(pWin, pRegion, what);
	    return;
    	}
    	break;
    case PW_BORDER:
	if (pWin->borderIsPixel)
	{
	    ppcPaintWindowSolid(pWin, pRegion, what);
	    return;
	}
	else if (pPrivWin->fastBorder)
	{
            ppcPaintWindowTile(pWin, pRegion, what);
	    return;
	}
	break;
    }
    miPaintWindow(pWin, pRegion, what);
}

void
ppcPaintWindowSolid(pWin, pRegion, what)
    register WindowPtr pWin;
    register RegionPtr pRegion;
    int what;		
{
    register int nbox;
    register BoxPtr pbox;
    register unsigned long int pixel;
    register unsigned long int pm ;

    TRACE(("ppcPaintWindowSolid(pWin= 0x%x, pRegion= 0x%x, what= %d)\n", pWin, pRegion, what));

    if ( !( nbox = REGION_NUM_RECTS(pRegion)))
	return ;
    pbox = REGION_RECTS(pRegion);

    if (what == PW_BACKGROUND)
	pixel = pWin->background.pixel;
    else
	pixel = pWin->border.pixel; 

    pm = ( 1 << pWin->drawable.depth ) - 1 ;
    for ( ; nbox-- ; pbox++ ) {
	/*
	 * call fill routine, the parms are:
	 * 	fill(color, alu, planes, x, y, width, height);
	 */
	vgaFillSolid( pWin, pixel, GXcopy, pm, pbox->x1, pbox->y1, 
		pbox->x2 - pbox->x1, pbox->y2 - pbox->y1 ) ; 
    }
    return ;
}

void
ppcPaintWindowTile(pWin, pRegion, what)
    register WindowPtr pWin;
    register RegionPtr pRegion;
    int what;		
{
    register int nbox;
    register BoxPtr pbox;
    register PixmapPtr pTile;
    register unsigned long int pm ;

    TRACE(("ppcPaintWindowTile(pWin= 0x%x, pRegion= 0x%x, what= %d)\n", pWin, pRegion, what));

    if ( !( nbox = REGION_NUM_RECTS(pRegion)))
	return ;
    pbox = REGION_RECTS(pRegion);

    if (what == PW_BACKGROUND)
	pTile = pWin->background.pixmap;
    else
	pTile = pWin->border.pixmap;

    pm = ( 1 << pWin->drawable.depth ) - 1 ;
    for ( ; nbox-- ; pbox++ ) {
	/*
	 * call tile routine, the parms are:
	 * 	tile(tile, alu, planes, x, y, width, height,xSrc,ySrc);
	 */
	ppcTileRect(pWin, pTile, GXcopy, pm, 
		pbox->x1, pbox->y1, 
		pbox->x2 - pbox->x1, pbox->y2 - pbox->y1,
		pWin->drawable.x, pWin->drawable.y );
    }
    return ;
}
