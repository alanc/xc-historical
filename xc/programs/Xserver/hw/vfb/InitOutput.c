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
#define NEED_EVENTS
#include "X11/Xproto.h"
#include "X11/Xos.h"
#include "scrnintstr.h"
#include "servermd.h"
#define PSZ 8
#include "cfb.h"
#include "mibstore.h"
#include "colormapst.h"
#include "gcstruct.h"
#include "input.h"
#include "mipointer.h"
#include <sys/types.h>
#ifdef HAS_MMAP
#include <sys/mman.h>
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#endif /* HAS_MMAP */
#include <sys/stat.h>
#include <errno.h>
#include <sys/param.h>
#include <X11/XWDFile.h>
#ifdef HAS_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#endif /* HAS_SHM */
#include "dix.h"

extern char *display;

#define FFB_DEFAULT_WIDTH  1280
#define FFB_DEFAULT_HEIGHT 1024
#define FFB_DEFAULT_DEPTH  8
#define XWD_WINDOW_NAME_LEN 64

typedef struct
{
    int scrnum;
    int width;
    int paddedWidth;
    int height;
    int depth;
    int bitsPerPixel;
    int sizeInBytes;
    int ncolors;
    char *pfbMemory;
    XWDColor *pXWDCmap;
    XWDFileHeader *pXWDHeader;

#ifdef HAS_MMAP
    int mmap_fd;
    char mmap_file[MAXPATHLEN];
#endif

#ifdef HAS_SHM
    int shmid;
#endif
} ffbScreenInfo, *ffbScreenInfoPtr;

static int ffbNumScreens;
static ffbScreenInfo ffbScreens[MAXSCREENS];
static Bool ffbPixmapDepths[33];
static char *pfbdir = NULL;
typedef enum { NORMAL_MEMORY_FB, SHARED_MEMORY_FB, MMAPPED_FILE_FB } fbMemType;
static fbMemType fbmemtype = NORMAL_MEMORY_FB;
static char needswap = 0;

#define swapcopy16(_dst, _src) \
    if (needswap) { CARD16 _s = _src; cpswaps(_s, _dst); } \
    else _dst = _src;

#define swapcopy32(_dst, _src) \
    if (needswap) { CARD32 _s = _src; cpswapl(_s, _dst); } \
    else _dst = _src;


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
	ffbScreens[i].scrnum = i;
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
ddxGiveUp()
{
    int i;

    /* clean up the framebuffers */

    switch (fbmemtype)
    {
#ifdef HAS_MMAP
    case MMAPPED_FILE_FB: 
	for (i = 0; i < ffbNumScreens; i++)
	{
	    if (-1 == unlink(ffbScreens[i].mmap_file))
	    {
		perror("unlink");
		ErrorF("unlink %s failed, errno %d",
		       ffbScreens[i].mmap_file, errno);
	    }
	}
	break;
#endif /* HAS_MMAP */
	
#ifdef HAS_SHM
    case SHARED_MEMORY_FB:
	for (i = 0; i < ffbNumScreens; i++)
	{
	    if (-1 == shmdt((char *)ffbScreens[i].pXWDHeader))
	    {
		perror("shmdt");
		ErrorF("shmdt failed, errno %d", errno);
	    }
	}
	break;
#endif /* HAS_SHM */
	
    case NORMAL_MEMORY_FB:
	for (i = 0; i < ffbNumScreens; i++)
	{
	    Xfree(ffbScreens[i].pXWDHeader);
	}
	break;
    }
}

void
AbortDDX()
{
    ddxGiveUp();
}

void
OsVendorInit()
{
}

