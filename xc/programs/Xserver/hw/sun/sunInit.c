/* $XConsortium: sunInit.c,v 5.32 92/12/03 10:05:15 rws Exp $ */
/*
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

#include    "sun.h"

extern int sunMouseProc();
extern int sunKbdProc();
#ifndef i386 /* { */
extern Bool sunBW2Init();
#endif /* } */
#ifndef MONO_ONLY /* { */
extern Bool sunCG3Init();
#ifndef i386 /* { */
extern Bool sunCG2Init();
extern Bool sunCG4Init();
#endif /* } */
#ifdef FBTYPE_SUNFAST_COLOR /* { */
extern Bool sunCG6Init();
#endif /* } */
#endif /* } */

#ifdef i386 /* { */
#define BW2I NULL
#else /* }{ */
#define BW2I sunBW2Init
#endif /* } */
#ifdef MONO_ONLY /* { */
#define CG2I NULL
#define CG3I NULL
#define CG4I NULL
#define CG6I NULL
#else /* }{ */
#define CG3I sunCG3Init
#ifdef i386 /* { */
#define CG2I NULL
#define CG4I NULL
#else /* }{ */
#define CG2I sunCG2Init
#define CG4I sunCG4Init
#endif /* } */
#ifdef FBTYPE_SUNFAST_COLOR /* { */
#define CG6I sunCG6Init
#else /* }{ */
#define CG6I NULL
#endif /* } */
#endif /* } */

static Bool	sunDevsInited = FALSE;

Bool sunAutoRepeatHandlersInstalled;	/* FALSE each time InitOutput called */
Bool sunSupportsDepth8 = FALSE;
Bool sunSupportsDepth24 = FALSE;
Bool sunDoF11 = TRUE, sunDoNumlock = TRUE, sunDoCompose = TRUE;
Bool FlipPixels = FALSE;
Bool FbInfo = FALSE;
int* sunProtected = NULL;

/*
 * The name member in the following table correspond to the 
 * FBTYPE_??? macros defined in /usr/include/sun/fbio.h file
 */
sunFbDataRec sunFbData[FBTYPE_LASTPLUSONE] = {
  { NULL, "SUN1BW (bwone)" },
  { NULL, "SUN1COLOR (cgone)" },
  { BW2I, "SUN2BW (bwtwo)" },	
  { CG2I, "SUN2COLOR (cgtwo)" },
  { NULL, "SUN2GP (gpone/gptwo)" },
  { NULL, "SUN5COLOR (RR accel)" },
  { CG3I, "SUN3COLOR (cgthree)" },
  { NULL, "MEMCOLOR" },
  { CG4I, "SUN4COLOR (cgfour)" },
  { NULL, "NOTSUN1" },
  { NULL, "NOTSUN2" },
  { NULL, "NOTSUN3" }	
#ifndef i386 /* { */
 ,{ CG6I, "SUNFAST_COLOR (cgsix/GX)" },
  { NULL, "SUNROP_COLOR (cgeight/cgnine)" },
  { NULL, "SUNFB_VIDEO" },
  { NULL, "SUNGIFB" },
  { NULL, "SUNPLAS" },
  { NULL, "SUNGP3 (cgtwelve/GS)" },
  { NULL, "SUNGT (gt)" },
  { NULL, "RESERVED1" }
#endif /* } */
};

/*
 * a list of devices to try if there is no environment or command
 * line list of devices
 */
#ifdef MONO_ONLY /* { */
static char *fallbackList[] = {
    BWTWO0DEV, BWTWO1DEV
};
#else /* }{ */
static char *fallbackList[] = {
#ifndef i386 /* { */
    CGTWO0DEV, CGTWO1DEV, CGTWO2DEV,
#endif /* } */
    CGTHREE0DEV,
#ifndef i386 /* { */
    CGTHREE1DEV, CGTHREE2DEV,
#endif /* } */
#ifdef FBTYPE_SUNFAST_COLOR /* { */
    CGSIX0DEV, CGSIX1DEV, CGSIX2DEV,
#endif /* } */
#ifndef i386 /* { */
    CGFOUR0DEV, BWTWO0DEV, BWTWO1DEV,
#endif /* } */
    "/dev/fb"
};
#endif /* } */

