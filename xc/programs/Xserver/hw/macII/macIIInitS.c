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

#include    "macII.h"
#include    <servermd.h>
#include    "dixstruct.h"
#include    "dix.h"
#include    "opaque.h"

extern int macIIMouseProc();
extern void macIIKbdProc();

extern Bool macIIMonoProbe();
extern Bool macIIMonoInit();
extern Bool macIIMonoCloseScreen();

extern Bool macIIColorInit();
extern Bool macIIColorCloseScreen();

extern Bool macIISlotProbe();
extern void ProcessInputEvents();

extern void SetInputCheck();
extern GCPtr CreateScratchGC();

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
    macIISlotProbe,  	"slot 0",	    neverProbed,
    macIISlotProbe,  	"slot 1",	    neverProbed,
    macIISlotProbe,  	"slot 2",	    neverProbed,
    macIISlotProbe,  	"slot 3",	    neverProbed,
    macIISlotProbe,  	"slot 4",	    neverProbed,
    macIISlotProbe,  	"slot 5",	    neverProbed,
    macIISlotProbe,  	"slot 6",	    neverProbed,
    macIISlotProbe,  	"slot 7",	    neverProbed,
    macIISlotProbe,  	"slot 8",	    neverProbed,
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
     macIIMonoProbe, 	"/dev/console",	    neverProbed,
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

#define NUMSLOTS 16
struct video *video_index[NUMSLOTS];     /* how to find it by slot number */
static struct video video[NUMSLOTS];   /* their attributes */

#define NUMMODES	7		/* 1,2,4,8,16,24,32 */

