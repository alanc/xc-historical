/*-
 * macIIColor.c --
 *	Functions to support the macII color board as a memory frame buffer.
 */

/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,p and  distribute   this
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

#include    "macII.h"

#include    "colormap.h"
#include    "colormapst.h"
#include    "resource.h"

extern int TellLostMap(), TellGainedMap();

extern int consoleFd;

extern CursorPtr currentCursor;

static struct ColorSpec {
	unsigned short value;
	unsigned short red;
	unsigned short green;
	unsigned short blue;
}; 

static void
macIIColorUpdateColormap(pScreen, index, count, pColorSpec)
    ScreenPtr	pScreen;
    int		index, count;
    struct ColorSpec	*pColorSpec;
{

	/* 
	 * Unfortunately we must slam the entire colormap into
	 * the video boards CLUT 'cause SuperMac (and others?)
	 * insist on swallowing the whole map at once. The MacOS
	 * does it this way. We require count to be 254 on entry
	 * here, the remaining two hardware CLUT entries are allocated
	 * to color the cursor.
	 */
	int fd;
	struct strioctl ctl; /* Streams ioctl control structure */
	struct CntrlParam pb;
	struct VDEntryRecord vde;

	pColorSpec[0xfe].value = 0xfe;
	pColorSpec[0xfe].red = currentCursor->foreRed;
	pColorSpec[0xfe].green = currentCursor->foreGreen;
	pColorSpec[0xfe].blue = currentCursor->foreBlue;
	count++;
	pColorSpec[0xff].value = 0xff;
	pColorSpec[0xff].red = currentCursor->backRed;
	pColorSpec[0xff].green = currentCursor->backGreen;
	pColorSpec[0xff].blue = currentCursor->backBlue;
	count++;

	if (consoleFd <= 0) {
		fd = open("/dev/console",O_RDWR);
	} else {
		fd = consoleFd;
	}
	if (fd <= 0) FatalError("Open Failed for VIDEO_CONTROL. \n");

	ctl.ic_cmd = VIDEO_CONTROL;
	ctl.ic_timout = -1;
	ctl.ic_len = sizeof(pb);
	ctl.ic_dp = (char *)&pb;

	vde.csTable = (char *) pColorSpec;
	vde.csStart = index;
	vde.csCount = count - 1;

#define noQueueBit 0x0200
#define SetEntries 0x3
	pb.qType = macIIFbs[pScreen->myNum].slot;
	pb.ioTrap = noQueueBit;
	pb.ioCmdAddr = (char *) -1;
	pb.csCode = SetEntries;
	* (char **) pb.csParam = (char *) &vde;

	if (ioctl(fd, I_STR, &ctl) == -1) {
		FatalError ("ioctl I_STR VIDEO_CONTROL failed");
		(void) close (fd);
	}

	if (consoleFd <=0) close(fd);

}

/*-
 *-----------------------------------------------------------------------
 * macIIColorSaveScreen --
 *	Preserve the color screen by turning on or off the video
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Video state is switched
 *
 *-----------------------------------------------------------------------
 */
