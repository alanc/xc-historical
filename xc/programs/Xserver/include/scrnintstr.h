/* $XConsortium: scrnintstr.h,v 5.19 93/09/29 17:08:44 dpw Exp $ */
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
#ifndef SCREENINTSTRUCT_H
#define SCREENINTSTRUCT_H

#include "screenint.h"
#include "miscstruct.h"
#include "region.h"
#include "pixmap.h"
#include "gc.h"
#include "colormap.h"
#include "cursor.h"
#include "validate.h"
#include "window.h"
#include "X11/Xproto.h"
#include "dix.h"

typedef struct _PixmapFormat {
    unsigned char	depth;
    unsigned char	bitsPerPixel;
    unsigned char	scanlinePad;
    } PixmapFormatRec;
    
typedef struct _Visual {
    unsigned long	vid;
    short		class;
    short		bitsPerRGBValue;
    short		ColormapEntries;
    short		nplanes;/* = log2 (ColormapEntries). This does not
				 * imply that the screen has this many planes.
				 * it may have more or fewer */
    unsigned long	redMask, greenMask, blueMask;
    int			offsetRed, offsetGreen, offsetBlue;
  } VisualRec;

typedef struct _Depth {
    unsigned char	depth;
    short		numVids;
    unsigned long	*vids;    /* block of visual ids for this depth */
  } DepthRec;

typedef struct _Screen {
    int			myNum;	/* index of this instance in Screens[] */
    ATOM		id;
    short		width, height;
    short		mmWidth, mmHeight;
    short		numDepths;
    unsigned char      	rootDepth;
    DepthPtr       	allowedDepths;
    unsigned long      	rootVisual;
    unsigned long	defColormap;
    short		minInstalledCmaps, maxInstalledCmaps;
    char                backingStoreSupport, saveUnderSupport;
    unsigned long	whitePixel, blackPixel;
    unsigned long	rgf;	/* array of flags; she's -- HUNGARIAN */
    GCPtr		GCperDepth[MAXFORMATS+1];
			/* next field is a stipple to use as default in
			   a GC.  we don't build default tiles of all depths
			   because they are likely to be of a color
			   different from the default fg pixel, so
			   we don't win anything by building
			   a standard one.
			*/
    PixmapPtr		PixmapPerDepth[1];
    pointer		devPrivate;
    short       	numVisuals;
    VisualPtr		visuals;
    int			WindowPrivateLen;
    unsigned		*WindowPrivateSizes;
    unsigned		totalWindowSize;
    int			GCPrivateLen;
    unsigned		*GCPrivateSizes;
    unsigned		totalGCSize;

    /* Random screen procedures */

    Bool (* CloseScreen)(
#if NeedNestedPrototypes
	int /*index*/,
	ScreenPtr /*pScreen*/
#endif
);

    void (* QueryBestSize)(
#if NeedNestedPrototypes
	int /*class*/,
	unsigned short* /*pwidth*/,
	unsigned short* /*pheight*/,
	ScreenPtr /*pScreen*/
#endif
);

    Bool (* SaveScreen)(
#if NeedNestedPrototypes
	 ScreenPtr /*pScreen*/,
	 int /*on*/
#endif
);

    void (* GetImage)(
#if NeedNestedPrototypes
	DrawablePtr /*pDrawable*/,
	int /*sx*/,
	int /*sy*/,
	int /*w*/,
	int /*h*/,
	unsigned int /*format*/,
	unsigned long /*planeMask*/,
	char * /*pdstLine*/
#endif
);

    void (* GetSpans)(
#if NeedNestedPrototypes
	DrawablePtr /*pDrawable*/,
	int /*wMax*/,
	DDXPointPtr /*ppt*/,
	int* /*pwidth*/,
	int /*nspans*/,
	char * /*pdstStart*/
#endif
);

    void (* PointerNonInterestBox)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	BoxPtr /*pBox*/
#endif
);

    void (* SourceValidate)(
#if NeedNestedPrototypes
	DrawablePtr /*pDrawable*/,
	int /*x*/,
	int /*y*/,
	int /*width*/,
	int /*height*/
#endif
);

    /* Window Procedures */

    Bool (* CreateWindow)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/
