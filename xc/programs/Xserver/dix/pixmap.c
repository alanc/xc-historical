/* $XConsortium$ */
/*

Copyright 1993 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#include "X.h"
#include "scrnintstr.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "servermd.h"
#include "site.h"


/*
 *  Scratch pixmap management and device independent pixmap allocation
 *  function.
 */


/* callable by ddx */
PixmapPtr
GetScratchPixmapHeader(pScreen, width, height, depth, bitsPerPixel, devKind,
		       pPixData)
    ScreenPtr   pScreen;
    int		width;
    int		height;
    int		depth;
    int		bitsPerPixel;
    int		devKind;
    pointer     pPixData;
{
    PixmapPtr pPixmap = pScreen->pScratchPixmap;

    if (pPixmap)
	pScreen->pScratchPixmap = NULL;
    else
	/* width and height of 0 means don't allocate any pixmap data */
	pPixmap = (*pScreen->CreatePixmap)(pScreen, 0, 0, depth);

    if (pPixmap)
	if ((*pScreen->ModifyPixmapHeader)(pPixmap, width, height, depth,
					   bitsPerPixel, devKind, pPixData))
	    return pPixmap;
    return NullPixmap;
}


/* callable by ddx */
void
FreeScratchPixmapHeader(pPixmap)
    PixmapPtr pPixmap;
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;

    if (pPixmap)
    {
	if (pScreen->pScratchPixmap)
	    (*pScreen->DestroyPixmap)(pPixmap);
	else
	    pScreen->pScratchPixmap = pPixmap;
    }
}


Bool
CreateScratchPixmapsForScreen(scrnum)
    int scrnum;
{
    /* let it be created on first use */
    screenInfo.screens[scrnum]->pScratchPixmap = NULL;
    return TRUE;
}


void
FreeScratchPixmapsForScreen(scrnum)
    int scrnum;
{
    FreeScratchPixmapHeader(screenInfo.screens[scrnum]->pScratchPixmap);
}


/* callable by ddx */
PixmapPtr
AllocatePixmap(pScreen, pixDataSize)
    ScreenPtr pScreen;
    int pixDataSize;
{
    PixmapPtr pPixmap;
#ifdef PIXPRIV
    char *ptr;
    DevUnion *ppriv;
    unsigned *sizes;
    unsigned size;
    int i;

    pPixmap = (PixmapPtr)xalloc(pScreen->totalPixmapSize + pixDataSize);
    if (!pPixmap)
	return NullPixmap;
    ppriv = (DevUnion *)(pPixmap + 1);
    pPixmap->devPrivates = ppriv;
    sizes = pScreen->PixmapPrivateSizes;
    ptr = (char *)(ppriv + pScreen->PixmapPrivateLen);
    for (i = pScreen->PixmapPrivateLen; --i >= 0; ppriv++, sizes++)
    {
        if (size = *sizes)
        {
	    ppriv->ptr = (pointer)ptr;
	    ptr += size;
        }
        else
	    ppriv->ptr = (pointer)NULL;
    }
#else
    pPixmap = (PixmapPtr)xalloc(sizeof(PixmapRec) + pixDataSize);
#endif
    return pPixmap;
}
