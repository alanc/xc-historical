/*
 * The Sun X drivers are a product of Sun Microsystems, Inc. and are provided
 * for unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify these drivers without charge, but are not authorized
 * to license or distribute them to anyone else except as part of a product or
 * program developed by the user.
 * 
 * THE SUN X DRIVERS ARE PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND
 * INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE
 * PRACTICE.
 *
 * The Sun X Drivers are provided with no support and without any obligation
 * on the part of Sun Microsystems, Inc. to assist in their use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THE SUN X
 * DRIVERS OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#include "X.h"
#include "Xmd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"

extern void miGetImage();	/* XXX should not be needed */
extern ColormapPtr CreateStaticColormap();	/* XXX is this needed? */

static VisualRec visuals[] = {
/* vid screen class rMask gMask bMask oRed oGreen oBlue bpRGB cmpE nplan */
#ifdef	notdef
    /*  Eventually,  we would like to offer this visual too */
    0,	0, StaticGray, 0,   0,    0,   0,    0,	  0,    1,   2,    1,
#endif
#ifdef	STATIC_COLOR
    0,  0, StaticColor,0,   0,    0,   0,    0,   0,    8,  256,   8,
#else
    0,  0, PseudoColor,0,   0,    0,   0,    0,   0,    8,  256,   8,
#endif
};

#define	NUMVISUALS	((sizeof visuals)/(sizeof visuals[0]))
#define	ROOTVISUAL	(NUMVISUALS-1)

static DepthRec depths[] = {
/* depth	numVid		vids */
    1,		0,		NULL,
    8,		1,		NULL,
};

static ColormapPtr cfbColorMaps[NUMVISUALS];	/* assume one per visual */
#define NUMDEPTHS	((sizeof depths)/(sizeof depths[0]))

/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
cfbScreenInit(index, pScreen, pbits, xsize, ysize, dpi)
    int index;
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpi;			/* dots per inch */
{
    long	*pVids;
    register PixmapPtr pPixmap;
    int	i;

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
    pScreen->backingStoreSupport = NotUseful;
    pScreen->saveUnderSupport = NotUseful;

    /* cursmin and cursmax are device specific */

    pScreen->numVisuals = NUMVISUALS;
    pScreen->visuals = visuals;

    pPixmap = (PixmapPtr )Xalloc(sizeof(PixmapRec));
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.depth = 8;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.serialNumber = 0;
    pPixmap->width = xsize;
    pPixmap->height = ysize;
    pPixmap->refcnt = 1;
    pPixmap->devPrivate = pbits;
    pPixmap->devKind = PixmapBytePad(xsize, 8);
    pScreen->devPrivate = (pointer)pPixmap;

    /* anything that cfb doesn't know about is assumed to be done
       elsewhere.  (we put in no-op only for things that we KNOW
       are really no-op.
    */
    pScreen->QueryBestSize = cfbQueryBestSize;
    pScreen->CreateWindow = cfbCreateWindow;
    pScreen->DestroyWindow = cfbDestroyWindow;
    pScreen->PositionWindow = cfbPositionWindow;
    pScreen->ChangeWindowAttributes = cfbChangeWindowAttributes;
    pScreen->RealizeWindow = cfbMapWindow;
    pScreen->UnrealizeWindow = cfbUnmapWindow;

    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
    pScreen->GetImage = miGetImage;
    pScreen->GetSpans = cfbGetSpans;	/* XXX */
    pScreen->CreateGC = cfbCreateGC;
    pScreen->CreatePixmap = cfbCreatePixmap;
    pScreen->DestroyPixmap = cfbDestroyPixmap;
    pScreen->ValidateTree = miValidateTree;

#ifdef	STATIC_COLOR
    pScreen->InstallColormap = NoopDDA;
    pScreen->UninstallColormap = NoopDDA;
    pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
#endif
    pScreen->GetStaticColormap = cfbGetStaticColormap;
#ifdef	STATIC_COLOR
    pScreen->StoreColors = NoopDDA;
    pScreen->ResolveColor = cfbResolveStaticColor;
#endif

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

    /*  Set up the remaining fields in the visuals[] array & make a RT_VISUALID */
    for (i = 0; i < NUMVISUALS; i++) {
	visuals[i].vid = FakeClientID(0);
	visuals[i].screen = index;
	AddResource(visuals[i].vid, RT_VISUALID, &visuals[i], NoopDDA, RC_CORE);
	switch (visuals[i].class) {
	case StaticGray:
	case StaticColor:
	    CreateColormap(FakeClientID(0), pScreen, &visuals[i], 
		&cfbColorMaps[i], AllocAll, 0);
	    break;
	case PseudoColor:
	case GrayScale:
	    CreateColormap(FakeClientID(0), pScreen, &visuals[i], 
		&cfbColorMaps[i], AllocNone, 0);
	    break;
	case TrueColor:
	case DirectColor:
	    FatalError("Bad visual in cfbScreenInit\n");
	}
	if (!cfbColorMaps[i])
	    FatalError("Can't create colormap in cfbScreenInit\n");
	cfbInitialize332Colormap(cfbColorMaps[i]);
    }
    pScreen->defColormap = cfbColorMaps[ROOTVISUAL]->mid;
    pScreen->rootVisual = visuals[ROOTVISUAL].vid;

    /*  Set up the remaining fields in the depths[] array */
    for (i = 0; i < NUMDEPTHS; i++) {
	if (depths[i].numVids > 0) {
	    depths[i].vids = pVids = (long *) Xalloc(sizeof (long) * depths[i].numVids);
	    /* XXX - here we offer only the 8-bit visual */
	    pVids[0] = visuals[ROOTVISUAL].vid;
	}
    }
    return( TRUE );
}

