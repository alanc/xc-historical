/*-
 * sunInit.c --
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

#ifndef	lint
static char sccsid[] = "%W %G Copyright 1987 Sun Micro";
#endif

/*-
 * Copyright (c) 1987 by Sun Microsystems,  Inc.
 */

#include    "sun.h"
#include    "dixstruct.h"
#include    "dix.h"
#include    "opaque.h"

extern int sunMouseProc();
extern void sunKbdProc();
extern Bool sunBW2Probe();
extern Bool sunCG2CProbe();
extern Bool sunCG4CProbe();

extern void SetInputCheck();
extern char *strncpy();

#define	XDEVICE	"XDEVICE"
#define	PARENT	"WINDOW_GFX"

int sunSigIO = 0;	 /* For use with SetInputCheck */

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
    sunSigIO++;
    isItTimeToYield++;
}

sunFbDataRec sunFbData[] = {
    sunBW2Probe,  	"/dev/bwtwo0",	    neverProbed,
    sunCG2CProbe,  	"/dev/cgtwo0",	    neverProbed,
    sunCG4CProbe,  	"/dev/cgfour0",	    neverProbed,
};

/*
 * NUMSCREENS is the number of supported frame buffers (i.e. the number of
 * structures in sunFbData which have an actual probeProc).
 */
#define NUMSCREENS (sizeof(sunFbData)/sizeof(sunFbData[0]))
#define NUMDEVICES 2

fbFd	sunFbs[NUMSCREENS];  /* Space for descriptors of open frame buffers */

static PixmapFormatRec	formats[] = {
    1, 1, BITMAP_SCANLINE_PAD,	/* 1-bit deep */
    8, 8, BITMAP_SCANLINE_PAD,	/* 8-bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

/*-
 *-----------------------------------------------------------------------
 * InitOutput --
 *	Initialize screenInfo for all actually accessible framebuffers.
 *	The
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
    int     	  i, index;

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
    {
        pScreenInfo->formats[i] = formats[i];
    }

    for (i = 0, index = 0; i < NUMSCREENS; i++) {
	if ((* sunFbData[i].probeProc) (pScreenInfo, index, i, argc, argv)) {
	    /* This display exists OK */
	    index++;
	} else {
	    /* This display can't be opened */
	    ;
	}
    }
    if (index == 0)
	FatalError("Can't find any displays\n");

    pScreenInfo->numScreens = index;

    sunInitCursor();
    signal(SIGWINCH, SIG_IGN);
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
    
    p = AddInputDevice(sunMouseProc, TRUE);
    k = AddInputDevice(sunKbdProc, TRUE);

    RegisterPointerDevice(p, MOTION_BUFFER_SIZE);
    RegisterKeyboardDevice(k);
    signal(SIGIO, SigIOHandler);

    SetInputCheck (&zero, &isItTimeToYield);
}

/*-
 *-----------------------------------------------------------------------
 * sunScreenInit --
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
 *-----------------------------------------------------------------------
 */
void
sunScreenInit (pScreen)
    ScreenPtr	  pScreen;
{
    fbFd    	  *fb;
    DrawablePtr	  pDrawable;
    fb = &sunFbs[pScreen->myNum];

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
    pScreen->CreateGC =	    	    	sunCreateGC;
    pScreen->CreateWindow = 	    	sunCreateWindow;
    pScreen->ChangeWindowAttributes = 	sunChangeWindowAttributes;
    pScreen->GetImage =	    	    	sunGetImage;
    pScreen->GetSpans =			sunGetSpans;

    /*
     * Cursor functions
     */
    pScreen->RealizeCursor = 	    	sunRealizeCursor;
    pScreen->UnrealizeCursor =	    	sunUnrealizeCursor;
    pScreen->DisplayCursor = 	    	sunDisplayCursor;
    pScreen->SetCursorPosition =    	sunSetCursorPosition;
    pScreen->CursorLimits = 	    	sunCursorLimits;
    pScreen->PointerNonInterestBox = 	sunPointerNonInterestBox;
    pScreen->ConstrainCursor = 	    	sunConstrainCursor;
    pScreen->RecolorCursor = 	    	sunRecolorCursor;

}

extern char *getenv();

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

