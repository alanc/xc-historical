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
/* $XConsortium: main.c,v 1.156 89/04/07 16:54:05 rws Exp $ */

#include "X.h"
#include "Xproto.h"
#include "input.h"
#include "scrnintstr.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "extension.h"
#include "colormap.h"
#include "cursorstr.h"
#include "opaque.h"
#include "servermd.h"

extern long defaultScreenSaverTime;
extern long defaultScreenSaverInterval;
extern int defaultScreenSaverBlanking;
extern int defaultScreenSaverAllowExposures;

extern Bool GivingUp;
void ddxGiveUp();

extern char *display;
char *ConnectionInfo;
xConnSetupPrefix connSetupPrefix;

extern WindowPtr *WindowTable;
extern FontPtr defaultFont;

extern void SetInputCheck();
extern void InitProcVectors();
extern void InitEvents();
extern void InitExtensions();
extern void DefineInitialRootWindow();
extern void QueryMinMaxKeyCodes();
extern Bool InitClientResources();
static Bool CreateConnectionBlock();
extern Bool CreateGCperDepthArray();
extern Bool CreateDefaultStipple();
extern void ResetWellKnownSockets();

PaddingInfo PixmapWidthPaddingInfo[33];
int connBlockScreenStart;

static int restart = 0;

void
NotImplemented()
{
    FatalError("Not implemented");
}

/*
 * This array encodes the answer to the question "what is the log base 2
 * of the number of pixels that fit in a scanline pad unit?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int answer[6][3] = {
	/* pad   pad   pad */
	/*  8     16    32 */

	{   3,     4,    5 },	/* 1 bit per pixel */
	{   1,     2,    3 },	/* 4 bits per pixel */
	{   0,     1,    2 },	/* 8 bits per pixel */
	{   ~0,    0,    1 },	/* 16 bits per pixel */
	{   ~0,    ~0,   0 },	/* 24 bits per pixel */
	{   ~0,    ~0,   0 }	/* 32 bits per pixel */
};

/*
 * This array gives the answer to the question "what is the first index for
 * the answer array above given the number of bits per pixel?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int indexForBitsPerPixel[ 33 ] = {
	~0, 0, ~0, ~0,	/* 1 bit per pixel */
	1, ~0, ~0, ~0,	/* 4 bits per pixel */
	2, ~0, ~0, ~0,	/* 8 bits per pixel */
	~0,~0, ~0, ~0,
	3, ~0, ~0, ~0,	/* 16 bits per pixel */
	~0,~0, ~0, ~0,
	4, ~0, ~0, ~0,	/* 24 bits per pixel */
	~0,~0, ~0, ~0,
	5		/* 32 bits per pixel */
};

/*
 * This array gives the answer to the question "what is the second index for
 * the answer array above given the number of bits per scanline pad unit?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int indexForScanlinePad[ 33 ] = {
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	0,  ~0, ~0, ~0,	/* 8 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	1,  ~0, ~0, ~0,	/* 16 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	2		/* 32 bits per scanline pad unit */
};

