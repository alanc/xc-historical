/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "cfbmskbits.h"
#include "mibstore.h"

extern int defaultColorVisualClass;

#define _BP 8
#define _RZ ((PSZ + 2) / 3)
#define _RS 0
#define _RM ((1 << _RZ) - 1)
#define _GZ ((PSZ - _RZ + 1) / 2)
#define _GS _RZ
#define _GM (((1 << _GZ) - 1) << _GS)
#define _BZ (PSZ - _RZ - _GZ)
#define _BS (_RZ + _GZ)
#define _BM (((1 << _BZ) - 1) << _BS)
#define _CE (1 << _RZ)

static VisualRec visuals[] = {
/* vid screen class rMask gMask bMask oRed oGreen oBlue bpRGB cmpE nplan */
#ifndef STATIC_COLOR
    0,  0, PseudoColor, 0,   0,   0,   0,   0,   0,   _BP,  1<<PSZ,   PSZ,
    0,  0, DirectColor, _RM, _GM, _BM, _RS, _GS, _BS, _BP, _CE,       PSZ,
    0,  0, GrayScale,   0,   0,   0,   0,   0,   0,   _BP,  1<<PSZ,   PSZ,
    0,  0, StaticGray,  0,   0,   0,   0,   0,   0,   _BP,  1<<PSZ,   PSZ,
#endif
    0,  0, StaticColor, _RM, _GM, _BM, _RS, _GS, _BS, _BP,  1<<PSZ,   PSZ,
    0,  0, TrueColor,   _RM, _GM, _BM, _RS, _GS, _BS, _BP, _CE,       PSZ
};

#define	NUMVISUALS	((sizeof visuals)/(sizeof visuals[0]))

static DepthRec depths[] = {
/* depth	numVid		vids */
    1,		0,		NULL,
    8,		NUMVISUALS,	NULL
};

#define NUMDEPTHS	((sizeof depths)/(sizeof depths[0]))

int cfbWindowPrivateIndex = -1;
int cfbGCPrivateIndex = -1;

miBSFuncRec cfbBSFuncRec = {
    cfbSaveAreas,
    cfbRestoreAreas,
    (void (*)()) 0,
    (PixmapPtr (*)()) 0,
    (PixmapPtr (*)()) 0,
};