/*-
 *-----------------------------------------------------------------------
 * sunOpenFrameBuffer --
 *	Open a frame buffer according to several rules. If running under
 *	overview and we're set up for it, use the device given in the
 *	PARENT envariable and note that the screen is under overview.
 *	Else find the device to use by looking in the sunFbData table,
 *	an XDEVICE envariable, a -dev switch or using /dev/fb if trying
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
sunOpenFrameBuffer(expect, pfbType, index, fbNum, argc, argv)
    int	    	  expect;   	/* The expected type of framebuffer */
    struct fbtype *pfbType; 	/* Place to store the fb info */
    int	    	  fbNum;    	/* Index into the sunFbData array */
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

    sunFbs[index].parent = FALSE;

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
	(strcmp(name, sunFbData[fbNum].devName) != 0)) {
	    name = (char *)NULL;
    }
	    
    /*
     * If we still don't have a device for this screen, check the
     * environment variable for one. If one was given, stick its
     * path in name and check its accessibility. If it's not
     * properly accessible, then reset the name to NULL to force the
     * checking of the sunFbData array.
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
	access(sunFbData[fbNum].devName, (R_OK | W_OK)) == 0) {
	    name = sunFbData[fbNum].devName;
    }

    /*
     * If we still have nothing and have yet to use "/dev/fb" for
     * a screen, default the name to be "/dev/fb"
     */
    if (devsw == (char *)NULL && name == (char *)NULL && !devFbUsed) {
	name = "/dev/fb";
    }


    if (name != (char *) NULL && sunUseSunWindows()) {
#ifdef	SUN_WINDOWS

	/*
	 * Running X in coexistence with SunWindows.
	 *
	 * This section of code enables X to run with SunWindows.  This is 
	 * accomplished by opening up screens and windows in the SunWindows 
	 * style and accepting input events from them.  However, since the X 
	 * graphics libraries want a raw framebuffer, we open the 
	 * framebuffer of the current screen and return that.  So we get 
	 * input from SunWindows but send output to the raw framebuffer.
	 */

	char       *parent = getenv("WINDOW_PARENT");
	struct screen sc;
	int	    winFd;
	int	    parentFd;
	int	    framebufferFd;
	Rect	    r;
	static struct screen newScreen;
	struct inputmask inputMask;
	Bool	    keepParent = FALSE;

	/*
	 * If no device was specified on the command line, open the window 
	 * specified in WINDOW_PARENT.  If a device was specified, open a 
	 * new screen on that device and use it as a parent window.
	 */

	if ( devsw ) {
	    bzero( (caddr_t)&newScreen, sizeof(newScreen) );
	    strcpy( newScreen.scr_fbname, name );
	    newScreen.scr_flags |= SCR_TOGGLEENABLE;
	    if ( (parentFd = win_screennew( &newScreen )) < 0 ) {
		ErrorF( "sunOpenFrameBuffer: Can't open new screen on %s.\n",
		    name );
		return( -1 );
	    }
	    keepParent = TRUE;
	} else {
	    if ((parentFd = open(parent, 2, 0)) < 0) {
		ErrorF("sunOpenFrameBuffer: Can't open parent %s.\n", parent);
		return (-1);
	    }
	}

	if ((winFd = win_getnewwindow()) < 0) {
	    ErrorF("sunOpenFrameBuffer: Can't open a new window.\n");
	    close( parentFd );
	    return (-1);
	}

	/* link the new window into the window hierarchy */
	bzero((caddr_t) & r, sizeof r);
	win_getrect(parentFd, &r);
	win_setrect(winFd, &r);
	win_setlink(winFd, WL_PARENT, win_fdtonumber(parentFd));
	win_setlink(winFd, WL_OLDERSIB, win_getlink(parentFd, WL_TOPCHILD));
	sunFbs[index].parent = TRUE;

	/*
	 * We would like to close the parent window here, since it's no 
	 * longer needed.  However, in the case where the parent is really a 
	 * screen, we have to keep it open, because closing a screen closes 
	 * all the windows on that screen.
	 */

	if ( ! keepParent )
	    close(parentFd);
	
	/*
	 * Express interest in SunView mouse events.  Note: the win_insert 
	 * must be AFTER setting the input mask in order to receive the 
	 * initial LOC_WINENTER event.
	 */

	input_imnull(&inputMask);
	inputMask.im_flags = IM_ASCII | IM_META | IM_NEGEVENT | IM_INTRANSIT;
	win_setinputcodebit(&inputMask, LOC_MOVE);
	win_setinputcodebit(&inputMask, LOC_WINEXIT);
	win_setinputcodebit(&inputMask, LOC_WINENTER);
	win_set_pick_mask(winFd, &inputMask);

	win_insert(winFd);

	/*
	 * Determine the framebuffer name from the window's screen, then 
	 * open it.  Then do an FBIOGTYPE ioctl to determine its type.
	 */

	win_screenget( winFd, &sc );
	if ((framebufferFd = open(sc.scr_fbname, O_RDWR, 0)) < 0) {
	    ErrorF("sunOpenFrameBuffer: can't open %s\n",sc.scr_fbname);
	    (void) close(winFd);
	    (void) close(parentFd);
	    return (-1);
	}

	if (ioctl(framebufferFd, FBIOGTYPE, pfbType) < 0) {
	    perror("sunOpenFrameBuffer");
	    (void) close(framebufferFd);
	    (void) close(winFd);
	    (void) close(parentFd);
	    return (-1);
	}

	if (pfbType->fb_type != expect) {
	    (void) close(framebufferFd);
	    (void) close(winFd);
	    (void) close(parentFd);
	    return (-1);
	}

        /*
	 * NDELAY only applies to "input" fds, or fds that can be
	 * read.  sunwindows fds are read, while frame buffer fds aren't.
	 * That's why this fcntl is in the conditional compilation section.
	 */

	if (fcntl(winFd, F_SETFL, O_NDELAY) < 0) {
	    ErrorF("Can't set O_NDELAY on %s\n",name);
	    perror("sunOpenFrameBuffer");
	    (void) close(winFd);
	    return (-1);
	}
	
	fd = framebufferFd;
	windowFd = winFd;
#else
	ErrorF("Not configured to run inside SunWindows\n");
	fd = -1;
#endif	SUN_WINDOWS
    } else if (name) {
	fd = open(name, O_RDWR, 0);
        if (fd < 0) {
	    return (-1);
	} 
	if (ioctl(fd, FBIOGTYPE, pfbType) < 0) {
	    perror("sunOpenFrameBuffer");
	    (void) close(fd);
	    return (-1);
	}
	/* XXX - this is temporary 'cos the CG4 pretends its a BW2 */
	if (strcmp(name, sunFbData[fbNum].devName) != 0 && pfbType->fb_type != expect) {
	    (void) close(fd);
	    return (-1);
	}
    }

    if (name && strcmp (name, "/dev/fb") == 0) {
	devFbUsed = TRUE;
    }

    return (fd);
}