main(argc, argv)
    int		argc;
    char	*argv[];
{
    int		i, j, k, looping;
    long	alwaysCheckForInput[2];

    /* Notice if we're restart.  Probably this is because we jumped through
     * uninitialized pointer */
    if (restart)
	FatalError("server restarted. Jumped through uninitialized pointer?\n");
    else
	restart = 1;
    /* These are needed by some routines which are called from interrupt
     * handlers, thus have no direct calling path back to main and thus
     * can't be passed argc, argv as parameters */
    argcGlobal = argc;
    argvGlobal = argv;
    display = "0";
    ProcessCommandLine(argc, argv);

    alwaysCheckForInput[0] = 0;
    alwaysCheckForInput[1] = 1;
    looping = 0;
    while(1)
    {
        ScreenSaverTime = defaultScreenSaverTime;
	ScreenSaverInterval = defaultScreenSaverInterval;
	ScreenSaverBlanking = defaultScreenSaverBlanking;
	ScreenSaverAllowExposures = defaultScreenSaverAllowExposures;

	/* Perform any operating system dependent initializations you'd like */
	OsInit();		
	if(!looping)
	{
	    CreateWellKnownSockets();
	    InitProcVectors();
	    clients = (ClientPtr *)xalloc(MAXCLIENTS * sizeof(ClientPtr));
	    if (!clients)
		FatalError("couldn't create client array");
	    for (i=1; i<MAXCLIENTS; i++) 
		clients[i] = NullClient;
	    serverClient = (ClientPtr)xalloc(sizeof(ClientRec));
	    if (!serverClient)
		FatalError("couldn't create server client");
            serverClient->sequence = 0;
            serverClient->closeDownMode = RetainPermanent;
            serverClient->clientGone = FALSE;
            serverClient->lastDrawable = (DrawablePtr)NULL;
	    serverClient->lastDrawableID = INVALID;
            serverClient->lastGC = (GCPtr)NULL;
	    serverClient->lastGCID = INVALID;
	    serverClient->numSaved = 0;
	    serverClient->saveSet = (pointer *)NULL;
	    serverClient->index = 0;
	    serverClient->clientAsMask = (Mask)0;
	}
        clients[0] = serverClient;
        currentMaxClients = 1;

	if (!InitClientResources(serverClient))      /* for root resources */
	    FatalError("couldn't init server resources");

	SetInputCheck(&alwaysCheckForInput[0], &alwaysCheckForInput[1]);
	screenInfo.arraySize = MAXSCREENS;
	screenInfo.numScreens = 0;
	WindowTable = (WindowPtr *)xalloc(MAXSCREENS * sizeof(WindowPtr));
	if (!WindowTable)
	    FatalError("couldn't create root window table");

	/*
	 * Just in case the ddx doesnt supply a format for depth 1 (like qvss).
	 */
	j = indexForBitsPerPixel[ 1 ];
	k = indexForScanlinePad[ BITMAP_SCANLINE_PAD ];
	PixmapWidthPaddingInfo[1].padRoundUp = BITMAP_SCANLINE_PAD-1;
	PixmapWidthPaddingInfo[1].padPixelsLog2 = answer[j][k];
 	j = indexForBitsPerPixel[8]; /* bits per byte */
 	PixmapWidthPaddingInfo[1].padBytesLog2 = answer[j][k];

	InitAtoms();
	InitEvents();
	InitOutput(&screenInfo, argc, argv);
	if (screenInfo.numScreens < 1)
	    FatalError("no screens found");
	InitExtensions(argc, argv);
	InitInput(argc, argv);
	if (InitAndStartDevices(argc, argv) != Success)
	    FatalError("failed to initialize core devices");

	if (SetDefaultFontPath(defaultFontPath) != Success)
	    ErrorF("failed to set default font path\n");
	if (!SetDefaultFont(defaultTextFont))
	    FatalError("could not open default font '%s'", defaultTextFont);
	if (!(rootCursor = CreateRootCursor(defaultCursorFont, 0)))
	    FatalError("could not open default cursor font '%s'",
		       defaultCursorFont);
	for (i = 0; i < screenInfo.numScreens; i++)
	    InitRootWindow(WindowTable[i]);
        DefineInitialRootWindow(WindowTable[0]);

	if (!CreateConnectionBlock())
	    FatalError("could not create connection block info");

	Dispatch();

	/* Now free up whatever must be freed */
	CloseDownExtensions();
	FreeAllResources();
	CloseDownDevices(argc, argv);
	for (i = screenInfo.numScreens - 1; i >= 0; i--)
	{
	    FreeGCperDepth(i);
	    FreeDefaultStipple(i);
	    (* screenInfo.screens[i]->CloseScreen)(i, screenInfo.screens[i]);
	    xfree(screenInfo.screens[i]);
	    screenInfo.numScreens = i;
	}
	xfree(WindowTable);

        CloseFont(defaultFont, (Font)0);
        defaultFont = (FontPtr)NULL;

	if (GivingUp)
	{
	    ddxGiveUp();
	    break;
	}

	ResetHosts(display);
	xfree(ConnectionInfo);

	ResetWellKnownSockets ();
	looping = 1;
    }
    exit(0);
}

static int padlength[4] = {0, 3, 2, 1};

