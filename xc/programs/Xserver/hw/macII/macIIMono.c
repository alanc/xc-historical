/*-
 * macIIBW2.c --
 *	Functions for handling the macII video board with 1 bit/pixel.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 * Copyright (c) 1987 by Adam de Boor, UC Berkeley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
 */

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


#ifndef	lint
static char sccsid[] = "%W %G Copyright 1987 Sun Micro";
#endif

/*-
 * Copyright (c) 1987 by Sun Microsystems,  Inc.
 */

#include    "macII.h"
#include    "resource.h"



/*-
 *-----------------------------------------------------------------------
 * macIIBW2SaveScreen --
 *	Disable the video on the frame buffer to save the screen.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Video enable state changes.
 *
 *-----------------------------------------------------------------------
 */
static Bool
macIIBW2SaveScreen (pScreen, on)
    ScreenPtr	  pScreen;
    Bool    	  on;
{
    int         state = on;
    if (on != SCREEN_SAVER_ON) {
      SetTimeSinceLastInputEvent();
/*	state = FBVIDEO_ON; */
    } else {
/*	state = FBVIDEO_OFF; */
    }
/*    (void) ioctl(macIIFbs[pScreen->myNum].fd, FBIOSVIDEO, &state); */
    return TRUE;
}

/*-
 *-----------------------------------------------------------------------
 * macIIBW2CloseScreen --
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
macIIBW2CloseScreen(i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    return (pScreen->SaveScreen(pScreen, SCREEN_SAVER_OFF));
}

/*-
 *-----------------------------------------------------------------------
 * macIIBW2ResolveColor --
 *	Resolve an RGB value into some sort of thing we can handle.
 *	Just looks to see if the intensity of the color is greater than
 *	1/2 and sets it to 'white' (all ones) if so and 'black' (all zeroes)
 *	if not.
 *
 * Results:
 *	*pred, *pgreen and *pblue are overwritten with the resolved color.
 *
 * Side Effects:
 *	see above.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
macIIBW2ResolveColor(pred, pgreen, pblue, pVisual)
    unsigned short	*pred;
    unsigned short	*pgreen;
    unsigned short	*pblue;
    VisualPtr		pVisual;
{
    /* 
     * Gets intensity from RGB.  If intensity is >= half, pick white, else
     * pick black.  This may well be more trouble than it's worth.
     */

    *pred = *pgreen = *pblue = 
        (((39L * (long)*pred +
           50L * (long)*pgreen +
           11L * (long)*pblue) >> 8) >= (((1<<8)-1)*50)) ? ~0 : 0;
    
}

/*-
 *-----------------------------------------------------------------------
 * macIIBW2CreateColormap --
 *	create a bw colormap
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	allocate two pixels
 *
 *-----------------------------------------------------------------------
 */
void
macIIBW2CreateColormap(pmap)
    ColormapPtr	pmap;
{
    int	red, green, blue, pix;

    /* this is a monochrome colormap, it only has two entries, just fill
     * them in by hand.  If it were a more complex static map, it would be
     * worth writing a for loop or three to initialize it */

    /* this will be pixel 0 */
    red = green = blue = ~0;
    AllocColor(pmap, &red, &green, &blue, &pix, 0);

    /* this will be pixel 1 */
    red = green = blue = 0;
    AllocColor(pmap, &red, &green, &blue, &pix, 0);

}

/*-
 *-----------------------------------------------------------------------
 * macIIBW2DestroyColormap --
 *	destroy a bw colormap
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
void
macIIBW2DestroyColormap(pmap)
    ColormapPtr	pmap;
{
}

/*-
 *-----------------------------------------------------------------------
 * macIIBW2Init --
 *	Initialize the macII framebuffer
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Most of the elements of the ScreenRec are filled in.  The
 *	video is enabled for the frame buffer...
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static Bool
macIIBW2Init (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    ColormapPtr pColormap;
    PixmapPtr   pPixmap;

    /*
     * Dots per inch {x,y} is 76 on the monochrome screen, less on color!
     */
    if (!mfbScreenInit(index, pScreen,
			   macIIFbs[index].fb,
			   macIIFbs[index].info.fb_width,
			   macIIFbs[index].info.fb_height, 76, 76))
	return (FALSE);

    /* macII screens may have extra video memory to the right of the visible
     * area, therefore the PixmapBytePad macro in mfbScreenInit gave the 
     * wrong value to the devKind field of the Pixmap it made for the screen.
     * So we fix it here. */

    pPixmap = (PixmapPtr)(pScreen->devPrivate);
    pPixmap->devKind =  macIIFbs[index].info.fb_pitch >> 3; /* bytes per scan line */

    pScreen->SaveScreen = macIIBW2SaveScreen;
    pScreen->ResolveColor = macIIBW2ResolveColor;
    pScreen->CreateColormap = macIIBW2CreateColormap;
    pScreen->DestroyColormap = macIIBW2DestroyColormap;
    pScreen->whitePixel = 0;
    pScreen->blackPixel = 1;

