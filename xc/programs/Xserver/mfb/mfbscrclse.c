/* $XConsortium: mfbscrclse.c,v 1.3 89/03/18 12:25:32 rws Exp $ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
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

#include "scrnintstr.h"

void
mfbCloseScreen(index, pScreen)
    register ScreenPtr pScreen;
{

    if (pScreen->allowedDepths)
    {
	if (pScreen->allowedDepths->vids)
		xfree(pScreen->allowedDepths->vids);
 
	xfree(pScreen->allowedDepths);
    }

    /*  pScreen->visuals does not need to be freed here, since it is added as
	a resource in "mfbScreenInit" and is freed with the rest of the 
	resources. */

    if(pScreen->devPrivate)
	xfree(pScreen->devPrivate);

    return;
}