#define FALLBACK_LIST_LEN sizeof fallbackList / sizeof fallbackList[0]

fbFd sunFbs[MAXSCREENS];

static PixmapFormatRec	formats[] = {
    { 1, 1, BITMAP_SCANLINE_PAD	}, /* 1-bit deep */
    { 8, 8, BITMAP_SCANLINE_PAD	}  /* 8-bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

/*
 * sunOpenFrameBuffer --
 *	Open a frame buffer according to several rules.
 *	Find the device to use by looking in the sunFbData table,
 *	an XDEVICE envariable, a -dev switch or using /dev/fb if trying
 *	to open screen 0 and all else has failed.
 *
 * Results:
 *	The fd of the framebuffer.
 */
static 
int sunOpenFrameBuffer(device, screen)
    char		*device;	/* e.g. "/dev/cgtwo0" */
    int			screen;    	/* what screen am I going to be */
{
    int			ret = TRUE;
    struct fbgattr	*fbattr;
    static int		devFbUsed;

    sunFbs[screen].fd = -1;
    if (strcmp (device, "/dev/fb") == 0 && devFbUsed)
	return FALSE;
    if (access (device, R_OK | W_OK) == -1)
	return FALSE;
    if ((sunFbs[screen].fd = open(device, O_RDWR, 0)) == -1)
	ret = FALSE;
    else {
	fbattr = (struct fbgattr *) xalloc (sizeof (struct fbgattr));
	if (ioctl(sunFbs[screen].fd, FBIOGATTR, fbattr) == -1) {
	    /*
		This is probably a bwtwo; the $64,000 question is: 
		is it the mono plane of a cgfour, or is it really a 
		real bwtwo?  If there is only a cgfour in the box or 
		only a bwtwo in the box, then it isn't a problem.  If 
		it's a 3/60, which has a bwtwo on the mother board *and* 
		a cgfour, then things get tricky because there's no way 
		to tell if the bwtwo is really being emulated by the cgfour.
	    */
	    xfree (fbattr);
	    fbattr = NULL;
	    if (ioctl(sunFbs[screen].fd, FBIOGTYPE, &sunFbs[screen].info) == -1) {
		Error("unable to get frame buffer attributes");
		(void) close(sunFbs[screen].fd);
		sunFbs[screen].fd = -1;
		ret = FALSE; 
	    }
	}
	if (ret) {
	    devFbUsed = TRUE;
	    if (fbattr)
		sunFbs[screen].info = fbattr->fbtype;
	    sunFbs[screen].fbPriv = (pointer) fbattr;
	    if (fbattr && 
		fbattr->fbtype.fb_type < FBTYPE_LASTPLUSONE && 
		!sunFbData[fbattr->fbtype.fb_type].init) {
		int _i;
		ret = FALSE;
		for (_i = 0; _i < FB_ATTR_NEMUTYPES; _i++) {
		    if (sunFbData[fbattr->emu_types[_i]].init) {
			sunFbs[screen].info.fb_type = fbattr->emu_types[_i];
			ret = TRUE;
			if (FbInfo)
			    ErrorF ("%s is emulating a %s\n", device,
				sunFbData[fbattr->fbtype.fb_type].name);
			break;
		    }
		}
	    }
	    if (FbInfo) 
		ErrorF ("%s is really a %s\n", device, 
		    sunFbData[fbattr ? fbattr->fbtype.fb_type : sunFbs[screen].info.fb_type].name);
	}
    }
    if (!ret)
	sunFbs[screen].fd = -1;
    return ret;
}

/*-
 *-----------------------------------------------------------------------
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	sunSigIO is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
#ifdef SVR4 /* { */
static void SigIOHandler(sig)
#else /* }{ */
static void SigIOHandler(sig, code, scp, addr)
    int		code;
    int		sig;
    struct sigcontext *scp;
    char	*addr;
#endif /* } */
{
    sunEnqueueEvents ();
}

/*-
 *-----------------------------------------------------------------------
 * sunNonBlockConsoleOff --
 *	Turn non-blocking mode on the console off, so you don't get logged
 *	out when the server exits.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
#ifdef SVR4
void sunNonBlockConsoleOff()
#else
void sunNonBlockConsoleOff(arg)
    char *arg;
#endif
{
    register int i;

    i = fcntl(2, F_GETFL, 0);
    if (i >= 0)
	(void) fcntl(2, F_SETFL, i & ~FNDELAY);
}

static
char **sunGetDeviceList (argc, argv)
    int		argc;
    char	**argv;
{
    int		i;
    char	*envList = NULL;
    char	*cmdList = NULL;
    char	**deviceList = (char **)NULL; 

    for (i = 1; i < argc; i++)
	if (strcmp (argv[i], "-dev") == 0 && i+1 < argc) {
	    cmdList = argv[i + 1];
	    break;
	}
    if (!cmdList)
	envList = getenv ("XDEVICE");

    if (cmdList || envList) {
	char	*_tmpa;
	char	*_tmpb;
	int	_i1;
	deviceList = (char **) xalloc ((MAXSCREENS + 1) * sizeof (char *));
	_tmpa = (cmdList) ? cmdList : envList;
	for (_i1 = 0; _i1 < MAXSCREENS; _i1++) {
	    _tmpb = strtok (_tmpa, ":");
	    if (_tmpb)
		deviceList[_i1] = _tmpb;
	    else
		deviceList[_i1] = NULL;
	    _tmpa = NULL;
	}
	deviceList[MAXSCREENS] = NULL;
    }
    if (!deviceList) {
	/* no environment and no cmdline, so default */
	deviceList = 
	    (char **) xalloc ((FALLBACK_LIST_LEN + 1) * sizeof (char *));
	for (i = 0; i < FALLBACK_LIST_LEN; i++)
	    deviceList[i] = fallbackList[i];
	deviceList[FALLBACK_LIST_LEN] = NULL;
    }
    return deviceList;
}

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

