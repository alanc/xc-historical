/*
 * Copyright 1993 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include "X11/X.h"
#include "X11/Xproto.h"
#include "X11/Xos.h"
#include "scrnintstr.h"
#include "servermd.h"
#define PSZ 8
#include "cfb.h"
#include "mibstore.h"
#include "colormapst.h"
#include "gcstruct.h"

#define FFB_DEFAULT_WIDTH  1280
#define FFB_DEFAULT_HEIGHT 1024
#define FFB_DEFAULT_DEPTH  8

typedef struct
{
    int width;
    int paddedWidth;
    int height;
    int depth;
    int bitsPerPixel;
    char *pfbMemory;
} ffbScreenInfo, *ffbScreenInfoPtr;

static int ffbNumScreens;
static ffbScreenInfo ffbScreens[MAXSCREENS];
static Bool ffbPixmapDepths[33];

static void
ffbInitializePixmapDepths()
{
    int i;
    ffbPixmapDepths[1] = TRUE; /* always need bitmaps */
    for (i = 2; i <= 32; i++)
	ffbPixmapDepths[i] = FALSE;
}

static void
ffbInitializeDefaultScreens()
{
    int i;

    for (i = 0; i < MAXSCREENS; i++)
    {
	ffbScreens[i].width  = FFB_DEFAULT_WIDTH;
	ffbScreens[i].height = FFB_DEFAULT_HEIGHT;
	ffbScreens[i].depth  = FFB_DEFAULT_DEPTH;
	ffbScreens[i].pfbMemory = NULL;
    }
    ffbNumScreens = 1;
}

static int
ffbBitsPerPixel(depth)
    int depth;
{
    if (depth == 1) return 1;
    else if (depth <= 8) return 8;
    else if (depth <= 16) return 16;
    else return 32;
}

void
AbortDDX()
{
}

void
ddxGiveUp()
{
}

void
OsVendorInit()
{
}

void
ddxUseMsg()
{
    ErrorF("-screen <scrnum> <WxHxD>  set screen's width, height, depth\n");
    ErrorF("-pixdepths <list-of-int>  support given pixmap depths\n");
}

int
ddxProcessArgument (argc, argv, i)
    int argc;
    char *argv[];
    int i;
{
    static Bool firstTime = TRUE;

    if (firstTime)
    {
	ffbInitializeDefaultScreens();
	ffbInitializePixmapDepths();
        firstTime = FALSE;
    }

    if (strcmp (argv[i], "-screen") == 0)	/* -screen n WxHxD */
    {
	int screenNum;
	if (i + 2 >= argc) UseMsg();
	screenNum = atoi(argv[i+1]);
	if (screenNum < 0 || screenNum >= MAXSCREENS)
	{
	    ErrorF("Invalid screen number %d\n", screenNum);
	    UseMsg();
	}
	if (3 != sscanf(argv[i+2], "%dx%dx%d",
			&ffbScreens[screenNum].width,
			&ffbScreens[screenNum].height,
			&ffbScreens[screenNum].depth))
	{
	    ErrorF("Invalid screen configuration %s\n", argv[i+2]);
	    UseMsg();
	}

	if (screenNum >= ffbNumScreens)
	    ffbNumScreens = screenNum + 1;
	return 3;
    }

    if (strcmp (argv[i], "-pixdepths") == 0)	/* -pixdepths list-of-depth */
    {
	int depth, ret = 1;

	if (++i >= argc) UseMsg();
	while ((depth = atoi(argv[i++])) != 0)
	{
	    if (depth < 0 || depth > 32)
	    {
		ErrorF("Invalid pixmap depth %d\n", depth);
		UseMsg();
	    }
	    ffbPixmapDepths[depth] = TRUE;
	    ret++;
	}
	return ret;
    }

    return 0;
}

#ifdef DDXTIME /* from ServerOSDefines */
CARD32
GetTimeInMillis()
{
    struct timeval  tp;

    gettimeofday(&tp, 0);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
#endif


static Bool
ffbMultiDepthCreateGC(pGC)
    GCPtr   pGC;
{
    switch (ffbBitsPerPixel(pGC->depth))
    {
    case 1:  return mfbCreateGC (pGC);
    case 8:  return cfbCreateGC (pGC);
    case 16: return cfb16CreateGC (pGC);
    case 32: return cfb32CreateGC (pGC);
    default: return FALSE;
    }
}

static void
ffbMultiDepthGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    char		*pdstStart;	/* where to put the bits */
{
    switch (pDrawable->bitsPerPixel) {
    case 1:
	mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 8:
	cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 16:
	cfb16GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 32:
	cfb32GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    }
    return;
}

static void
ffbMultiDepthGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    char	*pdstLine;
{
    switch (pDrawable->bitsPerPixel)
    {
    case 1:
	mfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 8:
	cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 16:
	cfb16GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 32:
	cfb32GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    }
}

static ColormapPtr InstalledMaps[MAXSCREENS];

static int
ffbListInstalledColormaps(pScreen, pmaps)
    ScreenPtr	pScreen;
    Colormap	*pmaps;
{
    /* By the time we are processing requests, we can guarantee that there
     * is always a colormap installed */
    *pmaps = InstalledMaps[pScreen->myNum]->mid;
    return (1);
}


static void
ffbInstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr oldpmap = InstalledMaps[index];

    if(pmap != oldpmap)
    {
	/* Uninstall pInstalledMap. No hardware changes required, just
	 * notify all interested parties. */
	if(oldpmap != (ColormapPtr)None)
	    WalkTree(pmap->pScreen, TellLostMap, (char *)&oldpmap->mid);
	/* Install pmap */
	InstalledMaps[index] = pmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);
    }
}