static Bool
CreateConnectionBlock()
{
    xConnSetup setup;
    xWindowRoot root;
    xDepth	depth;
    xVisualType visual;
    xPixmapFormat format;
    unsigned long vid;
    int i, j, k,
        lenofblock,
        sizesofar = 0;
    char *pBuf;

    
    /* Leave off the ridBase and ridMask, these must be sent with 
       connection */

    setup.release = VENDOR_RELEASE;
    /*
     * per-server image and bitmap parameters are defined in Xmd.h
     */
    setup.imageByteOrder = screenInfo.imageByteOrder;
    setup.bitmapScanlineUnit  = screenInfo.bitmapScanlineUnit;
    setup.bitmapScanlinePad = screenInfo.bitmapScanlinePad;
    setup.bitmapBitOrder = screenInfo.bitmapBitOrder;
    setup.motionBufferSize = NumMotionEvents();
    setup.numRoots = screenInfo.numScreens;
    setup.nbytesVendor = strlen(VENDOR_STRING); 
    setup.numFormats = screenInfo.numPixmapFormats;
    setup.maxRequestSize = MAX_REQUEST_SIZE;
    QueryMinMaxKeyCodes(&setup.minKeyCode, &setup.maxKeyCode);
    
    lenofblock = sizeof(xConnSetup) + 
            ((setup.nbytesVendor + 3) & ~3) +
	    (setup.numFormats * sizeof(xPixmapFormat)) +
            (setup.numRoots * sizeof(xWindowRoot));
    ConnectionInfo = (char *) xalloc(lenofblock);
    if (!ConnectionInfo)
	return FALSE;

    bcopy((char *)&setup, ConnectionInfo, sizeof(xConnSetup));
    sizesofar = sizeof(xConnSetup);
    pBuf = ConnectionInfo + sizeof(xConnSetup);

    bcopy(VENDOR_STRING, pBuf, (int)setup.nbytesVendor);
    sizesofar += setup.nbytesVendor;
    pBuf += setup.nbytesVendor;
    i = padlength[setup.nbytesVendor & 3];
    sizesofar += i;
    while (--i >= 0)
        *pBuf++ = 0;
    
    for (i=0; i<screenInfo.numPixmapFormats; i++)
    {
	format.depth = screenInfo.formats[i].depth;
	format.bitsPerPixel = screenInfo.formats[i].bitsPerPixel;
	format.scanLinePad = screenInfo.formats[i].scanlinePad;
	bcopy((char *)&format, pBuf, sizeof(xPixmapFormat));
	pBuf += sizeof(xPixmapFormat);
	sizesofar += sizeof(xPixmapFormat);
    }

    connBlockScreenStart = sizesofar;
    for (i=0; i<screenInfo.numScreens; i++) 
    {
	ScreenPtr	pScreen;
	DepthPtr	pDepth;
	VisualPtr	pVisual;

	pScreen = screenInfo.screens[i];
	root.windowId = WindowTable[i]->wid;
	root.defaultColormap = pScreen->defColormap;
	root.whitePixel = pScreen->whitePixel;
	root.blackPixel = pScreen->blackPixel;
	root.currentInputMask = 0;    /* filled in when sent */
	root.pixWidth = pScreen->width;
	root.pixHeight = pScreen->height;
	root.mmWidth = pScreen->mmWidth;
	root.mmHeight = pScreen->mmHeight;
	root.minInstalledMaps = pScreen->minInstalledCmaps;
	root.maxInstalledMaps = pScreen->maxInstalledCmaps; 
	root.rootVisualID = pScreen->rootVisual;		
	root.backingStore = pScreen->backingStoreSupport;
	root.saveUnders = pScreen->saveUnderSupport != NotUseful;
	root.rootDepth = pScreen->rootDepth;
	root.nDepths = pScreen->numDepths;
	bcopy((char *)&root, pBuf, sizeof(xWindowRoot));
	sizesofar += sizeof(xWindowRoot);
	pBuf += sizeof(xWindowRoot);

	pDepth = pScreen->allowedDepths;
	for(j = 0; j < pScreen->numDepths; j++, pDepth++)
	{
	    lenofblock += sizeof(xDepth) + 
		    (pDepth->numVids * sizeof(xVisualType));
	    pBuf = (char *)xrealloc(ConnectionInfo, lenofblock);
	    if (!pBuf)
	    {
		xfree(ConnectionInfo);
		return FALSE;
	    }
	    ConnectionInfo = pBuf;
	    pBuf += sizesofar;            
	    depth.depth = pDepth->depth;
	    depth.nVisuals = pDepth->numVids;
	    bcopy((char *)&depth, pBuf, sizeof(xDepth));
	    pBuf += sizeof(xDepth);
	    sizesofar += sizeof(xDepth);
	    for(k = 0; k < pDepth->numVids; k++)
	    {
		vid = pDepth->vids[k];
		pVisual = (VisualPtr) LookupID(vid, RT_VISUALID, RC_CORE);
		visual.visualID = pVisual->vid;
		visual.class = pVisual->class;
		visual.bitsPerRGB = pVisual->bitsPerRGBValue;
		visual.colormapEntries = pVisual->ColormapEntries;
		visual.redMask = pVisual->redMask;
		visual.greenMask = pVisual->greenMask;
		visual.blueMask = pVisual->blueMask;
		bcopy((char *)&visual, pBuf, sizeof(xVisualType));
		pBuf += sizeof(xVisualType);
		sizesofar += sizeof(xVisualType);
	    }
	}
    }
    connSetupPrefix.success = xTrue;
    connSetupPrefix.length = lenofblock/4;
    connSetupPrefix.majorVersion = X_PROTOCOL;
    connSetupPrefix.minorVersion = X_PROTOCOL_REVISION;
    return TRUE;
}

