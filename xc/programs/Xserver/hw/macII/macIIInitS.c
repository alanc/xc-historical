/*-
 * macIIInit.c --
 *	Initialization functions for screen/keyboard/mouse, etc.
 *
 * Copyright (c) 1987 by the Regents of the University of California
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

#include    "macII.h"
#include    <servermd.h>
#include    "dixstruct.h"
#include    "dix.h"
#include    "opaque.h"

extern int macIIMouseProc();
extern void macIIKbdProc();

extern Bool macIIBW2Probe();
extern Bool macIIBW2Init();
extern Bool macIIBW2CloseScreen();

extern Bool macIICG4CInit();
extern Bool macIICG4CCloseScreen();

extern Bool macIISlotProbe();
extern void ProcessInputEvents();

extern void SetInputCheck();
extern GCPtr CreateScratchGC();

#define	XDEVICE	"XDEVICE"

int macIISigIO = 0;	 /* For use with SetInputCheck */
static int autoRepeatHandlersInstalled; /* FALSE each time InitOutput called */

	/* What should this *really* be? */
#define MOTION_BUFFER_SIZE 0

/*-
 *-----------------------------------------------------------------------
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	isItTimeToYield is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
SigIOHandler(sig, code, scp)
    int		code;
    int		sig;
    struct sigcontext *scp;
{
    macIISigIO++;
    isItTimeToYield++;
}

macIIFbDataRec macIIFbData[] = {
    macIISlotProbe,  	"slot 9",	    neverProbed,
    macIISlotProbe,  	"slot A",	    neverProbed,
    macIISlotProbe,  	"slot B",	    neverProbed,
    macIISlotProbe,  	"slot C",	    neverProbed,
    macIISlotProbe,  	"slot D",	    neverProbed,
    macIISlotProbe,  	"slot E",	    neverProbed,
    /*
     * The following entry provides support for A/UX 1.0 where no
     * slot manager calls were available. After failing to probe
     * all the slots above, InitOutput falls through to this entry
     * identifying a single monochrome screen. It must be last in
     * this table!
     */
    macIIBW2Probe, 	"/dev/console",	    neverProbed,
};

/*
 * NUMSCREENS is the number of supported frame buffers (i.e. the number of
 * structures in macIIFbData which have an actual probeProc).
 */
#define NUMSCREENS (sizeof(macIIFbData)/sizeof(macIIFbData[0]))
#define NUMDEVICES 2

fbFd	macIIFbs[NUMSCREENS];  /* Space for descriptors of open frame buffers */