/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
cfbScreenInit(index, pScreen, pbits, xsize, ysize, dpi)
    int index;
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpi;			/* dots per inch */
{
    VisualID	*pVids;
    register PixmapPtr pPixmap;
    int	i, j;
    ColormapPtr cmap;

    pScreen->myNum = index;
    pScreen->width = xsize;
    pScreen->height = ysize;
    pScreen->mmWidth = (xsize * 254) / (dpi * 10);
    pScreen->mmHeight = (ysize * 254) / (dpi * 10);
    pScreen->numDepths = NUMDEPTHS;
    pScreen->allowedDepths = depths;

    pScreen->rootDepth = 8;
    pScreen->minInstalledCmaps = 1;
    pScreen->maxInstalledCmaps = 1;
    pScreen->backingStoreSupport = Always;
    pScreen->saveUnderSupport = NotUseful;

    /* cursmin and cursmax are device specific */

    pScreen->numVisuals = NUMVISUALS;
    pScreen->visuals = visuals;

    pPixmap = (PixmapPtr ) xalloc(sizeof(PixmapRec));
    if (!pPixmap)
	return FALSE;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.depth = 8;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.serialNumber = 0;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = xsize;
    pPixmap->drawable.height = ysize;
    pPixmap->refcnt = 1;
    pPixmap->devPrivate.ptr = pbits;
    pPixmap->devKind = PixmapBytePad(xsize, 8);
    pScreen->devPrivate = (pointer)pPixmap;

    /* anything that cfb doesn't know about is assumed to be done
       elsewhere.  (we put in no-op only for things that we KNOW
       are really no-op.
    */
    pScreen->CreateWindow = cfbCreateWindow;
    pScreen->DestroyWindow = cfbDestroyWindow;
    pScreen->PositionWindow = cfbPositionWindow;
    pScreen->ChangeWindowAttributes = cfbChangeWindowAttributes;
    pScreen->RealizeWindow = cfbMapWindow;
    pScreen->UnrealizeWindow = cfbUnmapWindow;

    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
    pScreen->QueryBestSize = mfbQueryBestSize;
    pScreen->GetImage = miGetImage;
    pScreen->GetSpans = cfbGetSpans;	/* XXX */
    pScreen->CreateGC = cfbCreateGC;
    pScreen->CreatePixmap = cfbCreatePixmap;
    pScreen->DestroyPixmap = cfbDestroyPixmap;
    pScreen->ValidateTree = miValidateTree;

#ifdef	STATIC_COLOR
    pScreen->InstallColormap = cfbInstallColormap;
    pScreen->UninstallColormap = cfbUninstallColormap;
    pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
    pScreen->StoreColors = NoopDDA;
#endif
    pScreen->ResolveColor = cfbResolveColor;

    pScreen->RegionCreate = miRegionCreate;
    pScreen->RegionCopy = miRegionCopy;
    pScreen->RegionDestroy = miRegionDestroy;
    pScreen->Intersect = miIntersect;
    pScreen->Inverse = miInverse;
    pScreen->Union = miUnion;
    pScreen->Subtract = miSubtract;
    pScreen->RegionReset = miRegionReset;
    pScreen->TranslateRegion = miTranslateRegion;
    pScreen->RectIn = miRectIn;
    pScreen->PointInRegion = miPointInRegion;
    pScreen->WindowExposures = miWindowExposures;
    pScreen->RegionNotEmpty = miRegionNotEmpty;
    pScreen->RegionEmpty = miRegionEmpty;
    pScreen->RegionExtents = miRegionExtents;
    pScreen->SendGraphicsExpose = miSendGraphicsExpose;

    pScreen->BlockHandler = NoopDDA;
    pScreen->WakeupHandler = NoopDDA;
    pScreen->blockData = (pointer)0;
    pScreen->wakeupData = (pointer)0;

    pScreen->CreateColormap = cfbInitializeColormap;
    pScreen->DestroyColormap = NoopDDA;

    /*  Set up the remaining fields in the visuals[] array & make a RT_VISUALID */
    for (i = 0; i < NUMVISUALS; i++) {
	visuals[i].vid = FakeClientID(0);
	visuals[i].screen = index;
	if (!AddResource(visuals[i].vid, RT_VISUALID, (pointer)&visuals[i],
			 (int(*)())NoopDDA, RC_CORE))
	{
	    while (--i >= 0)
		FreeResource(visuals[i].vid, RC_NONE);
	    xfree(pPixmap);
	    return FALSE;
	}
    }

    /*  Set up the remaining fields in the depths[] array */
    for (i = 0; i < NUMDEPTHS; i++) {
	if (depths[i].numVids > 0) {
	    depths[i].vids = pVids = (VisualID *) xalloc(sizeof (VisualID) *
							 depths[i].numVids);
	    if (!pVids)
	    {
		while (--i >= 0)
		    xfree(depths[i].vids);
		for (i = 0; i < NUMVISUALS; i++)
		    FreeResource(visuals[i].vid, RC_NONE);
		xfree(pPixmap);
		return FALSE;
	    }
	    for (j = 0; j < depths[i].numVids; j++)
		pVids[j] = visuals[j].vid;
	}
    }

    pScreen->defColormap = FakeClientID(0);
    if (defaultColorVisualClass < 0)
    {
	i = 0;
    }
    else
    {
	for (i = 0;
	     (i < NUMVISUALS) && (visuals[i].class != defaultColorVisualClass);
	     i++)
	    ;
	if (i >= NUMVISUALS)
	    i = 0;
    }
    pScreen->rootVisual = visuals[i].vid;
    if (cfbGCPrivateIndex == -1)
	cfbGCPrivateIndex = AllocateGCPrivateIndex ();
    if (cfbWindowPrivateIndex == -1)
	cfbWindowPrivateIndex = AllocateWindowPrivateIndex ();
    miInitializeBackingStore (pScreen, &cfbBSFuncRec);
    if (CreateColormap(pScreen->defColormap, pScreen, &visuals[i], &cmap,
		       (visuals[i].class & DynamicClass) ? AllocNone :
							   AllocAll,
		       0)
	== Success)
	return TRUE;
    for (i = 0; i < NUMDEPTHS; i++)
	xfree(depths[i].vids);
    for (i = 0; i < NUMVISUALS; i++)
	FreeResource(visuals[i].vid, RC_NONE);
    xfree(pPixmap);
    return FALSE;
}