/*
	grow the array of screenRecs if necessary.
	call the device-supplied initialization procedure 
with its screen number, a pointer to its ScreenRec, argc, and argv.
	return the number of successfully installed screens.

*/

int
AddScreen(pfnInit, argc, argv)
    Bool	(* pfnInit)();
    int argc;
    char **argv;
{

    int i;
    int scanlinepad, format, depth, bitsPerPixel, j, k;
    ScreenPtr pScreen;
#ifdef DEBUG
    void	(**jNI) ();
#endif /* DEBUG */

    i = screenInfo.numScreens;
    if (i == MAXSCREENS)
	return -1;

    pScreen = (ScreenPtr) xalloc(sizeof(ScreenRec));
    if (!pScreen)
	return -1;

#ifdef DEBUG
    for (jNI = &pScreen->QueryBestSize; 
	 jNI < (void (**) ()) &pScreen->SendGraphicsExpose;
	 jNI++)
	*jNI = NotImplemented;
#endif /* DEBUG */

    /*
     * This loop gets run once for every Screen that gets added,
     * but thats ok.  If the ddx layer initializes the formats
     * one at a time calling AddScreen() after each, then each
     * iteration will make it a little more accurate.  Worst case
     * we do this loop N * numPixmapFormats where N is # of screens.
     * Anyway, this must be called after InitOutput and before the
     * screen init routine is called.
     */
    for (format=0; format<screenInfo.numPixmapFormats; format++)
    {
 	depth = screenInfo.formats[format].depth;
 	bitsPerPixel = screenInfo.formats[format].bitsPerPixel;
  	scanlinepad = screenInfo.formats[format].scanlinePad;
 	j = indexForBitsPerPixel[ bitsPerPixel ];
  	k = indexForScanlinePad[ scanlinepad ];
 	PixmapWidthPaddingInfo[ depth ].padPixelsLog2 = answer[j][k];
 	PixmapWidthPaddingInfo[ depth ].padRoundUp =
 	    (scanlinepad/bitsPerPixel) - 1;
 	j = indexForBitsPerPixel[ 8 ]; /* bits per byte */
 	PixmapWidthPaddingInfo[ depth ].padBytesLog2 = answer[j][k];
    }
  
    /* This is where screen specific stuff gets initialized.  Load the
       screen structure, call the hardware, whatever.
       This is also where the default colormap should be allocated and
       also pixel values for blackPixel, whitePixel, and the cursor
       Note that InitScreen is NOT allowed to modify argc, argv, or
       any of the strings pointed to by argv.  They may be passed to
       multiple screens. 
    */ 
    pScreen->rgf = ~0L;  /* there are no scratch GCs yet*/
    pScreen->myNum = i;
    WindowTable[i] = NullWindow;
    screenInfo.screens[i] = pScreen;
    screenInfo.numScreens++;
    if ((*pfnInit)(i, pScreen, argc, argv) &&
	CreateRootWindow(i))
    {
	if (CreateGCperDepth(i))
	{
	    if (CreateDefaultStipple(i))
		return screenInfo.numScreens;
	    FreeGCperDepth(i);
	}
	FreeResource(WindowTable[i]->wid, RC_NONE);
    }
    xfree(pScreen);
    screenInfo.numScreens--;
    return -1;
}