static PixmapFormatRec	formats[] = {
    1, 1, BITMAP_SCANLINE_PAD,	/* 1-bit deep */
    8, 8, BITMAP_SCANLINE_PAD,	/* 8-bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

struct video *video_index[16];          /* how to find it by slot number */
static struct video video[16];    	/* their attributes */

/*-
 *-----------------------------------------------------------------------
 * InitOutput --
 *	Initialize screenInfo for all actually accessible framebuffers.
 *
 * Results:
 *	screenInfo init proc field set
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */

InitOutput(pScreenInfo, argc, argv)
    ScreenInfo 	  *pScreenInfo;
    int     	  argc;
    char    	  **argv;
{
    int     	  i, index, ac = argc;
    char	  **av = argv;

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
    {
        pScreenInfo->formats[i] = formats[i];
    }

    autoRepeatHandlersInstalled = FALSE;

    signal(SIGSYS, SIG_IGN);

    for (i = 0, index = 0; i < NUMSCREENS - 1; i++) {
	ErrorF("Probing: %s for a video card ... ", macIIFbData[i].devName);
	if ((* macIIFbData[i].probeProc) (pScreenInfo, index, i, argc, argv)) {
	    /* This display exists OK */
	    ErrorF("Succeded!\n");
	    index++;
	} else {
	    /* This display can't be opened */
	    ErrorF("none found.\n");
	}
    }

    signal(SIGSYS, SIG_DFL);

    if (index == 0) {
	ErrorF("Probing: %s ... ", macIIFbData[NUMSCREENS - 1].devName);
	if ((* macIIFbData[NUMSCREENS - 1].probeProc) 
	    (pScreenInfo, index, NUMSCREENS - 1, argc, argv)) {
	    ErrorF("Succeded!\n");
	    index++;
	} else {
	    ErrorF("Failed!\n");
	}
    }
    if (index == 0)
	FatalError("Can't find any displays\n");

    pScreenInfo->numScreens = index;

    macIIInitCursor();
}

/*-
 *-----------------------------------------------------------------------
 * InitInput --
 *	Initialize all supported input devices...what else is there
 *	besides pointer and keyboard?
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Two DeviceRec's are allocated and registered as the system pointer
 *	and keyboard devices.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
InitInput(argc, argv)
    int     	  argc;
    char    	  **argv;
{
    DevicePtr p, k;
    static int  zero = 0;
    
    p = AddInputDevice(macIIMouseProc, TRUE);
    k = AddInputDevice(macIIKbdProc, TRUE);

    RegisterPointerDevice(p, MOTION_BUFFER_SIZE);
    RegisterKeyboardDevice(k);

    signal(SIGIO, SigIOHandler);

    SetInputCheck (&zero, &isItTimeToYield);
}

/*-
 *-----------------------------------------------------------------------
 * macIISlotProbe --
 *	Attempt to find and initialize a framebuffer. 
 *
 * Results:
 *	TRUE if everything went ok. FALSE if not.
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped.
 *
 *-----------------------------------------------------------------------
 */
#define SLOT_LOW 9
Bool
macIISlotProbe (pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the macIIFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         i, oldNumScreens;
    int		depth;
    char 	*video_physaddr;
    static char *video_virtaddr = 120 * 1024 * 1024;

    if (macIIFbData[fbNum].probeStatus == probedAndFailed) {
	return FALSE;
    }

    if (macIIFbData[fbNum].probeStatus == neverProbed) {

	if ((depth = video_find(SLOT_LOW + fbNum)) < 0) {
		macIIFbData[fbNum].probeStatus = probedAndFailed;
		return FALSE;
	}

	video_physaddr = video[SLOT_LOW + fbNum].video_base;
	video_virtaddr = video_virtaddr + (8 * 1024 * 1024);

	/*
	 * Note that a unique reference number is provided as the
	 * first argument to phys. School of hard knocks ...
	 */
	if (phys(index, video_virtaddr, 1024*1024, video_physaddr) == -1) {
		FatalError ("phys failed, server must run suid root");
		return (FALSE);
	}

    	macIIFbs[index].fb = (pointer)(video_virtaddr + 
		video[SLOT_LOW + fbNum].video_data.v_baseoffset); 

	macIIFbs[index].fd = depth; /* XXX */
	macIIFbs[index].info = video[SLOT_LOW + fbNum].video_data;
	macIIFbData[fbNum].probeStatus = probedAndSucceeded;

    }

    /*
     * If we've ever successfully probed this device, do the following. 
     */

    oldNumScreens = pScreenInfo->numScreens;
    switch (macIIFbs[index].fd) { /* XXX */
	case 1:
    	    i = AddScreen(macIIBW2Init, argc, argv);
    	    pScreenInfo->screen[index].CloseScreen = macIIBW2CloseScreen;
	    break;
	case 8:
    	    i = AddScreen(macIICG4CInit, argc, argv);
    	    pScreenInfo->screen[index].CloseScreen = macIICG4CCloseScreen;
	    break;
	default:
	    FatalError("Encountered bogus depth: %d", macIIFbs[index].fd);
	    break;
    }

    return (i > oldNumScreens);
}

static int
video_find(slot)
int slot;
{
	register struct video *vp;
	register struct video_data *vdp;
	int depth;

	vp = &video[slot];


	/*
	 *	If it isn't a video card, ignore it.  Otherwise, get
	 *		the driver and video parameter block from the
	 *		slot ROM.
	 */
	
	vp->video_base = (char *)(0xf0000000 | (slot<<24));
	vp->dce.dCtlSlot = slot;
	vp->dce.dCtlDevBase = (long) vp->video_base;
	vp->dce.dCtlExtDev = 0;
	if ((depth = get_video_data(vp)) < 0)
		return(-1);
#ifdef notdef
	if (get_video_driver(vp)) {
		return(-1);
	}
#endif
	vdp = &vp->video_data;
	vp->video_mem_x = 8*vdp->v_rowbytes;
	vp->video_mem_y = vdp->v_bottom - vdp->v_top;
	vp->video_scr_x = vdp->v_right - vdp->v_left;
	vp->video_scr_y = vdp->v_bottom - vdp->v_top;
	vp->video_addr = vp->video_base + vdp->v_baseoffset;
	video_index[slot] = vp;
	vp->video_slot = slot;

	return(depth);
}

/*
    This routine uses the Slot Manager to find a video devices default
mode and corresponding video parameter block.  It returns zero upon
success and a slot manager error code upon failure.  This code is
pretty much stolen from the Monitors Desk Accessory.
    We search through the list of video parameter blocks for the one
with the smallest bits/pixel.  For most devices, this will be the
first on in the list.
    This routine fills in the video_data and video_def_mode fields of the
video structure.  It also fills in the dCtlSlotId field of the dce which
is a magic number understood by only a few people on earth.  These people
have gained this knowledge only by promising to remove their tongues.
*/

#include "sys/slotmgr.h"
static int get_video_data(vp)
register struct video *vp;
{
	int depth = 1024;
	int default_mode = 0x80;/* video modes normally default to 0x80 */
	int err;		/* last slot manager result */
	int success = 0;	/* assume failure */
	struct SpBlock pb;
	struct video_data *vd;
	caddr_t slotModesPointer;
	int nextMode;

	pb.spSlot = vp->dce.dCtlSlot;
	pb.spID = 0;
	pb.spCategory = 3;	/* catDisplay */
	pb.spCType = 1;		/* typeVideo */
	pb.spDrvrSW = 1;	/* drSwApple */
	pb.spTBMask = 1;

	err = slotmanager(_sNextTypesRsrc,&pb);
	if (err == 0 && pb.spSlot != vp->dce.dCtlSlot)
	    err = smNoMoresRsrcs;
	else if (err == 0) {
		vp->dce.dCtlSlotId = pb.spID;
		slotModesPointer = pb.spsPointer;
		for (nextMode = 0x80; depth != 1 && !err; nextMode++) {
			pb.spID = nextMode;
			pb.spsPointer = slotModesPointer;
			err = slotmanager(_sFindStruct,&pb);
			if (err == 0) {
				pb.spID = 1;	/* mVidParams */
				pb.spResult = 
				    (long *)malloc(sizeof(struct video_data));
				err = slotmanager(_sGetBlock,&pb);
				if (err == 0) {
					vd = (struct video_data *) pb.spResult;
					if (vd->v_pixelsize < depth) {
						depth = vd->v_pixelsize;
						default_mode = nextMode;
						vp->video_data = *vd;
						success = 1;
					}
					else free(pb.spResult);
				}
				else free(pb.spResult);
			}
		}
	}
	vp->video_def_mode = default_mode;
	return success? depth: -abs(err);
}

#ifdef notdef
/*
    This routine reads a Macintosh-style device driver from the slot ROM.  A
pointer to the driver is stored in the video_driver field of the video
structure.  If all goes well, zero gets returned.  Otherwise, a slot manager
error is returned.
*/

static int get_video_driver(vp)
register struct video *vp;
{
#define sRsrc_DrvrDir	4
#define sMacOS68000	1
#define sMacOS68020	2
	int err;		/* last slot manager result */
	struct SpBlock pb;

	pb.spSlot = vp->dce.dCtlSlot;
	pb.spID = vp->dce.dCtlSlotId;

	err = slotmanager(_sRsrcInfo,&pb);
	if (err == 0) {
		pb.spID = sRsrc_DrvrDir;
		err = slotmanager(_sFindStruct,&pb);
		if (err == 0) {
			pb.spID = sMacOS68020;
			err = slotmanager(_sGetBlock,&pb);
			if (err != 0) {
				pb.spID = sMacOS68000;
				err = slotmanager(_sGetBlock,&pb);
			}
			if (err == 0) {
			    vp->video_driver = (caddr_t) pb.spResult;
			}
		}
	}
	return err;
}
#endif

/*-
 *-----------------------------------------------------------------------
 * macIIQueryBestSize --
 *	Supposed to hint about good sizes for things.
 *
 * Results:
 *	Perhaps change *pwidth (Height irrelevant)
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
void
macIIQueryBestSize(class, pwidth, pheight)
int class;
short *pwidth;
short *pheight;
{
    unsigned width, test;

    switch(class)
    {
      case CursorShape:
      case TileShape:
      case StippleShape:
	  width = *pwidth;
	  if ((int)width > 0) {
	      /* Return the closest power of two not less than what they gave me */
	      test = 0x80000000;
	      /* Find the highest 1 bit in the width given */
	      while(!(test & width))
		 test >>= 1;
	      /* If their number is greater than that, bump up to the next
	       *  power of two */
	      if((test - 1) & width)
		 test <<= 1;
	      *pwidth = test;
	  }
	  /* We don't care what height they use */
	  break;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIScreenInit --
 *	Things which must be done for all types of frame buffers...
 *	Should be called last of all.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The graphics context for the screen is created. The CreateGC,
 *	CreateWindow and ChangeWindowAttributes vectors are changed in
 *	the screen structure.
 *
 *	Both a BlockHandler and a WakeupHandler are installed for the
 *	first screen.  Together, these handlers implement autorepeat
 *	keystrokes on the macII.
 *
 *-----------------------------------------------------------------------
 */