#endif
);

    Bool (* DestroyWindow)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/
#endif
);

    Bool (* PositionWindow)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	int /*x*/,
	int /*y*/
#endif
);

    Bool (* ChangeWindowAttributes)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	BITS32 /*mask*/
#endif
);

    Bool (* RealizeWindow)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/
#endif
);

    Bool (* UnrealizeWindow)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/
#endif
);

    int  (* ValidateTree)(
#if NeedNestedPrototypes
	WindowPtr /*pParent*/,
	WindowPtr /*pChild*/,
	VTKind /*kind*/
#endif
);

    void (* PostValidateTree)(
#if NeedNestedPrototypes
	WindowPtr /*pParent*/,
	WindowPtr /*pChild*/,
	VTKind /*kind*/
#endif
);

    void (* WindowExposures)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	RegionPtr /*prgn*/,
	RegionPtr /*other_exposed*/
#endif
);

    void (* PaintWindowBackground)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	RegionPtr /*pRegion*/,
	int /*what*/
#endif
);

    void (* PaintWindowBorder)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	RegionPtr /*pRegion*/,
	int /*what*/
#endif
);

    void (* CopyWindow)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	DDXPointRec /*ptOldOrg*/,
	RegionPtr /*prgnSrc*/
#endif
);

    void (* ClearToBackground)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	int /*x*/,
	int /*y*/,
	int /*w*/,
	int /*h*/,
	Bool /*generateExposures*/
#endif
);

    void (* ClipNotify)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	int /*dx*/,
	int /*dy*/
#endif
);

    /* Pixmap procedures */

    PixmapPtr (* CreatePixmap)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	int /*width*/,
	int /*height*/,
	int /*depth*/
#endif
);

    Bool (* DestroyPixmap)(
#if NeedNestedPrototypes
	PixmapPtr /*pPixmap*/
#endif
);

    /* Backing store procedures */

    void (* SaveDoomedAreas)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	RegionPtr /*prgnSave*/,
	int /*xorg*/,
	int /*yorg*/
#endif
);

    RegionPtr (* RestoreAreas)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	RegionPtr /*prgnRestore*/
#endif
);

    void (* ExposeCopy)(
#if NeedNestedPrototypes
	WindowPtr /*pSrc*/,
	DrawablePtr /*pDst*/,
	GCPtr /*pGC*/,
	RegionPtr /*prgnExposed*/,
	int /*srcx*/,
	int /*srcy*/,
	int /*dstx*/,
	int /*dsty*/,
	unsigned long /*plane*/
#endif
);

    RegionPtr (* TranslateBackingStore)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	int /*windx*/,
	int /*windy*/,
	RegionPtr /*oldClip*/,
	int /*oldx*/,
	int /*oldy*/
#endif
);

    RegionPtr (* ClearBackingStore)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	int /*x*/,
	int /*y*/,
	int /*w*/,
	int /*h*/,
	Bool /*generateExposures*/
#endif
);

    void (* DrawGuarantee)(
#if NeedNestedPrototypes
	WindowPtr /*pWindow*/,
	GCPtr /*pGC*/,
	int /*guarantee*/
#endif
);
    
    /* Font procedures */

    Bool (* RealizeFont)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	FontPtr /*pFont*/
#endif
);

    Bool (* UnrealizeFont)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	FontPtr /*pFont*/
#endif
);

    /* Cursor Procedures */
    void (* ConstrainCursor)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	BoxPtr /*pBox*/
#endif
);

    void (* CursorLimits)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	CursorPtr /*pCursor*/,
	BoxPtr /*pHotBox*/,
	BoxPtr /*pTopLeftBox*/
#endif
);

    Bool (* DisplayCursor)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	CursorPtr /*pCursor*/
#endif
);

    Bool (* RealizeCursor)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	CursorPtr /*pCursor*/
#endif
);

    Bool (* UnrealizeCursor)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	CursorPtr /*pCursor*/
#endif
);

    void (* RecolorCursor)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	CursorPtr /*pCursor*/,
	Bool /*displayed*/
#endif
);

    Bool (* SetCursorPosition)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	int /*x*/,
	int /*y*/,
	Bool /*generateEvent*/