static Bool
macIIColorSaveScreen (pScreen, on)
    ScreenPtr	  pScreen;
    Bool    	  on;
{
    return( TRUE );
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorCloseScreen --
 *	called to ensure video is enabled when server exits.
 *
 * Results:
 *	Screen is unsaved.
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
Bool
macIIColorCloseScreen(i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    extern macIIBlackScreen();

    macIIBlackScreen(pScreen->myNum);
    macIIFbs[pScreen->myNum].installedMap = NULL;
    return (pScreen->SaveScreen(pScreen, SCREEN_SAVER_OFF));
}


/*-
 *-----------------------------------------------------------------------
 * macIIColorInstallColormap --
 *	Install given colormap.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Existing map is uninstalled.
 *	All clients requesting ColormapNotify are notified
 *
 *-----------------------------------------------------------------------
 */
static void
macIIColorInstallColormap(cmap)
    ColormapPtr	cmap;
{
    register int i;
    register Entry *pent = cmap->red;
    register ColormapPtr installedMap = 
			 macIIFbs[cmap->pScreen->myNum].installedMap;
    struct ColorSpec Map[256];

    if (cmap == installedMap)
	return;
    if (installedMap)
	WalkTree(installedMap->pScreen, TellLostMap,
		 (char *) &(installedMap->mid));
    for (i = 0; i < cmap->pVisual->ColormapEntries; i++) {
	if (pent->fShared) {
	    Map[i].value = i;
	    Map[i].red = pent->co.shco.red->color;
	    Map[i].green = pent->co.shco.green->color;
	    Map[i].blue = pent->co.shco.blue->color;
	}
	else {
	    Map[i].value = i;
	    Map[i].red = pent->co.local.red;
	    Map[i].green = pent->co.local.green;
	    Map[i].blue = pent->co.local.blue;
	}
	pent++;
    }
    macIIFbs[cmap->pScreen->myNum].installedMap = cmap;
    macIIColorUpdateColormap(cmap->pScreen, 0, cmap->pVisual->ColormapEntries, 
			    Map);
    WalkTree(cmap->pScreen, TellGainedMap, (char *) &(cmap->mid));
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorUninstallColormap --
 *	Uninstall given colormap.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	default map is installed
 *	All clients requesting ColormapNotify are notified
 *
 *-----------------------------------------------------------------------
 */
static void
macIIColorUninstallColormap(cmap)
    ColormapPtr	cmap;
{
    if (cmap == macIIFbs[cmap->pScreen->myNum].installedMap) {
	Colormap defMapID = cmap->pScreen->defColormap;

	if (cmap->mid != defMapID) {
	    ColormapPtr defMap = (ColormapPtr) LookupID(defMapID, RT_COLORMAP, RC_CORE);

	    if (defMap)
		macIIColorInstallColormap(defMap);
	    else
	        ErrorF("macIIColor: Can't find default colormap\n");
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorListInstalledColormaps --
 *	Fills in the list with the IDs of the installed maps
 *
 * Results:
 *	Returns the number of IDs in the list
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
macIIColorListInstalledColormaps(pScreen, pCmapList)
    ScreenPtr	pScreen;
    Colormap	*pCmapList;
{
    *pCmapList = (macIIFbs[pScreen->myNum].installedMap)->mid;
    return (1);
}


/*-
 *-----------------------------------------------------------------------
 * macIIColorStoreColors --
 *	Sets the pixels in pdefs into the specified map.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
static void
macIIColorStoreColors(pmap, ndef, pdefs)
    ColormapPtr	pmap;
    int		ndef;
    xColorItem	*pdefs;
{
    struct ColorSpec Map[256];

    switch (pmap->class) {
    case PseudoColor:
	if (pmap == macIIFbs[pmap->pScreen->myNum].installedMap) {
	    /* We only have a single colormap */

	    register Entry *pent = pmap->red;
	    register int i;

    	    for (i = 0; i < pmap->pVisual->ColormapEntries; i++) {
	        if (pent->fShared) {
		    Map[i].value = i;
	    	    Map[i].red = pent->co.shco.red->color;
	    	    Map[i].green = pent->co.shco.green->color;
	    	    Map[i].blue = pent->co.shco.blue->color;
		    }
		else {
		    Map[i].value = i;
	    	    Map[i].red = pent->co.local.red;
	    	    Map[i].green = pent->co.local.green;
	    	    Map[i].blue = pent->co.local.blue;
		}
		pent++;
    	    }

	    while (ndef--) {
		register unsigned index = pdefs->pixel&0xff;

		/* PUTCMAP assumes colors to be assigned start at 0 */
		Map[index].value = index;
		Map[index].red = (pdefs->red);
		Map[index].green = (pdefs->green);
		Map[index].blue = (pdefs->blue);
		pdefs++;
	    }
	    macIIColorUpdateColormap(pmap->pScreen, 0, 
		                     pmap->pVisual->ColormapEntries, Map);
	}
	break;
    case DirectColor:
    default:
	ErrorF("macIIColorStoreColors: bad class %d\n", pmap->class);
	break;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorResolvePseudoColor --
 *	Adjust specified RGB values to closest values hardware can do.
 *
 * Results:
 *	Args are modified.
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
macIIColorResolvePseudoColor(pRed, pGreen, pBlue, pVisual)
    unsigned short	*pRed, *pGreen, *pBlue;
    VisualPtr	pVisual;
{
    *pRed = (*pRed & 0xff00) | (*pRed >> 8);
    *pGreen = (*pGreen & 0xff00) | (*pGreen >> 8);
    *pBlue = (*pBlue & 0xff00) | (*pBlue >> 8);
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorInit --
 *	Attempt to find and initialize a color framebuffer 
 *
 * Results:
 *	TRUE if everything went ok. FALSE if not.
 *
 * Side Effects:
 *	Most of the elements of the ScreenRec are filled in. Memory is
 *	allocated for the frame buffer and the buffer is mapped. The
 *	video is enabled for the frame buffer...
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
Bool
macIIColorInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    PixmapPtr   pPixmap;
    CARD16	zero = 0, ones = ~0;

    if (!cfbScreenInit (index, pScreen, 
			    macIIFbs[index].fb,
			    macIIFbs[index].info.v_right -
			    macIIFbs[index].info.v_left,
			    macIIFbs[index].info.v_bottom -
		            macIIFbs[index].info.v_top,
			    macIIFbs[index].info.v_hres >> 16))
	return (FALSE);

    /* macII screens may have extra video memory to the right of the visible
     * area, therefore the PixmapBytePad macro in cfbScreenInit gave the
     * wrong value to the devKind field of the Pixmap it made for the screen.
     * So we fix it here. */

    pPixmap = (PixmapPtr)(pScreen->devPrivate);
    pPixmap->devKind =  macIIFbs[index].info.v_rowbytes; 

    pScreen->SaveScreen =   	    	macIIColorSaveScreen;
    pScreen->RecolorCursor = 	    	macIIRecolorCursor;
    pScreen->InstallColormap = macIIColorInstallColormap;
    pScreen->UninstallColormap = macIIColorUninstallColormap;
    pScreen->ListInstalledColormaps = macIIColorListInstalledColormaps;
    pScreen->StoreColors = macIIColorStoreColors;
    pScreen->ResolveColor = macIIColorResolvePseudoColor;

    {
	ColormapPtr cmap = (ColormapPtr)LookupID(pScreen->defColormap, 
	RT_COLORMAP, RC_CORE);

	if (!cmap)
	    FatalError("Can't find default colormap\n");

	pScreen->whitePixel = 0xfc;
	pScreen->blackPixel = 0xfd;

	if (AllocColor(cmap, &ones, &ones, &ones, &(pScreen->whitePixel), 0)
	    || AllocColor(cmap, &zero, &zero, &zero, &(pScreen->blackPixel), 0))
		FatalError("Can't alloc black & white pixels in cfbScreeninit\n");
	macIIColorInstallColormap(cmap);
    }


    macIIColorSaveScreen( pScreen, SCREEN_SAVER_FORCER );
    macIIScreenInit (pScreen);
    return (TRUE);
}