void
macIIScreenInit (pScreen)
    ScreenPtr	  pScreen;
{
    fbFd    	  *fb;
    DrawablePtr	  pDrawable;
    extern void   macIIBlockHandler();
    extern void   macIIWakeupHandler();
    static ScreenPtr autoRepeatScreen;

    fb = &macIIFbs[pScreen->myNum];

    /*
     * Prepare the GC for cursor functions on this screen.
     * Do this before setting interceptions to avoid looping when
     * putting down the cursor...
     */
    pDrawable = (DrawablePtr)(pScreen->devPrivate);

    fb->pGC = CreateScratchGC (pDrawable->pScreen, pDrawable->depth);

    /*
     * By setting graphicsExposures false, we prevent any expose events
     * from being generated in the CopyArea requests used by the cursor
     * routines.
     */
    fb->pGC->graphicsExposures = FALSE;

    /*
     * Preserve the "regular" functions
     */
    fb->CreateGC =	    	    	pScreen->CreateGC;
    fb->CreateWindow = 	    	    	pScreen->CreateWindow;
    fb->ChangeWindowAttributes =    	pScreen->ChangeWindowAttributes;
    fb->GetImage =	    	    	pScreen->GetImage;
    fb->GetSpans =			pScreen->GetSpans;

    /*
     * Interceptions
     */
    pScreen->CreateGC =	    	    	macIICreateGC;
    pScreen->CreateWindow = 	    	macIICreateWindow;
    pScreen->ChangeWindowAttributes = 	macIIChangeWindowAttributes;
    pScreen->QueryBestSize =		macIIQueryBestSize;
    pScreen->GetImage =	    	    	macIIGetImage;
    pScreen->GetSpans =			macIIGetSpans;

    /*
     * Cursor functions
     */
    pScreen->RealizeCursor = 	    	macIIRealizeCursor;
    pScreen->UnrealizeCursor =	    	macIIUnrealizeCursor;
    pScreen->DisplayCursor = 	    	macIIDisplayCursor;
    pScreen->SetCursorPosition =    	macIISetCursorPosition;
    pScreen->CursorLimits = 	    	macIICursorLimits;
    pScreen->PointerNonInterestBox = 	macIIPointerNonInterestBox;
    pScreen->ConstrainCursor = 	    	macIIConstrainCursor;
    pScreen->RecolorCursor = 	    	macIIRecolorCursor;

    /*
     *	Block/Unblock handlers
     */
    if (autoRepeatHandlersInstalled == FALSE) {
	autoRepeatScreen = pScreen;
	autoRepeatHandlersInstalled = TRUE;
    }

    if (pScreen == autoRepeatScreen) {
        pScreen->BlockHandler = macIIBlockHandler;
        pScreen->WakeupHandler = macIIWakeupHandler;
    }

}