void InitOutput(pScreenInfo, argc, argv)
    ScreenInfo 	  *pScreenInfo;
    int     	  argc;
    char    	  **argv;
{
    int     	i, scr;
    int		nonBlockConsole = 0;
    char	**devList;
    static int	setup_on_exit = 0;
    extern Bool	RunFromSmartParent;
    extern void AddScreen();

    if (!monitorResolution)
	monitorResolution = 90;
    if (RunFromSmartParent)
	nonBlockConsole = 1;
    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i],"-debug"))
	    nonBlockConsole = 0;
    }

    /*
     *	Writes to /dev/console can block - causing an
     *	excess of error messages to hang the server in
     *	deadlock.  So.......
     */
    if (nonBlockConsole) {
	if (!setup_on_exit) {
#ifdef SVR4 /* { */
	    if (atexit(sunNonBlockConsoleOff))
#else /* }{ */
	    if (on_exit(sunNonBlockConsoleOff, (char *)0))
#endif /* } */
		ErrorF("InitOutput: can't register NBIO exit handler\n");

	    setup_on_exit = 1;
	}
	i = fcntl(2, F_GETFL, 0);
	if (i >= 0)
	    i = fcntl(2, F_SETFL, i | FNDELAY);
	if (i < 0) {
	    Error("fcntl");
	    ErrorF("InitOutput: can't put stderr in non-block mode\n");
	}
    }
    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
        pScreenInfo->formats[i] = formats[i];

    sunAutoRepeatHandlersInstalled = FALSE;

    if (!sunDevsInited) {
	/* first time ever */
	for (scr = 0; scr < MAXSCREENS; scr++)
	    sunFbs[scr].fd = -1;
	devList = sunGetDeviceList (argc, argv);
	for (i = 0, scr = 0; devList[i] != NULL && scr < MAXSCREENS; i++)
	    if (sunOpenFrameBuffer (devList[i], scr)) {
		AddScreen (sunFbData[sunFbs[scr].info.fb_type].init, 
				argc, argv);
		scr++;
	    }
	sunDevsInited = TRUE;
	xfree (devList);
    } else {
	/* 
	    xdm SIGHUP'd rather than SIGKILL'd between sessions, 
	    server low-level has reset and deleted the screens.  
	    Now re-add the screens that are already open'd and mmap'd
	*/
	for (scr = 0; scr < MAXSCREENS; scr++)
	    if (sunFbs[scr].fd != -1)
		AddScreen (sunFbData[sunFbs[scr].info.fb_type].init, 
				argc, argv);
    }

    if (!sunSupportsDepth8)
	pScreenInfo->numPixmapFormats--;
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
void InitInput(argc, argv)
    int     	  argc;
    char    	  **argv;
{
    int		i;
    DevicePtr	p, k;
    extern Bool mieqInit();

    for (i = 1; i < argc; i++) {
	if ((strcmp(argv[i],"-kbd") == 0) && (i+1 < argc)) {
	    i++;
	    if (!strcmp(argv[i],"f11"))
		sunDoF11 = TRUE;
	    else if (!strcmp(argv[i],"f36"))
		sunDoF11 = FALSE;
	    else if (!strcmp(argv[i],"compose"))
		sunDoCompose = TRUE;
	    else if (!strcmp(argv[i],"numlock"))
		sunDoNumlock = TRUE;
	    else if (!strcmp(argv[i],"normal"))
		sunDoCompose = sunDoNumlock = FALSE;
	}
	if ((strcmp(argv[i],"-protect") == 0) && (i+1 < argc)) {
	    int j,n;
	    i++;
	    for(j=n=0; argv[i][j]; j++)
		if (argv[i][j] == ',')
		    n++;
	    sunProtected = (int*)xalloc((n+1)*sizeof(int));
	    for(j=n=0, sunProtected[0]=0; argv[i][j]; j++)
		if (argv[i][j] == ',')
		    sunProtected[++n] = 0;
		else
		    sunProtected[n] = sunProtected[n]*10+argv[i][j]-'0';
	}
    }
    p = AddInputDevice(sunMouseProc, TRUE);
    k = AddInputDevice(sunKbdProc, TRUE);
    if (!p || !k)
	FatalError("failed to create input devices in InitInput");

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    miRegisterPointerDevice(screenInfo.screens[0], p);
    (void) mieqInit (k, p);
#ifdef SVR4
    (void) signal(SIGPOLL, SigIOHandler);
#else
    (void) signal(SIGIO, SigIOHandler);
#endif
}