#ifdef ZOIDS
    {
	GCPtr	pGC = CreateScratchGC(pScreen, 1);

	if (pGC) {
	    RegisterProc("PolySolidXAlignedTrapezoid", pGC,
			  macIIBW2SolidXZoids);
	    RegisterProc("PolySolidYAlignedTrapezoid", pGC,
			  macIIBW2SolidYZoids);
	    RegisterProc("PolyTiledXAlignedTrapezoid", pGC,
			  macIIBW2TiledXZoids);
	    RegisterProc("PolyTiledYAlignedTrapezoid", pGC,
			  macIIBW2TiledYZoids);
	    RegisterProc("PolyStipXAlignedTrapezoid", pGC,
			  macIIBW2StipXZoids);
	    RegisterProc("PolyStipYAlignedTrapezoid", pGC,
			  macIIBW2StipYZoids);
	    FreeScratchGC(pGC);
	}
    }
#endif ZOIDS

    if (CreateColormap(pScreen->defColormap, pScreen,
		   LookupID(pScreen->rootVisual, RT_VISUALID, RC_CORE),
		   &pColormap, AllocNone, 0) != Success
	|| pColormap == NULL)
	    FatalError("Can't create colormap in macIIBW2Init()\n");
    mfbInstallColormap(pColormap);

    /*
     * Enable video output...? 
     */
    (void) macIIBW2SaveScreen(pScreen, SCREEN_SAVER_FORCER);

    macIIScreenInit(pScreen);
    return (TRUE);

}

/*-
 *-----------------------------------------------------------------------
 * macIIBW2Probe --
 *	Attempt to find and initialize a macII framebuffer
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped. 
 *
 *-----------------------------------------------------------------------
 */

Bool
macIIBW2Probe(pScreenInfo, index, fbNum, argc, argv)
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
	struct fbtype fbType;

	if ((fd = macIIOpenFrameBuffer(FBTYPE_MACII, &fbType, index, fbNum,
				     argc, argv)) < 0) {
	    macIIFbData[fbNum].probeStatus = probedAndFailed;
	    return FALSE;
	}

	{
#include 	<sys/stropts.h>
#include	<sys/video.h>
		static char *video_virtaddr = 0x0;
		struct video_map vmap;
		struct video_data vdata;
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

		ctl.ic_cmd = VIDEO_DATA;
		ctl.ic_timout = -1;
		ctl.ic_len = sizeof(vdata);
		ctl.ic_dp = (char *)&vdata;
		if (ioctl(fd, I_STR, &ctl) == -1) {
			FatalError ("ioctl I_STR VIDEO_DATA failed");
			(void) close (fd);
			return (FALSE);
		}


    		macIIFbs[index].fb = 
		    (pointer)(video_virtaddr + vdata.v_baseoffset); 
		(void) close(fd);
	}

	macIIFbs[index].fd = fd; /* This fd has been closed! XXX */
	macIIFbs[index].info = fbType;
        macIIFbs[index].EnterLeave = NoopDDA;
	macIIFbData[fbNum].probeStatus = probedAndSucceeded;

    }

    /*
     * If we've ever successfully probed this device, do the following.
     */
    oldNumScreens = pScreenInfo->numScreens;
    i = AddScreen(macIIBW2Init, argc, argv);
    pScreenInfo->screen[index].CloseScreen = macIIBW2CloseScreen;
    return (i > oldNumScreens);
}

