/* $XConsortium: ppcBStore.c,v 1.1 94/03/28 21:34:47 dpw Exp $ */
/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


Copyright IBM Corporation 1987,1988,1989
All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that 
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.


Copyright 1987 by the Regents of the University of California
All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.


Copyright IBM Corporation 1987
All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/

/* Header: /andrew/X11/r3src/release/server/ddx/ibm/ppc/RCS/ppcBStore.c,v 30.0 89/01/23 19:29:01 paul Exp */

#include "X.h"
#include "servermd.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"

#include "mibstore.h"
#include "mfb.h"

#include "ppc.h"

#include "ibmTrace.h"

/*-----------------------------------------------------------------------
 * ppcSaveAreas --
 *	Function called by miSaveAreas to actually fetch the areas to be
 *	saved into the backing pixmap.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the screen into the pixmap.
 *
 *-----------------------------------------------------------------------
 */
void
ppcSaveAreas( pPixmap, prgnSave, xorg, yorg, pWin )
    register PixmapPtr pPixmap ; /* Backing pixmap */
    RegionPtr prgnSave ;	/* Region to save (pixmap-relative) */
    int xorg ;			/* X origin of region */
    int yorg ;			/* Y origin of region */
    WindowPtr pWin;
{
    register BoxPtr pBox ;
    register int nBox ;

    TRACE( ( "ppcSaveAreas(0x%x,0x%x,%d,%d)\n",
	   pPixmap, prgnSave, xorg, yorg ) ) ;
/* WHOOP WHOOP WHOOP XXX -- depth 8 *only* */ /* GJA -- ? */

    if ( !( nBox = REGION_NUM_RECTS(prgnSave) ) )
	return ;

    for ( pBox = REGION_RECTS(prgnSave) ; nBox-- ; pBox++ )
	vgaReadColorImage( pBox->x1 + xorg,
		 pBox->y1 + yorg,
		 pBox->x2 - pBox->x1,
		 pBox->y2 - pBox->y1,
		 ( (unsigned char *) pPixmap->devPrivate.ptr )
		 + ( pBox->y1 * pPixmap->devKind ) + pBox->x1,
		 pPixmap->devKind ) ;

    return ;
}

/*-----------------------------------------------------------------------
 * ppcRestoreAreas --
 *	Function called by miRestoreAreas to actually fetch the areas to be
 *	restored from the backing pixmap.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the pixmap into the screen.
 *
 *-----------------------------------------------------------------------
 */
void
ppcRestoreAreas( pPixmap, prgnRestore, xorg, yorg, pWin )
    register PixmapPtr pPixmap ; /* Backing pixmap */
    RegionPtr prgnRestore ;	/* Region to restore (screen-relative)*/
    int xorg ;			/* X origin of window */
    int yorg ;			/* Y origin of window */
    WindowPtr pWin;
{
    register BoxPtr pBox ;
    register int nBox ;

    TRACE( ( "ppcRestoreAreas(0x%x,0x%x,%d,%d)\n",
	   pPixmap, prgnRestore, xorg, yorg ) ) ;
/* WHOOP WHOOP WHOOP XXX -- depth 8 *only* */

    if ( !( nBox = REGION_NUM_RECTS(prgnRestore) ) )
	return ;
    for ( pBox = REGION_RECTS(prgnRestore) ; nBox-- ; pBox++ )
	vgaDrawColorImage( pBox->x1,
		 pBox->y1,
		 pBox->x2 - pBox->x1,
		 pBox->y2 - pBox->y1,
		 ( (char *)pPixmap->devPrivate.ptr )
		 + ( ( pBox->y1 - yorg ) * pPixmap->devKind )
		 + ( pBox->x1 - xorg ),
		 pPixmap->devKind,
		 GXcopy, ~0 ) ;
    return ;
}