static void
ffbUninstallColormap(pmap)
    ColormapPtr	pmap;
{
    ColormapPtr curpmap = InstalledMaps[pmap->pScreen->myNum];

    if(pmap == curpmap)
    {
	if (pmap->mid != pmap->pScreen->defColormap)
	{
	    curpmap = (ColormapPtr) LookupIDByType(pmap->pScreen->defColormap,
						   RT_COLORMAP);
	    (*pmap->pScreen->InstallColormap)(curpmap);
	}
    }
}

static Bool
ffbNoopReturnsTrue()
{
    return TRUE;
}

static char *
ffbAllocateFramebufferMemory(pffb)
    ffbScreenInfoPtr pffb;
{
    if (!pffb->pfbMemory)
    {
	int amount;
	if (pffb->bitsPerPixel == 1)
	    amount = (pffb->paddedWidth * pffb->height) / 8;
	else
	    amount = pffb->paddedWidth * pffb->height * (pffb->bitsPerPixel/8);
       
	pffb->pfbMemory = (char *)Xalloc(amount);
    }
    return pffb->pfbMemory;
}


static Bool
ffbScreenInit(index, pScreen, argc, argv)
    int index;
    ScreenPtr pScreen;
    int argc;
    char ** argv;
{
    ffbScreenInfoPtr pffb = &ffbScreens[index];
    int dpix = 100, dpiy = 100;
    int ret;
    char *pbits;

    pffb->paddedWidth = PixmapBytePad(pffb->width, pffb->depth);
    pffb->bitsPerPixel = ffbBitsPerPixel(pffb->depth);
    pbits = ffbAllocateFramebufferMemory(pffb); /* XXX free? */
    if (!pbits) return FALSE;

    switch (pffb->bitsPerPixel)
    {
    case 1:
	ret = mfbScreenInit(pScreen, pbits, pffb->width, pffb->height,
			    dpix, dpiy, pffb->paddedWidth);
	break;
    case 8:
	ret = cfbScreenInit(pScreen, pbits, pffb->width, pffb->height,
			    dpix, dpiy, pffb->paddedWidth);
	break;
    case 16:
	ret = cfb16ScreenInit(pScreen, pbits, pffb->width, pffb->height,
			      dpix, dpiy, pffb->paddedWidth);
	break;
    case 32:
	ret = cfb32ScreenInit(pScreen, pbits, pffb->width, pffb->height,
			      dpix, dpiy, pffb->paddedWidth);
	break;
    default:
	return FALSE;
    }

    pScreen->CreateGC = ffbMultiDepthCreateGC;
    pScreen->GetImage = ffbMultiDepthGetImage;
    pScreen->GetSpans = ffbMultiDepthGetSpans;

    pScreen->InstallColormap = ffbInstallColormap;
    pScreen->UninstallColormap = ffbUninstallColormap;
    pScreen->ListInstalledColormaps = ffbListInstalledColormaps;

    pScreen->SaveScreen = (SaveScreenProcPtr)ffbNoopReturnsTrue;
    pScreen->RealizeCursor = (RealizeCursorProcPtr)ffbNoopReturnsTrue;
    pScreen->UnrealizeCursor = (UnrealizeCursorProcPtr)ffbNoopReturnsTrue;
    pScreen->DisplayCursor = (DisplayCursorProcPtr)ffbNoopReturnsTrue;
    pScreen->SetCursorPosition = (SetCursorPositionProcPtr)ffbNoopReturnsTrue;
    pScreen->CursorLimits = (CursorLimitsProcPtr)ffbNoopReturnsTrue;
    pScreen->PointerNonInterestBox = (PointerNonInterestBoxProcPtr)
							ffbNoopReturnsTrue;
    pScreen->ConstrainCursor = (ConstrainCursorProcPtr)ffbNoopReturnsTrue;
    pScreen->RecolorCursor = (RecolorCursorProcPtr)ffbNoopReturnsTrue;
    pScreen->StoreColors = (StoreColorsProcPtr)ffbNoopReturnsTrue;

    return cfbCreateDefColormap(pScreen);
} /* end ffbScreenInit */


void
InitOutput(screenInfo, argc, argv)
    ScreenInfo *screenInfo;
    int argc;
    char **argv;
{
    int i;
    int NumFormats = 0;

    /* initialize pixmap formats */

    /* must have a pixmap depth to match every screen depth */
    for (i = 0; i < ffbNumScreens; i++)
    {
	ffbPixmapDepths[ffbScreens[i].depth] = TRUE;
    }

    for (i = 1; i <= 32; i++)
    {
	if (ffbPixmapDepths[i])
	{
	    if (NumFormats >= MAXFORMATS)
		FatalError ("MAXFORMATS is too small for this server\n");
	    screenInfo->formats[NumFormats].depth = i;
	    screenInfo->formats[NumFormats].bitsPerPixel = ffbBitsPerPixel(i);
	    screenInfo->formats[NumFormats].scanlinePad = BITMAP_SCANLINE_PAD;
	    NumFormats++;
	}
    }

    screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
    screenInfo->numPixmapFormats = NumFormats;

    /* initialize screens */

    for (i = 0; i < ffbNumScreens; i++)
    {
	if (-1 == AddScreen(ffbScreenInit, argc, argv))
	{
	    FatalError("Couldn't add screen %d", i);
	}
    }
}

