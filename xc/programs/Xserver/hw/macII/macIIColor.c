/*-
 * macIICG4C.c --
 *	Functions to support the macII CG4 board as a memory frame buffer.
 */

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

#ifndef	lint
static char sccsid[] = "@(#)macIICG4C.c	1.4 6/1/87 Copyright 1987 Sun Micro";
#endif

#include    "macII.h"

#include    "colormap.h"
#include    "colormapst.h"
#include    "resource.h"

/* XXX - next line means only one CG4 - fix this */
static ColormapPtr macIICG4CInstalledMap;

extern int TellLostMap(), TellGainedMap();

extern int 	consoleFd;

struct {
	u_char red;
	u_char green;
	u_char blue;
} clut_shadow[256];

static void
macIICG4CUpdateColormap(pScreen, index, count, rmap, gmap, bmap)
    ScreenPtr	pScreen;
    int		index, count;
    u_char	*rmap, *gmap, *bmap;
{

	while (count--) {
		clut_shadow[index].red = rmap[index];
		clut_shadow[index].green = gmap[index];
		clut_shadow[index].blue =  bmap[index];
		index++;
	}
	{
		int fd;
		struct strioctl ctl; /* Streams ioctl control structure */

		if (consoleFd <= 0) {
			fd = open("/dev/console",O_RDWR);
		} else {
			fd = consoleFd;
		}
		if (fd <= 0) FatalError("Open Failed for VIDEO_SETCLUT. \n");

    	        ctl.ic_cmd = VIDEO_SETCLUT;
		ctl.ic_timout = -1;
		ctl.ic_len = 3 * 256;
		ctl.ic_dp = &(clut_shadow[0]);
    		if (ioctl(fd, I_STR, &ctl) == -1) {
		        FatalError ("ioctl I_STR VIDEO_SETCLUT failed");
			(void) close (fd);
			return;
		}

		if (consoleFd <=0) close(fd);

	}

}

/*-
 *-----------------------------------------------------------------------
 * macIICG4CSaveScreen --
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
macIICG4CSaveScreen (pScreen, on)
    ScreenPtr	  pScreen;
    Bool    	  on;
{
    int		state = on;

    if (on != SCREEN_SAVER_ON) {
      SetTimeSinceLastInputEvent();
      state = 1;
    } else {
      state = 0;
    }
/* XXX fd is closed??!! XXX */
/*
    (void) ioctl(macIIFbs[pScreen->myNum].fd, FBIOSVIDEO, &state);
*/
    return( TRUE );
}