void
ddxUseMsg()
{
    ErrorF("-screen scrn WxHxD     set screen's width, height, depth\n");
    ErrorF("-pixdepths list-of-int support given pixmap depths\n");

#ifdef HAS_MMAP
    ErrorF("-fbdir directory       put framebuffers in mmap'ed files in directory\n");
#endif

#ifdef HAS_SHM
    ErrorF("-shmem                 put framebuffers in shared memory\n");
#endif
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
	while ((i < argc) && (depth = atoi(argv[i++])) != 0)
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

#ifdef HAS_MMAP
    if (strcmp (argv[i], "-fbdir") == 0)	/* -fbdir directory */
    {
	if (++i >= argc) UseMsg();
	pfbdir = argv[i];
	fbmemtype = MMAPPED_FILE_FB;
	return 2;
    }
#endif /* HAS_MMAP */

#ifdef HAS_SHM
    if (strcmp (argv[i], "-shmem") == 0)	/* -shmem */
    {
	fbmemtype = SHARED_MEMORY_FB;
	return 1;
    }
#endif

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

    if (pmap != oldpmap)
    {
	int entries;
	XWDFileHeader *pXWDHeader;
	XWDColor *pXWDCmap;
	VisualPtr pVisual;
	Pixel *     ppix;
	xrgb *      prgb;
	xColorItem *defs;
	int i;

	if(oldpmap != (ColormapPtr)None)
	    WalkTree(pmap->pScreen, TellLostMap, (char *)&oldpmap->mid);
	/* Install pmap */
	InstalledMaps[index] = pmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

	entries = pmap->pVisual->ColormapEntries;
	pXWDHeader = ffbScreens[pmap->pScreen->myNum].pXWDHeader;
	pXWDCmap = ffbScreens[pmap->pScreen->myNum].pXWDCmap;
	pVisual = pmap->pVisual;

	swapcopy32(pXWDHeader->visual_class, pVisual->class);
	swapcopy32(pXWDHeader->red_mask, pVisual->redMask);
	swapcopy32(pXWDHeader->green_mask, pVisual->greenMask);
	swapcopy32(pXWDHeader->blue_mask, pVisual->blueMask);
	swapcopy32(pXWDHeader->bits_per_rgb, pVisual->bitsPerRGBValue);
	swapcopy32(pXWDHeader->colormap_entries, pVisual->ColormapEntries);

	ppix = (Pixel *)ALLOCATE_LOCAL(entries * sizeof(Pixel));
	prgb = (xrgb *)ALLOCATE_LOCAL(entries * sizeof(xrgb));
	defs = (xColorItem *)ALLOCATE_LOCAL(entries * sizeof(xColorItem));

	for (i = 0; i < entries; i++)  ppix[i] = i;

	QueryColors(pmap, entries, ppix, prgb);

	for (i = 0; i < entries; i++) { /* convert xrgbs to xColorItems */
	    defs[i].pixel = ppix[i] & 0xff; /* change pixel to index */
	    defs[i].red = prgb[i].red;
	    defs[i].green = prgb[i].green;
	    defs[i].blue = prgb[i].blue;
	    defs[i].flags =  DoRed|DoGreen|DoBlue;
	}
	(*pmap->pScreen->StoreColors)(pmap, entries, defs);

	DEALLOCATE_LOCAL(ppix);
	DEALLOCATE_LOCAL(prgb);
	DEALLOCATE_LOCAL(defs);
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

void
ffbStoreColors(pmap, ndef, pdefs)
    ColormapPtr pmap;
    int         ndef;
    xColorItem  *pdefs;
{
    XWDColor *pXWDCmap;
    int i;

    if (pmap != InstalledMaps[pmap->pScreen->myNum]) return;

    pXWDCmap = ffbScreens[pmap->pScreen->myNum].pXWDCmap;

    /* XXX direct color doesn't work here yet */
    if ((pmap->pVisual->class | DynamicClass) == DirectColor)
	return;

    for (i = 0; i < ndef; i++)
    {
	if (pdefs[i].flags & DoRed)
	    swapcopy16(pXWDCmap[pdefs[i].pixel].red, pdefs[i].red);
	if (pdefs[i].flags & DoGreen)
	    swapcopy16(pXWDCmap[pdefs[i].pixel].green, pdefs[i].green);
	if (pdefs[i].flags & DoBlue)
	    swapcopy16(pXWDCmap[pdefs[i].pixel].blue, pdefs[i].blue);
    }
}

static Bool
ffbSaveScreen(pScreen, on)
    ScreenPtr pScreen;
    int on;
{
    return TRUE;
}

#ifdef HAS_MMAP

void
ffbBlockHandler(blockData, pTimeout, pReadmask)
    pointer   blockData;
    OSTimePtr pTimeout;
    pointer   pReadmask;
{
    int i;

    for (i = 0; i < ffbNumScreens; i++)
    {
	if (-1 == msync((caddr_t)ffbScreens[i].pXWDHeader,
			(size_t)ffbScreens[i].sizeInBytes, MS_ASYNC))
	{
	    perror("msync");
	    ErrorF("msync failed, errno %d", errno);
	}
    }
}


void
ffbWakeupHandler(blockData, result, pReadmask)
    pointer blockData;
    int     result;
    pointer pReadmask;
{
}


void
ffbAllocateMmappedFramebuffer(pffb)
    ffbScreenInfoPtr pffb;
{
#define DUMMY_BUFFER_SIZE 65536
    char dummyBuffer[DUMMY_BUFFER_SIZE];
    int currentFileSize, writeThisTime;

    sprintf(pffb->mmap_file, "%s/Xffb_screen%d", pfbdir, pffb->scrnum);
    if (-1 == (pffb->mmap_fd = open(pffb->mmap_file, O_CREAT|O_RDWR, 0666)))
    {
	perror("open");
	ErrorF("open %s failed, errno %d", pffb->mmap_file, errno);
	return;
    }

    /* Extend the file to be the proper size */

    bzero(dummyBuffer, DUMMY_BUFFER_SIZE);
    for (currentFileSize = 0;
	 currentFileSize < pffb->sizeInBytes;
	 currentFileSize += writeThisTime)
    {
	writeThisTime = min(DUMMY_BUFFER_SIZE,
			    pffb->sizeInBytes - currentFileSize);
	if (-1 == write(pffb->mmap_fd, dummyBuffer, writeThisTime))
	{
	    perror("write");
	    ErrorF("write %s failed, errno %d", pffb->mmap_file, errno);
	    return;
	}
    }

    /* try to mmap the file */

    pffb->pXWDHeader = (XWDFileHeader *)mmap((caddr_t)NULL, pffb->sizeInBytes,
				    PROT_READ|PROT_WRITE,
				    MAP_FILE|MAP_SHARED,
				    pffb->mmap_fd, 0);
    if (-1 == (int)pffb->pXWDHeader)
    {
	perror("mmap");
	ErrorF("mmap %s failed, errno %d", pffb->mmap_file, errno);
	pffb->pXWDHeader = NULL;
	return;
    }

    if (!RegisterBlockAndWakeupHandlers(ffbBlockHandler, ffbWakeupHandler,
					NULL))
    {
	pffb->pXWDHeader = NULL;
    }
}
#endif /* HAS_MMAP */


#ifdef HAS_SHM
void
ffbAllocateSharedMemoryFramebuffer(pffb)
    ffbScreenInfoPtr pffb;
{
    /* create the shared memory segment */

    pffb->shmid = shmget(IPC_PRIVATE, pffb->sizeInBytes, IPC_CREAT|0777);
    if (pffb->shmid < 0)
    {
	perror("shmget");
	ErrorF("shmget %d bytes failed, errno %d", pffb->sizeInBytes, errno);
	return;
    }

    /* try to attach it */

    pffb->pXWDHeader = (XWDFileHeader *)shmat(pffb->shmid, 0, 0);
    if (-1 == (int)pffb->pXWDHeader)
    {
	perror("shmat");
	ErrorF("shmat failed, errno %d", errno);
	pffb->pXWDHeader = NULL; 
	return;
    }

    ErrorF("screen %d shmid %d\n", pffb->scrnum, pffb->shmid);
}
#endif /* HAS_SHM */

static char *
ffbAllocateFramebufferMemory(pffb)
    ffbScreenInfoPtr pffb;
{
    if (pffb->pfbMemory) return pffb->pfbMemory; /* already done */

    if (pffb->bitsPerPixel == 1)
	pffb->sizeInBytes = (pffb->paddedWidth * pffb->height);
    else
	pffb->sizeInBytes = pffb->paddedWidth * pffb->height *
			    (pffb->bitsPerPixel/8);

    /* XXX calculate how many entries in colormap */

    if (pffb->depth <= 10)
    {
	pffb->ncolors = 1 << pffb->depth;
    }
    else
    {
	int nplanes = pffb->depth / 3;
	if (pffb->depth % 3) nplanes++;
	pffb->ncolors = 3 * (1 << nplanes);
    }

    /* add extra bytes for XWDFileHeader, window name, and colormap */

    pffb->sizeInBytes += SIZEOF(XWDheader) + XWD_WINDOW_NAME_LEN +
		    pffb->ncolors * SIZEOF(XWDColor);

    pffb->pXWDHeader = NULL; 
    switch (fbmemtype)
    {
#ifdef HAS_MMAP
    case MMAPPED_FILE_FB:  ffbAllocateMmappedFramebuffer(pffb); break;
#endif

#ifdef HAS_SHM
    case SHARED_MEMORY_FB: ffbAllocateSharedMemoryFramebuffer(pffb); break;
#endif

    case NORMAL_MEMORY_FB:
	pffb->pXWDHeader = (XWDFileHeader *)Xalloc(pffb->sizeInBytes);
	break;
    }

    if (pffb->pXWDHeader)
    {
	pffb->pXWDCmap = (XWDColor *)((char *)pffb->pXWDHeader
				+ SIZEOF(XWDheader) + XWD_WINDOW_NAME_LEN);
	pffb->pfbMemory = (char *)(pffb->pXWDCmap + pffb->ncolors);
	return pffb->pfbMemory;
    }
    else
	return NULL;
}


void
ffbWriteXWDFileHeader(pScreen)
    ScreenPtr pScreen;
{
    ffbScreenInfoPtr pffb = &ffbScreens[pScreen->myNum];
    XWDFileHeader *pXWDHeader = pffb->pXWDHeader;
    char hostname[XWD_WINDOW_NAME_LEN];
    VisualPtr	pVisual;
    unsigned long swaptest = 1;
    int i;

    needswap = *(char *) &swaptest;

    pXWDHeader->header_size = (char *)pffb->pXWDCmap - (char *)pffb->pXWDHeader;
    pXWDHeader->file_version = XWD_FILE_VERSION;

    pXWDHeader->pixmap_format = ZPixmap;
    pXWDHeader->pixmap_depth = pffb->depth;
    pXWDHeader->pixmap_width = pXWDHeader->window_width = pffb->width;
    pXWDHeader->pixmap_height = pXWDHeader->window_height = pffb->height;
    pXWDHeader->xoffset = 0;
    pXWDHeader->byte_order = IMAGE_BYTE_ORDER;
    pXWDHeader->bitmap_unit = BITMAP_SCANLINE_UNIT;
    pXWDHeader->bitmap_bit_order = BITMAP_BIT_ORDER;
    pXWDHeader->bitmap_pad = BITMAP_SCANLINE_PAD;
    pXWDHeader->bits_per_pixel = pffb->bitsPerPixel;
    pXWDHeader->bytes_per_line = pffb->paddedWidth;
    pXWDHeader->ncolors = pffb->ncolors;

    /* visual related fields are written when colormap is installed */

    pXWDHeader->window_x = pXWDHeader->window_y = 0;
    pXWDHeader->window_bdrwidth = 0;

    /* write xwd "window" name: Xffb hostname:server.screen */

    if (-1 == gethostname(hostname, sizeof(hostname)))
	hostname[0] = 0;
    else
	hostname[XWD_WINDOW_NAME_LEN-1] = 0;
    sprintf((char *)(pXWDHeader+1), "Xffb %s:%s.%d", hostname, display,
	    pScreen->myNum);

    /* write colormap pixel slot values */

    for (i = 0; i < pffb->ncolors; i++)
    {
	pffb->pXWDCmap[i].pixel = i;
	/* XXX direct/true color different? */
    }

    /* byte swap to most significant byte first */

    if (needswap)
    {
	SwapLongs((CARD32 *)pXWDHeader, SIZEOF(XWDheader)/4);
	for (i = 0; i < pffb->ncolors; i++)
	{
	    register char n;
	    swapl(&pffb->pXWDCmap[i].pixel, n);
	}
    }
}


static Bool
ffbCursorOffScreen (ppScreen, x, y)
    ScreenPtr   *ppScreen;
    int         *x, *y;
{
    return FALSE;
}

static void
ffbCrossScreen (pScreen, entering)
    ScreenPtr   pScreen;
    Bool        entering;
{
}

miPointerScreenFuncRec ffbPointerCursorFuncs =
{
    ffbCursorOffScreen,
    ffbCrossScreen,
    miPointerWarpCursor
};

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
    pbits = ffbAllocateFramebufferMemory(pffb);
    if (!pbits) return FALSE;

    switch (pffb->bitsPerPixel)
    {
    case 1:
	ret = mfbScreenInit(pScreen, pbits, pffb->width, pffb->height,
			    dpix, dpiy, pffb->paddedWidth * 8);
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

    if (!ret) return FALSE;

    pScreen->CreateGC = ffbMultiDepthCreateGC;
    pScreen->GetImage = ffbMultiDepthGetImage;
    pScreen->GetSpans = ffbMultiDepthGetSpans;

    pScreen->InstallColormap = ffbInstallColormap;
    pScreen->UninstallColormap = ffbUninstallColormap;
    pScreen->ListInstalledColormaps = ffbListInstalledColormaps;

    pScreen->SaveScreen = ffbSaveScreen;
    pScreen->StoreColors = ffbStoreColors;

    miDCInitialize(pScreen, &ffbPointerCursorFuncs);

    ffbWriteXWDFileHeader(pScreen);

    if (pffb->bitsPerPixel == 1)
    {
	/* XXX probably should have options for these? */
	pScreen->blackPixel = 0;
	pScreen->whitePixel = 1;
	ret = mfbCreateDefColormap(pScreen);
    }
    else
    {
	ret = cfbCreateDefColormap(pScreen);
    }

    return ret;

} /* end ffbScreenInit */


void
InitOutput(screenInfo, argc, argv)
    ScreenInfo *screenInfo;
    int argc;
    char **argv;
{
    int i;
    int NumFormats = 0;
    FILE *pf = stderr;

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

} /* end InitOutput */