#endif
);

    /* GC procedures */

    Bool (* CreateGC)(
#if NeedNestedPrototypes
	GCPtr /*pGC*/
#endif
);

    /* Colormap procedures */

    Bool (* CreateColormap)(
#if NeedNestedPrototypes
	ColormapPtr /*pColormap*/
#endif
);

    void (* DestroyColormap)(
#if NeedNestedPrototypes
	ColormapPtr /*pColormap*/
#endif
);

    void (* InstallColormap)(
#if NeedNestedPrototypes
	ColormapPtr /*pColormap*/
#endif
);

    void (* UninstallColormap)(
#if NeedNestedPrototypes
	ColormapPtr /*pColormap*/
#endif
);

    int (* ListInstalledColormaps) (
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/,
	XID* /*pmaps */
#endif
);

    void (* StoreColors)(
#if NeedNestedPrototypes
	ColormapPtr /*pColormap*/,
	int /*ndef*/,
	xColorItem * /*pdef*/
#endif
);

    void (* ResolveColor)(
#if NeedNestedPrototypes
	unsigned short* /*pred*/,
	unsigned short* /*pgreen*/,
	unsigned short* /*pblue*/,
	VisualPtr /*pVisual*/
#endif
);

    /* Region procedures */

    RegionPtr (* RegionCreate)(
#if NeedNestedPrototypes
	BoxPtr /*rect*/,
	int /*size*/
#endif
);

    void (* RegionInit)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/,
	BoxPtr /*rect*/,
	int /*size*/
#endif
);

    Bool (* RegionCopy)(
#if NeedNestedPrototypes
	RegionPtr /*dst*/,
	RegionPtr /*src*/
#endif
);

    void (* RegionDestroy)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/
#endif
);

    void (* RegionUninit)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/
#endif
);

    Bool (* Intersect)(
#if NeedNestedPrototypes
	RegionPtr /*newReg*/,
	RegionPtr /*reg1*/,
	RegionPtr /*reg2*/
#endif
);

    Bool (* Union)(
#if NeedNestedPrototypes
	RegionPtr /*newReg*/,
	RegionPtr /*reg1*/,
	RegionPtr /*reg2*/
#endif
);

    Bool (* Subtract)(
#if NeedNestedPrototypes
	RegionPtr /*regD*/,
	RegionPtr /*regM*/,
	RegionPtr /*regS*/
#endif
);

    Bool (* Inverse)(
#if NeedNestedPrototypes
	RegionPtr /*newReg*/,
	RegionPtr /*reg1*/,
	BoxPtr /*invRect*/
#endif
);

    void (* RegionReset)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/,
	BoxPtr /*pBox*/
#endif
);

    void (* TranslateRegion)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/,
	int /*x*/,
	int /*y*/
#endif
);

    int (* RectIn)(
#if NeedNestedPrototypes
	RegionPtr /*region*/,
	BoxPtr /*prect*/
#endif
);

    Bool (* PointInRegion)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/,
	int /*x*/,
	int /*y*/,
	BoxPtr /*box*/
#endif
);

    Bool (* RegionNotEmpty)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/
#endif
);

    void (* RegionEmpty)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/