/*-
 *-----------------------------------------------------------------------
 * macIICG4CCloseScreen --
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
static Bool
macIICG4CCloseScreen(i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    macIICG4CInstalledMap = NULL;
    return (pScreen->SaveScreen(pScreen, SCREEN_SAVER_OFF));
}

/*-
 *-----------------------------------------------------------------------
 * macIICG4CInstallColormap --
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
macIICG4CInstallColormap(cmap)
    ColormapPtr	cmap;
{
    register int i;
    register Entry *pent = cmap->red;
    u_char	  rmap[256], gmap[256], bmap[256];

    if (cmap == macIICG4CInstalledMap)
	return;
    if (macIICG4CInstalledMap)
	WalkTree(macIICG4CInstalledMap->pScreen, TellLostMap,
		 (char *) &(macIICG4CInstalledMap->mid));
    for (i = 0; i < cmap->pVisual->ColormapEntries; i++) {
	if (pent->fShared) {
	    rmap[i] = pent->co.shco.red->color >> 8;
	    gmap[i] = pent->co.shco.green->color >> 8;
	    bmap[i] = pent->co.shco.blue->color >> 8;
	}
	else {
	    rmap[i] = pent->co.local.red >> 8;
	    gmap[i] = pent->co.local.green >> 8;
	    bmap[i] = pent->co.local.blue >> 8;
	}
	pent++;
    }
    macIICG4CInstalledMap = cmap;
    macIICG4CUpdateColormap(cmap->pScreen, 0, 256, rmap, gmap, bmap);
    WalkTree(cmap->pScreen, TellGainedMap, (char *) &(cmap->mid));
}

/*-
 *-----------------------------------------------------------------------
 * macIICG4CUninstallColormap --
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
macIICG4CUninstallColormap(cmap)
    ColormapPtr	cmap;
{
    if (cmap == macIICG4CInstalledMap) {
	Colormap defMapID = cmap->pScreen->defColormap;

	if (cmap->mid != defMapID) {
	    ColormapPtr defMap = (ColormapPtr) LookupID(defMapID, RT_COLORMAP, RC_CORE);

	    if (defMap)
		macIICG4CInstallColormap(defMap);
	    else
	        ErrorF("macIICG4C: Can't find default colormap\n");
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIICG4CListInstalledColormaps --
 *	Fills in the list with th  IDs of the installed maps
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
macIICG4CListInstalledColormaps(pScreen, pCmapList)
    ScreenPtr	pScreen;
    Colormap	*pCmapList;
{
    *pCmapList = macIICG4CInstalledMap->mid;
    return (1);
}


/*-
 *-----------------------------------------------------------------------
 * macIICG4CStoreColors --
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
macIICG4CStoreColors(pmap, ndef, pdefs)
    ColormapPtr	pmap;
    int		ndef;
    xColorItem	*pdefs;
{
    switch (pmap->class) {
    case PseudoColor:
	if (pmap == macIICG4CInstalledMap) {
	    /* We only have a single colormap */
	    u_char	rmap[256], gmap[256], bmap[256];

	    while (ndef--) {
		register unsigned index = pdefs->pixel&0xff;

		/* PUTCMAP assumes colors to be assigned start at 0 */
		rmap[index] = (pdefs->red) >> 8;
		gmap[index] = (pdefs->green) >> 8;
		bmap[index] = (pdefs->blue) >> 8;
	 	macIICG4CUpdateColormap(pmap->pScreen,
				      index, 1, rmap, gmap, bmap);
		pdefs++;
	    }
	}
	break;
    case DirectColor:
    default:
	ErrorF("macIICG4CStoreColors: bad class %d\n", pmap->class);
	break;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIICG4CResolvePseudoColor --
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
macIICG4CResolvePseudoColor(pRed, pGreen, pBlue, pVisual)
    CARD16	*pRed, *pGreen, *pBlue;
    VisualPtr	pVisual;
{
    *pRed &= 0xff00;
    *pGreen &= 0xff00;
    *pBlue &= 0xff00;
}

/*-
 *-----------------------------------------------------------------------
 * macIICG4CInit --
 *	Attempt to find and initialize a cg4 framebuffer 
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
extern int cfbResolveStaticColor();
/*ARGSUSED*/
static Bool
macIICG4CInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    PixmapPtr   pPixmap;
    CARD16	zero = 0, ones = ~0;

    /*
     * Dots per inch {x,y} is 68 for color screen.
     */
    if (!cfbScreenInit (index, pScreen, 
			    macIIFbs[index].fb,
			    macIIFbs[index].info.v_right -
			    macIIFbs[index].info.v_left + 1,
			    macIIFbs[index].info.v_bottom -
		            macIIFbs[index].info.v_top + 1,
			    macIIFbs[index].info.v_hres))
	return (FALSE);

    /* macII screens may have extra video memory to the right of the visible
     * area, therefore the PixmapBytePad macro in cfbScreenInit gave the
     * wrong value to the devKind field of the Pixmap it made for the screen.
     * So we fix it here. */

    pPixmap = (PixmapPtr)(pScreen->devPrivate);
    pPixmap->devKind =  macIIFbs[index].info.v_rowbytes; 

    pScreen->SaveScreen =   	    	macIICG4CSaveScreen;
    pScreen->RecolorCursor = 	    	macIIRecolorCursor;

#ifndef STATIC_COLOR
    pScreen->InstallColormap = macIICG4CInstallColormap;
    pScreen->UninstallColormap = macIICG4CUninstallColormap;
    pScreen->ListInstalledColormaps = macIICG4CListInstalledColormaps;
    pScreen->StoreColors = macIICG4CStoreColors;
    pScreen->ResolveColor = macIICG4CResolvePseudoColor;