/*
 * signal() that provides similiar semantics under both SunOS and Solaris,
 * i.e. POSIX, with automatic restarting of system calls.  The following
 * source is borrowed from W. Richard Stevens book "Advanced Programming 
 * in the UNIX Environment".  
 *
 * Why automatic restarting?  Under SunOS, sigvec() based signals default 
 * to automatic restart.  Under Solaris, sigaction() based signals don't, 
 * at least not according to Stevens, although in his book, he does say 
 * that SunOS sigaction() *does* restart system calls by default, so 
 * there's a bit of redundancy in this implementation of signal() under
 * SunOS.
 * 
 * By placing this here in sunInit.c, we ensure that any other calls to 
 * signal(), e.g. those in .../mit/server/os use this function too and not 
 * the one in libc.  Technically speaking, this is really ugly; somewhere
 * or another it's said (probably ANSI C spec) that you may not use names
 * that are defined in the Standard library. But, short of changing the
 * calls to signal() elsewhere in the server source, there's no way to
 * do this. Sigh.
 *
 * The rest of the server just uses signal() to install signal handlers,
 * and the really esoteric signal stuff is done right here in ddx/sun,
 * namely in sunKbd.c and sunMouse.c, where the keyboard and mouse inputs
 * are bundled off to the dix layer, and they've all been converted to 
 * the POSIX family of functions also.
 */

#ifndef i386 /* { */
#ifdef SVR4 /* { */
void (*signal (int signo, void (*func)(int)))(int)
#else /* }{ */
void (*signal (signo, func))()
    int signo;
    void (*func)();
#endif /* } */
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT /* { */
	act.sa_flags |= SA_INTERRUPT;
#endif /* } */
    } else {
#ifdef SA_RESTART /* { */
	act.sa_flags |= SA_RESTART;
#endif /* } */
    }
    if (sigaction (signo, &act, &oact) == -1)
	oact.sa_handler = (void (*)())SIG_ERR;

    return oact.sa_handler;
}
#endif /* } */