#ifdef notdef
extern char *getenv();
extern char *strncpy();

/*-
 *-----------------------------------------------------------------------
 * nthdev --
 *	Return the nth device in a colon-separated list of devices.
 *	n is 0-origin.
 *
 * Results:
 *	A pointer to a STATIC string which is the device name.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static char *
nthdev (dList, n)
    register char    *dList;	    /* Colon-separated device names */
    int	    n;	  	    /* Device number wanted */
{
    char *result;
    static char returnstring[100];

    while (n--) {
	while (*dList && *dList != ':') {
	    dList++;
	}
    }
    if (*dList) {
	register char *cp = dList;

	while (*cp && *cp != ':') {
	    cp++;
	}
	result = returnstring;
	strncpy (result, dList, cp - dList);
	result[cp - dList] = '\0';
    } else {
	result = (char *)0;
    }
    return (result);
}
#endif

/*-
 *-----------------------------------------------------------------------
 * macIIOpenFrameBuffer --
 *	Open a frame buffer according to several rules. 
 *	Find the device to use by looking in the macIIFbData table,
 *	an XDEVICE envariable, a -dev switch or using /dev/console if trying
 *	to open screen 0 and all else has failed.
 *
 * Results:
 *	The fd of the framebuffer.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
int
macIIOpenFrameBuffer(expect, pfbType, index, fbNum, argc, argv)
    int	    	  expect;   	/* The expected type of framebuffer */
    fbtype *pfbType; 	/* Place to store the fb info */
    int	    	  fbNum;    	/* Index into the macIIFbData array */
    int	    	  index;    	/* Screen index */
    int	    	  argc;	    	/* Command-line arguments... */
    char	  **argv;   	/* ... */
{
    char       	  *name=(char *)0;
    int           i;	    	/* Index into argument list */
    int           fd = -1;	    	/* Descriptor to device */
    static int	  devFbUsed=FALSE;  /* true if /dev/fb has been used for a */
    	    	  	    	    /* screen already */
    static Bool	  inited = FALSE;
    static char	  *xdevice; 	/* string of devices to use from environ */
    static char	  *devsw;   	/* string of devices from args */

#ifdef notdef
    if (!inited) {
	xdevice = devsw = (char *)NULL;

	xdevice = getenv (XDEVICE);
	/*
	 * Look for an argument of the form -dev <device-string>
	 * If such a one is found place the <device-string> in devsw.
	 */
	for (i = 1; i < argc; i++) {
	    if ((strcmp(argv[i], "-dev") == 0) && (i + 1 < argc)) {
		devsw = argv[i+1];
		break;
	    }
	}
	inited = TRUE;
    }

    /*
     * Attempt to find a file name for the frame buffer 
     */

    /*
     * First see if any device was given on the command line.
     * If one was and the device is both readable and writeable,
     * set 'name' to it, else set it to NULL.
     */
    if (devsw == (char *)NULL ||
	(name = nthdev (devsw, index)) == (char *)NULL ||
	(access (name, R_OK | W_OK) != 0) ||
	(strcmp(name, macIIFbData[fbNum].devName) != 0)) {
	    name = (char *)NULL;
    }
	    
    /*
     * If we still don't have a device for this screen, check the
     * environment variable for one. If one was given, stick its
     * path in name and check its accessibility. If it's not
     * properly accessible, then reset the name to NULL to force the
     * checking of the macIIFbData array.
     */
    if (devsw == (char *)NULL && name == (char *)NULL &&
	xdevice != (char *)NULL &&
	(name = nthdev(xdevice, index)) != (char *)NULL &&
	(access (name, R_OK | W_OK) != 0)) {
	    name = (char *)NULL;
    }

    /*
     * Take the device given in the frame buffer description
     * and see if it exists and is accessible. If it does/is,
     * we will use it, as long as no other device was given.
     */
    if (devsw == (char *)NULL && name == (char *)NULL &&
	access(macIIFbData[fbNum].devName, (R_OK | W_OK)) == 0) {
	    name = macIIFbData[fbNum].devName;
    }

    /*
     * If we still have nothing and have yet to use "/dev/console" for
     * a screen, default the name to be "/dev/console"
     */
    if (devsw == (char *)NULL && name == (char *)NULL && !devFbUsed) {
	name = "/dev/console";
    }
#else
	name = "dev/console";
#endif notdef


    if (name) {
	fd = open(name, O_RDWR, 0);
        if (fd < 0) {
	    return (-1);
	} 

        {
		struct strioctl ctl;

        	ctl.ic_cmd = VIDEO_DATA;
		ctl.ic_timout = -1;
        	ctl.ic_len = sizeof(fbtype);
		ctl.ic_dp = (char *)pfbType;
        	if (ioctl(fd, I_STR, &ctl) < 0) {
                	FatalError("Failed to ioctl I_STR VIDEO_DATA.\n");
			(void) close(fd);
                	return(!Success);
        	}

	}
    }

#ifdef notdef
    if (name && strcmp (name, "/dev/console") == 0) {
	devFbUsed = TRUE;
    }
#endif

    return (fd);
}