#endif

    {
	ColormapPtr cmap = (ColormapPtr)LookupID(pScreen->defColormap, RT_COLORMAP, RC_CORE);

	if (!cmap)
	    FatalError("Can't find default colormap\n");
	if (AllocColor(cmap, &ones, &ones, &ones, &(pScreen->whitePixel), 0)
	    || AllocColor(cmap, &zero, &zero, &zero, &(pScreen->blackPixel), 0))
		FatalError("Can't alloc black & white pixels in cfbScreeninit\n");
	macIICG4CInstallColormap(cmap);
    }


    macIICG4CSaveScreen( pScreen, SCREEN_SAVER_FORCER );
    macIIScreenInit (pScreen);
    return (TRUE);
}

/*-
 *--------------------------------------------------------------
 * macIICG4CSwitch --
 *      Enable or disable color plane 
 *
 * Results:
 *      Color plane enabled for select =0, disabled otherwise.
 *
 *--------------------------------------------------------------
 */
static void
macIICG4CSwitch (pScreen, select)
   ScreenPtr  pScreen;
   u_char     select;
{
}

/*-
 *-----------------------------------------------------------------------
 * macIICG4CProbe --
 *	Attempt to find and initialize a color framebuffer. 
 *
 * Results:
 *	TRUE if everything went ok. FALSE if not.
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped.
 *
 *-----------------------------------------------------------------------
 */
Bool
macIICG4CProbe (pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the macIIFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         i, oldNumScreens;

    if (macIIFbData[fbNum].probeStatus == probedAndFailed) {
	return FALSE;
    }

    if (macIIFbData[fbNum].probeStatus == neverProbed) {
	int         fd;
	fbtype fbType;

	if ((fd = macIIOpenFrameBuffer(FBTYPE_MACII, &fbType, index, fbNum,
				     argc, argv)) < 0) {
	    macIIFbData[fbNum].probeStatus = probedAndFailed;
	    return FALSE;
	}


	{
		static char *video_virtaddr = 0x0;
		struct video_map vmap;
		struct strioctl ctl; /* Streams ioctl control structure */

		/* map to next 8MB segment boundary */
		video_virtaddr = video_virtaddr + (8 * 1024 * 1024); 
	        vmap.map_physnum = 0;
        	vmap.map_virtaddr = video_virtaddr;

		ctl.ic_cmd = VIDEO_MAP;
		ctl.ic_timout = -1;
		ctl.ic_len = sizeof(vmap);
		ctl.ic_dp = (char *)&vmap;
		if (ioctl(fd, I_STR, &ctl) == -1) {
			FatalError ("ioctl I_STR VIDEO_MAP failed");
			(void) close (fd);
			return (FALSE);
		}

    		macIIFbs[index].fb = 
		    (pointer)(video_virtaddr + fbType.v_baseoffset); 

		ctl.ic_cmd = VIDEO_SETCOLOR;
		ctl.ic_timout = -1;
		ctl.ic_len = sizeof(video_mode);
		ctl.ic_dp = (char *)&video_mode;
		video_mode = NGC_COLOR;
		if (ioctl(fd, I_STR, &ctl) == -1) {
			FatalError ("ioctl I_STR VIDEO_SETCOLOR failed");
			(void) close (fd);
			return (FALSE);
		}

		ctl.ic_cmd = VIDEO_SETMODE;
		ctl.ic_timout = -1;
		ctl.ic_len = sizeof(video_mode);
		ctl.ic_dp = (char *)&video_mode;
		video_mode = NGC_8NI;
		if (ioctl(fd, I_STR, &ctl) == -1) {
			FatalError ("ioctl I_STR VIDEO_SETMODE failed");
			(void) close (fd);
			return (FALSE);
		}

		(void) close(fd);
	}
	macIIFbs[index].fd = fd; /* This fd has been closed! XXX */
	macIIFbs[index].info = fbType;
        macIIFbs[index].EnterLeave = macIICG4CSwitch;
	macIIFbData[fbNum].probeStatus = probedAndSucceeded;

    }

    /*
     * If we've ever successfully probed this device, do the following. 
     */

    oldNumScreens = pScreenInfo->numScreens;
    i = AddScreen(macIICG4CInit, argc, argv);
    pScreenInfo->screen[index].CloseScreen = macIICG4CCloseScreen;
    /* Now set the enable plane for color */
    if (index == 0) macIICG4CSwitch (&(pScreenInfo->screen[0]), 0);

    return (i > oldNumScreens);
}