static struct video_mode_struct{
	int depth[NUMMODES];
        int mode[NUMMODES];   
	struct video_data  info[NUMMODES];
} video_modes[NUMSLOTS] = {
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

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

#define SLOT_LO 0x09
    for (i = SLOT_LO, index = 0; i < NUMSCREENS - 1; i++) {
#ifdef debug
	ErrorF("Probing: %s for a video card ... ", macIIFbData[i].devName);
#endif
	if ((* macIIFbData[i].probeProc) (pScreenInfo, index, i, argc, argv)) {
	    /* This display exists OK */
#ifdef debug
	    ErrorF("Succeded!\n");
#endif
	    index++;
	} else {
	    /* This display can't be opened */
#ifdef debug
	    ErrorF("none found.\n");
#endif
	}
    }

    if (index == 0) {
#ifdef debug
	ErrorF("Probing: /dev/console ... ");
#endif
	if (macIIMonoProbe(pScreenInfo, index, NUMSCREENS - 1, argc, argv)) {
#ifdef debug
	    ErrorF("Succeded!\n");
#endif
	    index++;
	} else {
#ifdef debug
	    ErrorF("Failed!\n");
#endif
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
    static char *video_virtaddr = (char *)(120 * 1024 * 1024);

    if (macIIFbData[fbNum].probeStatus == probedAndFailed) {
	return FALSE;
    }

    if (macIIFbData[fbNum].probeStatus == neverProbed) {

	if ((depth = video_find(fbNum)) < 0) {
		macIIFbData[fbNum].probeStatus = probedAndFailed;
		return FALSE;
	}

	video_physaddr = video[fbNum].video_base;
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
		video[fbNum].video_data.v_baseoffset); 

	macIIFbs[index].slot = fbNum;
	macIIFbs[index].installedMap = NULL;

	/* set things up for default mode */
	macIIFbs[index].default_depth = depth;
        macIIFbs[index].info = video[fbNum].video_data;

	/* if the board supports 8 bits, patch up a couple of things */
	{
	    int i;
	    for (i = 0; i < NUMMODES; i++) {
		if (video_modes[fbNum].depth[i] == 8) {
		    macIIFbs[index].default_depth = 8;
	            macIIFbs[index].info = video_modes[fbNum].info[i];
		}
	    }
	}
	macIIFbData[fbNum].probeStatus = probedAndSucceeded;

    }

    /*
     * If we've ever successfully probed this device, do the following. 
     */

    oldNumScreens = pScreenInfo->numScreens;
    switch (macIIFbs[index].default_depth) {
	case 1:
	    macIIBlackScreen(index);
    	    i = AddScreen(macIIMonoInit, argc, argv);
    	    pScreenInfo->screen[index].CloseScreen = macIIMonoCloseScreen;
	    break;
	case 8:
	    {
	    /* poke the video board */
		struct VDPgInfo vdp;
		struct CntrlParam pb;
		struct strioctl ctl; /* Streams ioctl control structure */
		int mode_index;
		int fd;

#ifdef debug
ErrorF("trying to goose an 8 bit board\n");
#endif
		ctl.ic_cmd = VIDEO_CONTROL;
		ctl.ic_timout = -1;
		ctl.ic_len = sizeof(pb);
		ctl.ic_dp = (char *)&pb;

	        fd = open("/dev/console", O_RDWR, 0);
                if (fd < 0) {
		    FatalError ("could not open /dev/console");
	        } 
		for (mode_index = 0; mode_index < NUMMODES; ) {
		    if (video_modes[fbNum].depth[mode_index] == 8) {
			break;
		    }
		    mode_index++;
		}
#ifdef debug
ErrorF("mode_index: %d fbNum: %x mode: %x\n", mode_index, fbNum, video_modes[fbNum].mode[mode_index]);
#endif
		vdp.csMode = video_modes[fbNum].mode[mode_index];
		vdp.csData = 0;
		vdp.csPage = 0;
		vdp.csBaseAddr = (char *) NULL;

#define noQueueBit 0x0200
#define SetMode 0x2
		pb.qType = fbNum;
		pb.ioTrap = noQueueBit;
		pb.ioCmdAddr = (char *) -1;
		pb.csCode = SetMode;
		* (char **) pb.csParam = (char *) &vdp;

		if (ioctl(fd, I_STR, &ctl) == -1) {
			FatalError ("ioctl I_STR VIDEO_CONTROL failed");
			(void) close (fd);
			return (FALSE);
		}
		if (pb.qType != 0) {
			FatalError ("ioctl I_STR VIDEO_CONTROL CMD failed");
			(void) close (fd);
			return (FALSE);
		}

	    }
	    macIIBlackScreen(index);
    	    i = AddScreen(macIIColorInit, argc, argv);
    	    pScreenInfo->screen[index].CloseScreen = macIIColorCloseScreen;
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
#define MGC
#ifdef MGC
	get_video_data_new(vp);
	{
	int s, d;

#ifdef debug
	ErrorF("after get_video_data_new...\n");
#endif
	for (s = 0; s < NUMSLOTS; s++) {
#ifdef debug
	    ErrorF("Slot %d  ", s);
#endif
	    for (d = 0; d < NUMMODES; d++) {
		if (video_modes[s].depth[d] != 0) {
#ifdef debug
		    ErrorF("mode for %2d bits = %2x  ",
		       video_modes[s].depth[d], video_modes[s].mode[d]);
#endif
		}
	    }
#ifdef debug
	    ErrorF("\n");
#endif
	}
#ifdef debug
	ErrorF("\n\n");
#endif

	}
#endif MGC
	return success? depth: -abs(err);
}

static int get_video_data_new(vp)
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
	int i;
	int slot;

	pb.spSlot = vp->dce.dCtlSlot;
	pb.spID = 0;
	pb.spCategory = 3;	/* catDisplay */
	pb.spCType = 1;		/* typeVideo */
	pb.spDrvrSW = 1;	/* drSwApple */
	pb.spTBMask = 1;

        slot = vp->dce.dCtlSlot;
	err = slotmanager(_sNextTypesRsrc,&pb);
	if (err == 0 && pb.spSlot != vp->dce.dCtlSlot)
	    err = smNoMoresRsrcs;
	else if (err == 0) {
	    vp->dce.dCtlSlotId = pb.spID;
	    slotModesPointer = pb.spsPointer;
	    for (nextMode = 0x80, i=0; ((!err) && (i < NUMMODES)); nextMode++) {
		pb.spID = nextMode;
		pb.spsPointer = slotModesPointer;
		err = slotmanager(_sFindStruct,&pb);
		if (err == 0) {
		    pb.spID = 1;	/* mVidParams */
		    pb.spResult = (long *)(&video_modes[slot].info[i]);
		    err = slotmanager(_sGetBlock,&pb);
		    if (err == 0) {
		        vd = (struct video_data *) pb.spResult;
		        video_modes[slot].depth[i] = vd->v_pixelsize;
		        video_modes[slot].mode[i] = nextMode;
			i++;
		    }
		}
	    }
	}
	return 0;
}

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
 *-------------------------------------------------------------%---------
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

/*-
 *-----------------------------------------------------------------------
 * macIIBlackScreen --
 *	Fill a frame buffer with pixel 1.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
int
macIIBlackScreen(index)
	int index;
{
    fbFd *pf;
    register unsigned char* fb;
    register int fbinc, line, lw;
    register unsigned int *fbt;

    pf = &macIIFbs[index];
    fb = pf->fb; /* Assumed longword aligned! */

    switch (pf->info.v_pixelsize) {
    case 1:
    {
	fbinc = pf->info.v_rowbytes;
        for (line = pf->info.v_top; line < pf->info.v_bottom; line++) {
	    lw = ((pf->info.v_right - pf->info.v_left) + 31) >> 5;
	    fbt = (unsigned int *)fb;
	    do {
		*fbt++ = 0xffffffff;
	    } while (--lw);
	    fb += fbinc;
	}
	break;
    }
    case 8:
    {
	fbinc = pf->info.v_rowbytes;
        for (line = pf->info.v_top; line < pf->info.v_bottom; line++) {
	    lw = ((pf->info.v_right - pf->info.v_left) + 3) >> 2;
	    fbt = (unsigned int *)fb;
	    do {
		*fbt++ = 0x01010101;
	    } while (--lw);
	    fb += fbinc;
	}
	break;
    }
    default:
	ErrorF("Bad depth in macIIBlackScreen.");
	break;
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIOpenFrameBuffer --
 *	Open a frame buffer  through the /dev/console interface.
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
    fbtype 	  *pfbType; 	/* Place to store the fb info */
    int	    	  fbNum;    	/* Index into the macIIFbData array */
    int	    	  index;    	/* Screen index */
    int	    	  argc;	    	/* Command-line arguments... */
    char	  **argv;   	/* ... */
{
    int           fd = -1;	    	/* Descriptor to device */
    struct strioctl ctl;

    fd = open("/dev/console", O_RDWR, 0);
    if (fd < 0) {
	return (-1);
    } 

    ctl.ic_cmd = VIDEO_DATA;
    ctl.ic_timout = -1;
    ctl.ic_len = sizeof(fbtype);
    ctl.ic_dp = (char *)pfbType;
    if (ioctl(fd, I_STR, &ctl) < 0) {
        FatalError("Failed to ioctl I_STR VIDEO_DATA.\n");
	(void) close(fd);
        return(!Success);
    }

    return (fd);
}