#endif
);

    BoxPtr (* RegionExtents)(
#if NeedNestedPrototypes
	RegionPtr /*pReg*/
#endif
);

    Bool (* RegionAppend)(
#if NeedNestedPrototypes
	RegionPtr /*dstrgn*/,
	RegionPtr /*rgn*/
#endif
);

    Bool (* RegionValidate)(
#if NeedNestedPrototypes
	RegionPtr /*badreg*/,
	Bool* /*pOverlap*/
#endif
);

    RegionPtr (* BitmapToRegion)(
#if NeedNestedPrototypes
	PixmapPtr /*pPix*/
#endif
);

    RegionPtr (* RectsToRegion)(
#if NeedNestedPrototypes
	int /*nrects*/,
	xRectangle* /*prect*/,
	int /*ctype*/
#endif
);

    void (* SendGraphicsExpose)(
#if NeedNestedPrototypes
	ClientPtr /*client*/,
	RegionPtr /*pRgn*/,
	XID /*drawable*/,
	int /*major*/,
	int /*minor*/
#endif
);

    /* os layer procedures */
    void (* BlockHandler)(
#if NeedNestedPrototypes
	int /*screenNum*/,
	pointer /*blockData*/,
	struct timeval ** /*pTimeout*/,
	pointer /*pReadmask*/
#endif
);

    void (* WakeupHandler)(
#if NeedNestedPrototypes
	 int /*screenNum*/,
	 pointer /*wakeupData*/,
	 unsigned long /*result*/,
	 pointer /*pReadMask*/
#endif
);

    pointer blockData;
    pointer wakeupData;

    /* anybody can get a piece of this array */
    DevUnion	*devPrivates;

    Bool (* CreateScreenResources)(
#if NeedNestedPrototypes
	ScreenPtr /*pScreen*/
#endif
);

    Bool (* ModifyPixmapHeader)(
#if NeedNestedPrototypes
	PixmapPtr /*pPixmap*/,
	int /*width*/,
	int /*height*/,
	int /*depth*/,
	int /*bitsPerPixel*/,
	int /*devKind*/,
	pointer /*pPixData*/
#endif
);

    PixmapPtr pScratchPixmap;		/* scratch pixmap "pool" */

#ifdef PIXPRIV
    int			PixmapPrivateLen;
    unsigned		*PixmapPrivateSizes;
    unsigned		totalPixmapSize;
#endif

    void (* MarkWindow)(
#if NeedNestedPrototypes
	WindowPtr /*pWin*/
#endif
);

    Bool (* MarkOverlappedWindows)(
#if NeedNestedPrototypes
	WindowPtr /*parent*/,
	WindowPtr /*firstChild*/,
	WindowPtr * /*pLayerWin*/
#endif
);


    Bool (* ChangeSaveUnder)(
#if NeedNestedPrototypes
	WindowPtr /*pLayerWin*/,
	WindowPtr /*firstChild*/
#endif
);

    void (* PostChangeSaveUnder)(
#if NeedNestedPrototypes
	WindowPtr /*pLayerWin*/,
	WindowPtr /*firstChild*/
#endif
);

    void (* MoveWindow)(
#if NeedNestedPrototypes
	WindowPtr /*pWin*/,
	int /*x*/,
	int /*y*/,
	WindowPtr /*pSib*/,
	VTKind /*kind*/
#endif
);

    void (* ResizeWindow)(
#if NeedNestedPrototypes
    WindowPtr /*pWin*/,
    int /*x*/,
    int /*y*/, 
    unsigned int /*w*/,
    unsigned int /*h*/,
    WindowPtr /*pSib*/
#endif
);

    WindowPtr (* GetLayerWindow)(
#if NeedNestedPrototypes
    WindowPtr /*pWin*/
#endif
);

    void (* HandleExposures)(
#if NeedNestedPrototypes
    WindowPtr /*pWin*/
#endif
);

    void (* ReparentWindow)(
#if NeedNestedPrototypes
    WindowPtr /*pWin*/,
    WindowPtr /*pPriorParent*/
#endif
);

#ifdef SHAPE
    void (* SetShape)(
#if NeedFunctionPrototypes
	WindowPtr /*pWin*/
#endif
);
#endif /* SHAPE */

    void (* ChangeBorderWidth)(
#if NeedFunctionPrototypes
	WindowPtr /*pWin*/,
	unsigned int /*width*/
#endif
);

    void (* MarkUnrealizedWindow)(
#if NeedFunctionPrototypes
	WindowPtr /*pChild*/,
	WindowPtr /*pWin*/,
	Bool /*fromConfigure*/
#endif
);

} ScreenRec;

typedef struct _ScreenInfo {
    int		imageByteOrder;
    int		bitmapScanlineUnit;
    int		bitmapScanlinePad;
    int		bitmapBitOrder;
    int		numPixmapFormats;
    PixmapFormatRec
		formats[MAXFORMATS];
    int		arraySize;
    int		numScreens;
    ScreenPtr	screens[MAXSCREENS];
} ScreenInfo;

extern ScreenInfo screenInfo;

#endif /* SCREENINTSTRUCT_H */
