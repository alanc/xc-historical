/* $XConsortium: xieperf.c,v 1.2 93/10/26 10:07:30 rws Exp $ */

int   verbosity_Group_xielib ;
int   verbosity_Group_xielib_user_level ;

/**** module xieperf.c ****/
/******************************************************************************
				NOTICE
                              
This software is being provided by AGE Logic, Inc. and MIT under the
following license.  By obtaining, using and/or copying this software,
you agree that you have read, understood, and will comply with these
terms and conditions:

     Permission to use, copy, modify, distribute and sell this
     software and its documentation for any purpose and without
     fee or royalty and to grant others any or all rights granted
     herein is hereby granted, provided that you agree to comply
     with the following copyright notice and statements, including
     the disclaimer, and that the same appears on all copies and
     derivative works of the software and documentation you make.
     
     "Copyright 1993 by AGE Logic, Inc. and the Massachusetts
     Institute of Technology"
     
     THIS SOFTWARE IS PROVIDED "AS IS".  AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.  By way of
     example, but not limitation, AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS
     FOR ANY PARTICULAR PURPOSE OR THAT THE SOFTWARE DOES NOT
     INFRINGE THIRD-PARTY PROPRIETARY RIGHTS.  AGE LOGIC AND MIT
     SHALL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE.  IN NO
     EVENT SHALL EITHER PARTY BE LIABLE FOR ANY INDIRECT,
     INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS
     OF PROFITS, REVENUE, DATA OR USE, INCURRED BY EITHER PARTY OR
     ANY THIRD PARTY, WHETHER IN AN ACTION IN CONTRACT OR TORT OR
     BASED ON A WARRANTY, EVEN IF AGE LOGIC OR MIT OR LICENSEES
     HEREUNDER HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
     DAMAGES.
    
     The names of AGE Logic, Inc. and MIT may not be used in
     advertising or publicity pertaining to this software without
     specific, written prior permission from AGE Logic and MIT.

     Title to this software shall at all times remain with AGE
     Logic, Inc.
*****************************************************************************
  
	xieperf.c -- xieperf main module ( looks a lot like x11perf.c ) 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <math.h>

extern int errno;

#include "xieperf.h"

/* Only for working on ``fake'' servers, for hardware that doesn't exist */
static Bool     drawToFakeServer = False;
static Pixmap   tileToQuery     = None;

static Bool     labels		= False;
static Bool 	loadTests 	= False;
static char 	*loadTestsFile;
static int      repeat		= 5;
static int	seconds		= 5;
Bool 	dontClear;
unsigned short capabilities = 0;
unsigned short class_request = SUBSET_FULL;
extern int CacheSizeMax;
static FILE	*fp = ( FILE * ) NULL;

char	*imagepath = "./images";

static Window   status;     /* Status window and GC */
static GC       tgc;

static double syncTime = 0.0;

static int saveargc;
static char **saveargv;

static char *foreground = NULL;
static char *background = NULL;

static int  fixedReps = 0;

static Bool *doit;
Bool WMSafe = False;
static Bool notBoring = False;
static Bool loCal = False;
Bool showErrors = False;
static Bool runErrors = False;
static Bool runEvents = False;
static Bool showLabels = False;   /* Hear me now and believe me later,
				     when 'True', is helpful if test 
				     dies in calibration */ 
				     

static int visualClass = -1;

static XRectangle ws[] = {  /* Clip rectangles */
    {195, 195, 120, 120},
    { 45, 145, 120, 120},
    {345, 245, 120, 120}

};
#define MAXCLIP     (sizeof(ws) / sizeof(ws[0]))
static Window clipWindows[MAXCLIP];
static Colormap cmap;
static int depth = -1;  /* -1 means use default depth */
Window drawableWindow;
Window monitorWindow;
Window monitor2Window;
static int maxcoloridx;

Window	CreateXIEParent();
void	AllocateRest();

/* ScreenSaver state */
static XParmRec    xparms;
static int ssTimeout, ssInterval, ssPreferBlanking, ssAllowExposures;
static XiePhotomap XIEPhotomap;

/************************************************
*	    time related stuff			*
************************************************/

#ifdef VMS

typedef struct _vms_time {
    unsigned long low;
    unsigned long high;
}vms_time;

struct timeval {
    long tv_sec;        /* seconds since Jan. 1, 1970 */
    long tv_usec;  /* and microseconds */
};

struct timezone {
    int  tz_minuteswest;     /* of Greenwich */
    int  tz_dsttime;    /* type of dst correction to apply */
};


static int firsttime = True;
static vms_time basetime;

int gettimeofday(tp, tzp)
    struct timeval *tp;
    struct timezone *tzp;
{
    vms_time current_time, resultant;
    unsigned long mumble, foo;
    int status;

    if (firsttime) {
        sys$gettim(&basetime);
        firsttime = False;
    }
    sys$gettim(&current_time);
    resultant.high = current_time.high - basetime.high;
    resultant.low = current_time.low - basetime.low;
    if (current_time.low < basetime.low) {
        resultant.high -= 1;
    }
    status = lib$ediv( &(10000000), &resultant, &tp->tv_sec, &tp->tv_usec);
    tp->tv_usec /= 10;
    return 0;
}

#endif

static struct  timeval start;

void PrintTime()
{
    time_t t;

    t = time((long *)NULL);
    printf("%s\n", ctime(&t));
}

void InitTimes ()
{
    struct timezone foo;
    gettimeofday(&start, &foo);
}

double ElapsedTime(correction)
    double correction;
{
    struct timeval stop;
    struct timezone foo;
    
    gettimeofday(&stop, &foo);
    if (stop.tv_usec < start.tv_usec) {
        stop.tv_usec += 1000000;
	stop.tv_sec -= 1;
    }
    return  (double)(stop.tv_usec - start.tv_usec) +
            (1000000.0 * (double)(stop.tv_sec - start.tv_sec)) - correction;
}

double RoundTo3Digits(d)
    double d;
{
    /* It's kind of silly to print out things like ``193658.4/sec'' so just
       junk all but 3 most significant digits. */

    double exponent, sign;

    exponent = 1.0;
    /* the code below won't work if d should happen to be non-positive. */
    if (d < 0.0) {
	d = -d;
	sign = -1.0;
    } else
	sign = 1.0;
    if (d >= 1000.0) {
	do {
	    exponent *= 10.0;
	} while (d/exponent >= 1000.0);
	d = (double)((int) (d/exponent + 0.5));
	d *= exponent;
    } else {
	if (d != 0.0) {
	    while (d*exponent < 100.0) {
	        exponent *= 10.0;
	    }
	}
	d = (double)((int) (d*exponent + 0.5));
	d /= exponent;
    }
    return d * sign;
}


void ReportTimes(usecs, n, str, average)
    double  usecs;
    int     n;
    char    *str;
{
    double msecsperobj, objspersec;

    if(usecs != 0.0)
    {
        msecsperobj = usecs / (1000.0 * (double)n);
        objspersec = (double) n * 1000000.0 / usecs;

        /* Round obj/sec to 3 significant digits.  Leave msec untouched, to
	   allow averaging results from several repetitions. */
        objspersec =  RoundTo3Digits(objspersec);

        if (average) {
	    printf("%6d trep @ %7.4f msec (%6.1f/sec): %s\n", 
		    n, msecsperobj, objspersec, str);
	} else {
	    printf("%6d reps @ %7.4f msec (%6.1f/sec): %s\n", 
	        n, msecsperobj, objspersec, str);
	}
    } else {
	printf("%6d %sreps @ 0.0 msec (unmeasurably fast): %s\n",
	    n, average ? "t" : "", str);
    }

}

/************************************************
*		Generic X stuff			*
************************************************/

static char *program_name;
void usage();

Display *
GetDisplay()
{
	return( xparms.d );
}

/*
 * Get_Display_Name (argc, argv) Look for -display, -d, or host:dpy (obselete)
 * If found, remove it from command line.  Don't go past a lone -.
 */
char *Get_Display_Name(pargc, argv)
    int     *pargc;  /* MODIFIED */
    char    **argv; /* MODIFIED */
{
    int     argc = *pargc;
    char    **pargv = argv+1;
    char    *displayname = NULL;
    int     i;

    for (i = 1; i != argc; i++) {
	char *arg = argv[i];

	if (!strcmp (arg, "-display") || !strcmp (arg, "-d")) {
	    if (++i >= argc) usage ();

	    displayname = argv[i];
	    *pargc -= 2;
	    continue;
	}
	if (!strcmp(arg,"-")) {
	    while (i<argc)  *pargv++ = argv[i++];
	    break;
	}
	*pargv++ = arg;
    }

    *pargv = NULL;
    return (displayname);
}


/*
 * Open_Display: Routine to open a display with correct error handling.
 */
Display *Open_Display(display_name)
    char *display_name;
{
    Display *d;

    d = XOpenDisplay(display_name);
    if (d == NULL) {
	fprintf (stderr, "%s:  unable to open display '%s'\n",
		 program_name, XDisplayName (display_name));
	usage ();
	/* doesn't return */
    }

    return(d);
}

#ifdef SIGNALRETURNSINT
int
#else
void
#endif
Cleanup(sig)
    int sig;
{
    fflush(stdout);
    /* This will screw up if Xlib is in the middle of something */
    XSetScreenSaver(xparms.d, ssTimeout, ssInterval, ssPreferBlanking,
	ssAllowExposures);
    XFlush(xparms.d);
    exit(sig);
}

/************************************************
*		Performance stuff		*
************************************************/

void usage()
{
    char    **cpp;
    int     i = 0;
    static char *help_message[] = {
"where options include:",
"    -display <host:display>   the XIE server to contact",
"    -images <path>            path to images supplied with this software,", 
"                              defaults to ./images if flag not specified",
"    -timeout <s>              timeout value for certain tests (default=60 secs)",
"    -sync                     do the tests in synchronous mode",
"    -script file              read tests from a file, ``-'' to use console",
"    -repeat <n>               do tests <n> times (outer loop) (default = 5)",
"    -time <s>                 do tests for <s> seconds each (default = 5)",
"    -depth <depth>            use a visual with <depth> planes per pixel",
"    -GrayScale                use a GrayScale visual",
"    -StaticGray               use a StaticGray visual",
"    -StaticColor              use a StaticColor visual",
"    -PseudoColor              use a PseudoColor visual",
"    -TrueColor                use a TrueColor visual",
"    -DirectColor              use a DirectColor visual",
"    -WMSafe                   let window manager install colormap",
"    -lessboring               very subjective selection of less boring tests",
"    -showlabels               print test label to screen before calibration",
"    -showevents               for error and event tests, display as received",
"    -events                   include event tests",
"    -errors                   include error tests",
"    -loCal                    no calibration, fix reps to 5 ( -reps overrides this )",
"    -all                      do all tests",
"    -tests		       generate a list of all recognizable test names",
"    -cache <n>		       cache up to n photomaps in server",
"    -labels		       print test labels ( use -all or -range to specify tests )",	
"    -DIS                      run DIS subset tests only",			
"    -range <test1>[,<test2>]  like all, but do <test1> to <test2>",
"    -reps <n>                 fix the inner loop rep count (default = auto scale)",
NULL};

    fflush(stdout);
    fprintf(stderr, "usage: %s [-options ...]\n", program_name);
    for (cpp = help_message; *cpp; cpp++) {
	fprintf(stderr, "%s\n", *cpp);
    }

    fprintf(stderr, "\n");
    
    /* Print out original command line as the above usage message is so long */
    for (i = 0; i != saveargc; i++) {
	fprintf(stderr, "%s ", saveargv[i]);
    }
    fprintf(stderr, "\n\n");
    exit (1);
}

void NullProc(xp, p)
    XParms  xp;
    Parms   p;
{
}

Bool NullInitProc(xp, p, reps)
    XParms  xp;
    Parms   p;
    int reps;
{
    return reps;
}


void HardwareSync(xp)
    XParms  xp;
{
    /*
     * Some graphics hardware allows the server to claim it is done,
     * while in reality the hardware is busily working away.  So fetch
     * a pixel from the window that was drawn to, which should be
     * enough to make the server wait for the graphics hardware.
     */
    XImage *image;

    image = XGetImage(xp->d, xp->w, WIDTH-1, HEIGHT-1, 1, 1, ~0, ZPixmap);
    XDestroyImage(image);
}

void DoHardwareSync(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i;
    
    for (i = 0; i != reps; i++) {
	HardwareSync(xp);
    }
}

static Test syncTest = {
    "syncTime", "Internal test for finding how long HardwareSync takes",
    NullInitProc, DoHardwareSync, NullProc, NullProc, 
    VALL, 0, 0,
    {1}
};


Window CreatePerfWindow(xp, x, y, width, height)
    XParms  xp;
    int     width, height, x, y;
{
    XSetWindowAttributes xswa;
    Window w;
    Screen *s;
    int su;
    unsigned long value_mask;

    xswa.background_pixel = xp->background;
    xswa.border_pixel = xp->foreground;
    if ( WMSafe == False )
    	xswa.override_redirect = True;
    else
    	xswa.override_redirect = False;
    xswa.backing_store = False;
    xswa.save_under = False;
    xswa.colormap = cmap;
    value_mask = CWBackPixel | CWBorderPixel | CWColormap |
	CWOverrideRedirect |CWBackingStore | CWSaveUnder;

    w = XCreateWindow(xp->d, xp->p, x, y, width, height, 1,
        xp->vinfo.depth, CopyFromParent, xp->vinfo.visual,
	value_mask, &xswa);
    XMapWindow (xp->d, w);
    return w;
}

Window CreatePerfWindowUnmapped(xp, x, y, width, height)
    XParms  xp;
    int     width, height, x, y;
{
    XSetWindowAttributes xswa;
    Window w;
    Screen *s;
    int su;
    unsigned long value_mask;

    xswa.background_pixel = xp->background;
    xswa.border_pixel = xp->foreground;
    xswa.colormap = cmap;
    if ( WMSafe == False )
    	xswa.override_redirect = True;
    else
    	xswa.override_redirect = False;
    xswa.backing_store = False;
    xswa.save_under = False;
    value_mask = CWBackPixel | CWBorderPixel | CWColormap |
	CWOverrideRedirect |CWBackingStore | CWSaveUnder;
    w = XCreateWindow(xp->d, xp->p, x, y, width, height, 1,
        xp->vinfo.depth, CopyFromParent, xp->vinfo.visual, value_mask, &xswa);
    return w;
}

void
InstallThisColormap( display, window, cmap )
Display	*display;
Window	window;
Colormap cmap;
{
	XSetWindowColormap( display, window, cmap );
	if ( WMSafe == False )
		XInstallColormap( display, cmap );
}

void
InstallDefaultColormap( display, window )
Display	*display;
Window	window;
{
	XSetWindowColormap( display, window, 
		DefaultColormap( display, DefaultScreen( display ) ) );
	if ( WMSafe == False )
		XInstallColormap( display, 
			DefaultColormap( display, DefaultScreen( display ) ) );
}

void
InstallCustomColormap( display, window )
Display	*display;
Window	window;
{
	XSetWindowColormap( display, window, cmap );
	if ( WMSafe == False )
		XInstallColormap( display, cmap );
}

void CreateClipWindows(xp, clips)
    XParms  xp;
    int     clips;
{
    int j;
    XWindowAttributes    xwa;

    (void) XGetWindowAttributes(xp->d, xp->w, &xwa);
    if (clips > MAXCLIP) clips = MAXCLIP;
    for (j = 0; j != clips; j++) {
	clipWindows[j] = CreatePerfWindow(xp, xwa.x + ws[j].x, 
		xwa.y + ws[j].y, ws[j].width, ws[j].height);
    }
} /* CreateClipWindows */


void DestroyClipWindows(xp, clips)
    XParms  xp;
    int     clips;
{
    int j;

    if (clips > MAXCLIP) clips = MAXCLIP;
    for (j = 0; j != clips; j++) {
	XDestroyWindow(xp->d, clipWindows[j]);
    }
} /* DestroyClipWindows */


double DoTest(xp, test, reps)
    XParms  xp;
    Test    *test;
    int     reps;
{
    double  time;
    unsigned int ret_width, ret_height;

    /* Tell screen-saver to restart counting again.  See comments below for the
       XSetScreenSaver call. */
    XForceScreenSaver(xp->d, ScreenSaverReset);
    if ( dontClear == False )
	XClearWindow( xp->d, xp->w );
    HardwareSync (xp); 
    InitTimes ();
    (*test->proc) (xp, &test->parms, reps);
    HardwareSync(xp);

    time = ElapsedTime(syncTime);
    if (drawToFakeServer)
        XQueryBestSize(xp->d, TileShape, tileToQuery,
		       32, 32, &ret_width, &ret_height);
    (*test->passCleanup) (xp, &test->parms);
    return time;
}


int CalibrateTest(xp, test, seconds, usecperobj)
    XParms  xp;
    Test    *test;
    int     seconds;
    double  *usecperobj;
{
#define goal    2500000.0   /* Try to get up to 2.5 seconds		    */
#define enough  2000000.0   /* But settle for 2.0 seconds		    */
#define tick      10000.0   /* Assume clock not faster than .01 seconds     */

    double  usecs;
    int     reps, didreps;  /* Reps desired, reps performed		    */
    int     exponent;

    /* Attempt to get an idea how long each rep lasts by getting enough
       reps to last more tan enough.  Then scale that up to the number of
       seconds desired.

       If init call to test ever fails, return False and test will be skipped.
    */

    if (fixedReps != 0) {
	return fixedReps;
    }
    reps = 1;
    for (;;) {
	XDestroySubwindows(xp->d, xp->w);
	XClearWindow(xp->d, xp->w);
	didreps = (*test->init) (xp, &test->parms, reps);
	if (didreps == 0) {
	    return 0;
	}
	/* Create clip windows if requested */
	CreateClipWindows(xp, test->clips);
    	if ( dontClear == False )
		XClearWindow( xp->d, xp->w );
	HardwareSync(xp);
	InitTimes();
	(*test->proc) (xp, &test->parms, reps);
	HardwareSync(xp);
	usecs = ElapsedTime(syncTime);
	(*test->cleanup) (xp, &test->parms);
	DestroyClipWindows(xp, test->clips);

	if (didreps != reps) {
	    /* The test can't do the number of reps as we asked for.  
	       Give up */
	    *usecperobj = 
		usecs / (double)(didreps * test->parms.objects);
	    return didreps;
	}
	/* Did we go long enough? */
	if (usecs >= enough) break;

	/* Don't let too short a clock make new reps wildly high */
	if (usecs < tick) usecs = tick;

	/* Try to get up to goal seconds. */
	reps = (int) (goal * (double)reps / usecs) + 1;
    }

    *usecperobj = usecs / (double) (reps * test->parms.objects);
    reps = (int) ((double)seconds * 1000000.0 * (double)reps / usecs) + 1;

    /* Now round reps up to 1 digit accuracy, so we don't get stupid-looking
       numbers of repetitions. */
    reps--;
    exponent = 1;
    while (reps > 9) {
	reps /= 10;
	exponent *= 10;
    }
    reps = (reps + 1) * exponent;
    return reps;
} /* CalibrateTest */

void CreatePerfGCs(xp, func, pm)
    XParms  xp;
    int     func;
    unsigned long   pm;
{
    XGCValues gcvfg, gcvbg;
    unsigned long	fg, bg;

    fg = xp->foreground;
    bg = xp->background;
    gcvfg.graphics_exposures = False;
    gcvbg.graphics_exposures = False;
    gcvfg.plane_mask = pm;
    gcvbg.plane_mask = pm;
    gcvfg.function = func;
    gcvbg.function = func;
    
    if (func == GXxor) {
	/* Make test look good visually if possible */
	gcvbg.foreground = gcvfg.foreground = bg ^ fg;
	gcvbg.background = gcvfg.background = bg;
    } else {
	gcvfg.foreground = fg;
	gcvfg.background = bg;
	gcvbg.foreground = bg;
	gcvbg.background = fg;
    }
    xp->fggc = XCreateGC(xp->d, xp->w,
	GCForeground | GCBackground | GCGraphicsExposures
      | GCFunction | GCPlaneMask, &gcvfg);
    xp->bggc = XCreateGC(xp->d, xp->w, 
	GCForeground | GCBackground | GCGraphicsExposures
      | GCFunction | GCPlaneMask, &gcvbg);

}

void DestroyPerfGCs(xp)
XParms	xp;
{
    XFreeGC(xp->d, xp->fggc);
    XFreeGC(xp->d, xp->bggc);
}

void DisplayStatus(d, message, test)
    Display *d;
    char    *message;
    char    *test;
{
    char    s[500];

    XClearWindow(d, status);
    sprintf(s, "%s %s", message, test);
    /* We should really look at the height, descent of the font, etc. but
       who cares.  This works. */
    XDrawString(d, status, tgc, 10, 13, s, strlen(s));
}


void ProcessTest(xp, test, func, pm, label, locreps)
    XParms  xp;
    Test    *test;
    int     func;
    unsigned long   pm;
    char    *label;
    int	    locreps;
{
    double  time, totalTime;
    int     reps;
    int     j;

    CreatePerfGCs(xp, func, pm);
    if ( showLabels == True )
    {
	    fprintf( stderr, "'%s'\n", label );
	    fflush( stderr );
    }
    if ( loCal == False )
    {
	    DisplayStatus(xp->d, "Calibrating", label);
	    reps = CalibrateTest(xp, test, seconds, &time);
    }
    else
    {
    	if (fixedReps != 0)
            reps = fixedReps;
	else
	    reps = 5;
    }
    if ( locreps != -1 )
	reps = locreps;
    if (reps != 0) {
	DisplayStatus(xp->d, "Testing", label);
	XDestroySubwindows(xp->d, xp->w);
	XClearWindow(xp->d, xp->w);

	/* scale down the reps if needed */

	reps = (*test->init) (xp, &test->parms, reps);
	/* Create clip windows if requested */
	CreateClipWindows(xp, test->clips);

	totalTime = 0.0;
	for (j = 0; j != repeat; j++) {
    
	    time = DoTest(xp, test, reps);
	    totalTime += time;
	    ReportTimes (time, reps * test->parms.objects,
		    label, False);
	}
	if (repeat > 1) {
	    ReportTimes(totalTime,
		repeat * reps * test->parms.objects,
		label, True);
	}
	(*test->cleanup) (xp, &test->parms);
	DestroyClipWindows(xp, test->clips);
    } else {
	/* Test failed to initialize properly */
    }
    printf ("\n");
    fflush(stdout);
    DestroyPerfGCs(xp);
} /* ProcessTest */


main(argc, argv)
    int argc;
    char **argv;

{
    int		i, j, n, skip;
    int		numTests;       /* Even though the linker knows, we don't. */
    char	hostname[100];
    char	*displayName;
    Bool	foundOne = False;
    Bool	synchronous = False;
    XGCValues	tgcv;
    int		screen;
    int		rop, pm;
    XVisualInfo *vinfolist, vinfotempl;
    unsigned long vmask;
    XieExtensionInfo        *xieInfo;
    XWindowAttributes attribs;
    XEvent  event;
    int	done, cclass, timeout;

    CacheInit();
    dontClear = False;
    /* Save away argv, argc, for usage to print out */
    saveargc = argc;
    saveargv = (char **) malloc(argc * sizeof(char *));
    for (i = 0; i != argc; i++) {
	saveargv[i] = argv[i];
    }

    xparms.pack = False;
    xparms.version = 1;

    /* Count number of tests */
    ForEachTest(numTests);
    doit = (Bool *)calloc(numTests, sizeof(Bool));

    /* Parse arguments */
    program_name = argv[0];
    xparms.displayName = displayName = Get_Display_Name (&argc, argv);
    for (i = 1; i != argc; i++) {
	if (strcmp (argv[i], "-all") == 0) {
	    ForEachTest (j)
		doit[j] = True;
	    foundOne = True;
	} else if (strcmp (argv[i], "-labels") == 0) {
	    labels = True;
	} else if (strcmp(argv[i], "-range") == 0) {
	    char *cp1;
	    char *cp2;
	    
	    if (argc <= ++i)
		usage();
	    cp1 = argv[i];
	    if (*cp1 == '-')
		cp1++;
	    for (cp2 = cp1; *cp2 != '\0' && *cp2 != ','; cp2++) {};
	    if (*cp2 == ',') {
		*cp2++ = '\0';
		if (*cp2 == '-')
		    cp2++;
	    } else {
		cp2 = "-";
	    }
	    ForEachTest (j) {
		if (strcmp (cp1, (test[j].option) + 1) == 0 &&
		    (test[j].versions & xparms.version)) {
		    int k = j;
		    do {
			doit[k] = True;
		    } while (strcmp(cp2, (test[k].option + 1)) != 0 &&
			     (test[k].versions & xparms.version) &&
			     test[++k].option != NULL);
		    if (*cp2 != '-' && test[k].option == NULL)
			usage();
		    break;
		}
	    }
	    if (test[j].option == NULL)
		usage();
	    foundOne = True;
	} else if (strcmp (argv[i], "-sync") == 0) {
	    synchronous = True;
	} else if (strcmp (argv[i], "-images") == 0) {
		i++;
		if (argc <= i )
			usage();
		imagepath = argv[i];
	} else if (strcmp (argv[i], "-timeout") == 0) {
	    i++;
	    if (argc <= i)
		usage ();
	    timeout = atoi (argv[i]);
	    if (timeout < 0)
	       usage ();
	    else
	       SetTimeout( timeout );
	    
	} else if (strcmp (argv[i], "-repeat") == 0) {
	    i++;
	    if (argc <= i)
		usage ();
	    repeat = atoi (argv[i]);
	    if (repeat <= 0)
	       usage ();
	} else if (strcmp (argv[i], "-time") == 0) {
	    i++;
	    if (argc <= i)
		usage ();
	    seconds = atoi (argv[i]);
	    if (seconds <= 0)
	       usage ();
	} else if (strcmp(argv[i], "-reps") == 0) {
	    i++;
	    if (argc <= i)
		usage ();
	    fixedReps = atoi (argv[i]);
	    if (fixedReps <= 0)
		usage ();
        } else if (strcmp(argv[i], "-depth") == 0) {
            i++;
            if (argc <= i)
                usage ();
            depth = atoi(argv[i]);
            if (depth <= 0)
                usage ();
        } else if (strcmp(argv[i], "-script") == 0) {
            i++;
	    loadTests = True;
            if (argc <= i)
		usage();
            loadTestsFile = argv[i];
	    foundOne = True;
        } else if (strcmp(argv[i], "-cache") == 0) {
            i++;
            if (argc <= i)
                usage ();
            CacheSizeMax = atoi(argv[i]);
            if (CacheSizeMax < 0)
                usage ();
	} else if (strcmp(argv[i], "-loCal") == 0) {
		loCal = True;
	} else if (strcmp(argv[i], "-showlabels") == 0) {
		showLabels = True;
	} else if (strcmp(argv[i], "-events") == 0) {
		runEvents = True;
	} else if (strcmp(argv[i], "-errors") == 0) {
		runErrors = True;
	} else if (strcmp(argv[i], "-showevents") == 0) {
		showErrors = True;
	} else if (strcmp(argv[i], "-lessboring") == 0) {
		notBoring = True;
	} else if (strcmp(argv[i], "-GrayScale") == 0) {
		visualClass = GrayScale;
	} else if (strcmp(argv[i], "-StaticGray") == 0) {
		visualClass = StaticGray;
	} else if (strcmp(argv[i], "-PseudoColor") == 0) {
		visualClass = PseudoColor;
	} else if (strcmp(argv[i], "-StaticColor") == 0) {
		visualClass = StaticColor;
	} else if (strcmp(argv[i], "-DirectColor") == 0) {
		visualClass = DirectColor;
	} else if (strcmp(argv[i], "-TrueColor") == 0) {
		visualClass = TrueColor;
	} else if (strcmp(argv[i], "-DIS") == 0) {
		class_request = SUBSET_DIS;
	} else if (strcmp(argv[i], "-WMSafe") == 0) {
		WMSafe = True;
	} else if (strcmp(argv[i], "-tests") == 0) {
		j = 0;
    		while (test[j].option != NULL) {
			fprintf(stderr, "    %-24s   %s\n",
				test[j].option, test[j].label);
			j++;
		}
		exit(0);
	} else {
	    ForEachTest (j) {
		if (strcmp (argv[i], test[j].option) == 0 &&
		    (test[j].versions & xparms.version)) {
		    doit[j] = True;
		    goto LegalOption;
		}
	    }
	    usage ();
	LegalOption: 
		foundOne = True;
	}
    }

    if (labels) {
	/* Just print out list of tests for use with .sh programs that
	   assemble data from different xieperf runs into a nice format */
	ForEachTest (i) {
	    int child;
	    if (doit[i] && (test[i].versions & xparms.version)) {
		printf ("%s\n", test[i].label);
	    }
	}
	exit(0);
    }

    if (!foundOne)
	usage ();
    if ( loadTests == True )
    {
	if ( !strcmp( loadTestsFile, "-" ) )
		fp = stdin;
	else
		fp = fopen( loadTestsFile, "r" );
	if ( fp == ( FILE * ) NULL )
	{
		fprintf( stderr, "Couldn't open script file '%s'\n",
			loadTestsFile );
		fprintf( stderr, "script argument will be ignored\n" );
		loadTests = False;
	}
    }

    xparms.d = Open_Display (displayName);
    screen = DefaultScreen(xparms.d);

    /* check for presence of XIE */

    printf("xieperf - XIE performance program, beta version 0.95\n");
    if ( !XieInitialize(xparms.d, &xieInfo ) ) {
	printf("\nXIE not supported on this display!\n");
        exit(1);
    }
    printf("\nXIE V%d.%d\n", xieInfo->server_major_rev, 
	xieInfo->server_minor_rev);

    capabilities = ( xieInfo->service_class << 8 );	
    if ( IsDIS( capabilities ) )
	printf( "DIS server\n" );
    else
	printf( "FULL server\n" );

    if ( IsFull( capabilities ) && IsDIS( class_request ) )
    {
	printf( "Service class is DIS - running DIS tests only\n" );
	capabilities = SUBSET_DIS;
    }

    InitEventInfo( &xparms, xieInfo );
    /* get visual info of default visual */
    vmask = VisualIDMask | VisualScreenMask;
    vinfotempl.visualid = XVisualIDFromVisual(XDefaultVisual(xparms.d, screen));
    vinfotempl.screen = screen;
    vinfolist = XGetVisualInfo(xparms.d, vmask, &vinfotempl, &n);
    if (!vinfolist || n != 1) {
	fprintf (stderr, "%s: can't get visual info of default visual\n",
	    program_name);
	exit(1);
    }

    if (depth == -1 && visualClass == -1) {
        /* use the default visual and colormap */
        xparms.vinfo = *vinfolist;
	free( vinfolist );
    } else {
        /* find the specified visual */
	vmask = VisualScreenMask;
        vinfotempl.screen = screen;
	if ( depth != -1 )
	{
		vmask |= VisualDepthMask;
        	vinfotempl.depth = depth;
	}
	if ( visualClass != -1 )
	{
		vmask |= VisualClassMask;
#if	defined(__cplusplus) || defined(c_plusplus)
		vinfotempl.c_class = visualClass;
#else
		vinfotempl.class = visualClass; 
#endif
	}
	if ( vinfolist )
		free( vinfolist );
        vinfolist = XGetVisualInfo(xparms.d, vmask, &vinfotempl, &n);
        if (!vinfolist) {
            fprintf (stderr, 
		"%s: can't find an appropriate visual for requested depth and/or class\n", program_name);
            exit(1);
        }
        xparms.vinfo = *vinfolist;  /* use the first one in list */
	free( vinfolist );
    }

#if	defined(__cplusplus) || defined(c_plusplus)
    cclass = xparms.vinfo.c_class;
#else
    cclass = xparms.vinfo.class;
#endif
    cmap = XCreateColormap(xparms.d, DefaultRootWindow(xparms.d),
	xparms.vinfo.visual, AllocNone );
    if ( WMSafe == False )
	XInstallColormap( xparms.d, cmap );
    if ( cclass == GrayScale || cclass == PseudoColor || cclass == DirectColor )
    { 
    	AllocateRest( &xparms );
	xparms.foreground = 0;
    	xparms.background = maxcoloridx;
    }
    else
    {
	xparms.foreground = BlackPixel( xparms.d, DefaultScreen( xparms.d ) );
	xparms.background = WhitePixel( xparms.d, DefaultScreen( xparms.d ) );
    }

    if (!foreground) foreground = "Black";
    if (!background) background = "White";

#ifndef VMS
    gethostname (hostname, 100);
    printf ("%s server on %s\nfrom %s\n",
	    ServerVendor (xparms.d), DisplayString (xparms.d), hostname);
#else
    printf ("%s server on %s\n",
	    ServerVendor (xparms.d), DisplayString (xparms.d));
#endif
    PrintTime ();

    /* Force screen out of screen-saver mode, grab current data, and set
       time to blank to 8 hours.  We should just be able to turn the screen-
       saver off, but this causes problems on some servers.  We also reset
       the screen-saver timer each test, as 8 hours is about the maximum time
       we can use, and that isn't long enough for some X terminals using a
       serial protocol to finish all the tests.  As long as the tests run to 
       completion, the old screen-saver values are restored. */
    XForceScreenSaver(xparms.d, ScreenSaverReset);
    XGetScreenSaver(xparms.d, &ssTimeout, &ssInterval, &ssPreferBlanking,
	&ssAllowExposures);
    (void) signal(SIGINT, Cleanup); /* ^C */
    (void) signal(SIGQUIT, Cleanup);
    (void) signal(SIGTERM, Cleanup);
    (void) signal(SIGHUP, Cleanup);
    XSetScreenSaver(xparms.d, 8 * 3600, ssInterval, ssPreferBlanking, 
	ssAllowExposures);

    if (drawToFakeServer) {
        tileToQuery =
	    XCreatePixmap(xparms.d, DefaultRootWindow (xparms.d), 32, 32, 1);
    }
    if ( WMSafe == True )
    {
	xparms.p = CreateXIEParent( &xparms );
    	XSetTransientForHint( xparms.d, xparms.p, DefaultRootWindow (xparms.d) );
	XMapRaised( xparms.d, xparms.p );
    }
    else
	xparms.p = DefaultRootWindow( xparms.d );
    xparms.w = CreatePerfWindowUnmapped(&xparms, 2, 2, WIDTH, HEIGHT);
    if ( WMSafe == True )
    	XSetTransientForHint( xparms.d, xparms.w, xparms.p );
    monitorWindow = CreatePerfWindowUnmapped(&xparms, WIDTH - 100, 
	HEIGHT - 100, MONWIDTH, MONHEIGHT);
    if ( WMSafe == True )
    	XSetTransientForHint( xparms.d, monitorWindow, xparms.p );
    monitor2Window = CreatePerfWindowUnmapped(&xparms, WIDTH - 100, 
	HEIGHT - 100, MONWIDTH, MONHEIGHT);
    if ( WMSafe == True )
    	XSetTransientForHint( xparms.d, monitor2Window, xparms.p );

    drawableWindow = CreatePerfWindowUnmapped(&xparms, 610, 0, 
	WIDTH, HEIGHT);
    if ( WMSafe == True )
    	XSetTransientForHint( xparms.d, drawableWindow, xparms.p );
    status = CreatePerfWindowUnmapped(&xparms, 2, HEIGHT+5, WIDTH, 20);
    if ( WMSafe == True )
    	XSetTransientForHint( xparms.d, status, xparms.p );
    tgcv.foreground = xparms.foreground;
    tgcv.background = xparms.background;
    tgc = XCreateGC(xparms.d, status, GCForeground | GCBackground, &tgcv);

    if (synchronous)
	XSynchronize (xparms.d, True);

    /* Get mouse pointer out of the way of the performance window.  On
       software cursor machines it will slow graphics performance.  On
       all current MIT-derived servers it will slow window 
       creation/configuration performance. */
    XGetWindowAttributes( xparms.d, DefaultRootWindow( xparms.d ), &attribs );
    XSync( xparms.d, 0 );

    /* wait for first expose event before we trudge forward */

    XSelectInput( xparms.d, xparms.w, ExposureMask );
    XSync( xparms.d, 0 );
    XMapWindow( xparms.d, xparms.w );
    XMapWindow( xparms.d, status );

    done = 0;
    while( done == 0 )
    {
	XNextEvent( xparms.d, &event );
        switch( event.type )
	{
		case Expose:
			done = 1;
			break;
	}
    }

    XWarpPointer(xparms.d, None, xparms.p, 0, 0, 0, 0, 10, HEIGHT+30);

    /* Figure out how long to call HardwareSync, so we can adjust for that
       in our total elapsed time */

    (void) CalibrateTest(&xparms, &syncTest, 1, &syncTime);
    printf("Sync time adjustment is %6.4f msecs.\n\n", syncTime/1000);

    ListAllTechs( &xparms );

    if ( loadTests == False )
    {
    	ForEachTest (i) {
		int child;
		char label[200];

		if ( !EventOrErrorValid( test[ i ].parms.description ) )
			continue;
		if (doit[i] && (test[i].versions & xparms.version) && ServerIsCapable( test[ i ].parms.description ) ) {
			if ( IsNotBoring( test[ i ].parms.description ) )
			{
				strcpy (label, test[i].label);
				ProcessTest(&xparms, &test[i], GXcopy, ~0, label, -1);
			}
		} /* if doit */
	    } /* ForEachTest */
    }
    else
    {
	Bool 	done;
	int 	repeattmp, 
		repeatsave;
	int 	reps;
	done = False;

	while ( done == False )
	{
		repeattmp = -1;
		if ( ( i = GetNextTest( fp, &repeattmp, &reps ) ) < 0 )
			done = True;
		else
		{
			int child;
			char label[200];

			if ( !EventOrErrorValid( test[ i ].parms.description ) )
				continue;
			if ((test[i].versions & xparms.version) && 
				ServerIsCapable( test[ i ].parms.description ) )
			{
				if ( IsNotBoring( test[ i ].parms.description ) )
				{
					strcpy (label, test[i].label);
					if ( repeattmp != -1 )
					{
						repeatsave = repeat;
						repeat = repeattmp;
					}
					ProcessTest(&xparms, 
						&test[i], GXcopy, ~0, label, reps);
					if ( repeattmp != -1 )
						repeat = repeatsave;
				}
		        }
		}
	}
    }

    if ( fp != ( FILE * ) NULL )
	fclose( fp );
    ReclaimPhotomapMemory();	/* shut up Purify :-) */
    XDestroyWindow(xparms.d, xparms.w);
    XDestroyWindow(xparms.d, xparms.p );
    /* Restore ScreenSaver to original state. */
    XSetScreenSaver(xparms.d, ssTimeout, ssInterval, ssPreferBlanking,
	ssAllowExposures);
    XCloseDisplay(xparms.d);
}

int
TestIndex ( testname )
char	*testname;
{
	int	j, found;

	found = -1;
	ForEachTest (j) 
	{
       		if (strcmp (testname, (test[j].option) + 1) == 0 )
		{
			found = j;
			break;
		}
	}
	return( found );
}

Window
CreateXIEParent( xp )
XParms	xp;
{
	Window root, ret;
	unsigned int width, height, border_width, depth;
    	XSetWindowAttributes xswa;
	int x, y;
	int hasBS;
	unsigned long value_mask;

/* 
	Time for a little explaining...

	Xieperf puts up windows, moves them around, and associates
	colormaps with windows. If no window manager is running, then 
	things are fine - I don't even call this function. I just set 
	the window attribute override_redirect = True at window creation 
	time, and associate my grayscale colormap with the window at 
	creation time also.

	If a window manager is running, I need to be nice. Nice means
	allow the window manager to install colormaps as my windows
	gain focus, and let the manager control any geometry changes
	that I issue. 

	The user, when a window manager is running, is required to use 
	the -WMSafe argument to xieperf to indicate to me that a window
	manager is running. If this arg is given, then I do the following:

	I make a child window of the root window which acts as the parent 
	window for xieperf's windows. I use XSetTransientForHint() to tell
	the window manager essentially that each child window of this 
	parent is a transient window so please don't mess with my geometry
	requests and please install the colormap associated with each
	child window as that window gains focus.

	Since the parent window has dimensions equal to the root window,
	the app will have focus and my colormap ( which is also the color 
	map of the parent window ) will be installed ( assuming the
	graciousness of the window manager ). Also, any child windows will 
	appear, disappear, and be moved at my command. Finally, it was
	my goal to make the parent window transparent, so it has the usual 
	root window appearance. This could be done by calling the function
	XSetWindowBackgroundPixmap(), and specifying ParentRelative as its
	final argument.

	But I ran into problems. I got the desired effects - namely window
	placement, colormap handling, and the transparency thing all seemed
	o.k., but under twm my parent window would not clear itself when
	child windows were unmapped, nor would it respond to XClearWindow()
	calls. Turning backing store on for the parent window fixed the
	problem, but that is not a viable solution since backing store isn't
	always around to help. And, when running under mwm, bizarro effects 
	like side-by-side mirror images of a child window would appear. It
        was dazzling, but unappreciated...

	My solution was simple. If this function is called, and the server 
	currently supports backing store, I use that, go for the transparent
	window, and things are happy.

	If this function is called and there is no backing store support, 
	instead of having a transparent parent window, the parent window is 
	created with a background pixel of BlackPixel ( also it was shown
	that while the transparent version of the parent window never cleared 
	either on its own or explicitly via XClearWindow(), windows that had
	normal pixel-based backgrounds would clear themselves whenever the 
	child windows were unmapped ).

	Comments would be appreciated... there should be a way to do this
	that isn't too painful. The Xlib Programming Manual ( O'Reilly
	Volume 1, p 96 ) states that if ParentRelative is used as a 
	backround pixmap, the backgrounds are automatically repainted on
	exposure. As the above states, they aren't. My feeling is that there 
        is a bug in the R5 cfb code or...
*/ 

	hasBS = DoesBackingStore( DefaultScreenOfDisplay( xp->d ) );
	XGetGeometry( xp->d, DefaultRootWindow( xp->d ), &root, &x, &y,
		&width, &height, &border_width, &depth );

    	xswa.colormap = cmap;
	if ( hasBS )
	{
		xswa.backing_store = WhenMapped;
	}
	else
	{
		xswa.background_pixel = 
			BlackPixel( xp->d, DefaultScreen( xp->d ) );
	}
	value_mask = CWColormap | ( hasBS ? CWBackingStore : CWBackPixel );
	ret = XCreateWindow( xp->d, DefaultRootWindow( xp->d ),
		x, y, width, height, border_width, xp->vinfo.depth, 
		CopyFromParent, xp->vinfo.visual, 
		value_mask, &xswa );

	if ( hasBS )
	{
		/* make it transparent */

		XSetWindowBackgroundPixmap( xp->d, ret, ParentRelative );	
	}
	return( ret );
}

void
AllocateRest( xp )
XParms	xp;
{
	int	i, n_colors;
	long	intensity;
	unsigned long *pixels;
	XColor	*gray;

	/* get what we can, and remember it. we should get all since
	   we created the colormap */

	i = ( 1 << xp->vinfo.depth );
	pixels = ( unsigned long * ) malloc( sizeof( unsigned long ) * i );
	if ( pixels == ( unsigned long * ) NULL )
	{
		fprintf( stderr, 
			"Couldn't allocate pixel vector for XAllocColorCells\n" );
		exit( 1 );
	}

	gray = ( XColor * ) malloc( sizeof( XColor ) * i );
	if ( gray == ( XColor * ) NULL )
	{
		fprintf( stderr, "Couldn't allocate XColor vector for XAllocColorCells\n" );
		free( pixels );
		exit( 1 );
	}
	
	n_colors = 0;
	while ( i ) 
	{
		if ( XAllocColorCells( xp->d, cmap, True, NULL, 0, pixels, i ) )
		{
			n_colors = i;
			break;
		}
		i--;
	}	

	/* do a grey scale ramp */

	maxcoloridx = n_colors - 1;

       	for (i=0; i<n_colors; i++) {
                intensity = (i*65535L) / (long) (n_colors-1);
                gray[i].pixel = pixels[i];
                gray[i].red   = intensity;
                gray[i].green = intensity;
                gray[i].blue  = intensity;
                gray[i].flags = DoRed | DoGreen | DoBlue;
        }
	if ( n_colors )
	{
        	XStoreColors(xp->d,cmap,gray,n_colors);
        	XSync(xp->d,0);
	}	
	else
	{
		fprintf( stderr, "Couldn't allocate colors in colormap\n" );
		free( pixels );
		free( gray );
		exit( 0 );
	}
	free( pixels );
	free( gray );
}

int
GetWords (argi, argc, argv, wordsp, nump)
    int     argi;
    int     argc;
    char    **argv;
    char    **wordsp;
    int     *nump;
{
    int	    count;

    if (argc <= argi)
	usage();
    count = 0;
    while (argv[argi] && *(argv[argi]) != '-') {
	*wordsp++ = argv[argi];
	++argi;
	count++;
    }
    *nump = count;
    return count;
}

static int
atox (s)
    char    *s;
{
    int     v, c;

    v = 0;
    while (*s) {
	if ('0' <= *s && *s <= '9')
	    c = *s - '0';
	else if ('a' <= *s && *s <= 'f')
	    c = *s - 'a' + 10;
	else if ('A' <= *s && *s <= 'F')
	    c = *s - 'A' + 10;
	v = v * 16 + c;
	s++;
    }
    return v;
}

int GetNumbers (argi, argc, argv, intsp, nump)
    int     argi;
    int     argc;
    char    **argv;
    int     *intsp;
    int     *nump;
{
    char    *words[256];
    int	    count;
    int	    i;
    int	    flip;

    count = GetWords (argi, argc, argv, words, nump);
    for (i = 0; i < count; i++) {
	flip = 0;
	if (!strncmp (words[i], "~", 1)) {
	    words[i]++;
	    flip = ~0;
	}
	if (!strncmp (words[i], "0x", 2))
	    intsp[i] = atox(words[i] + 2) ^ flip;
	else
	    intsp[i] = atoi (words[i]) ^ flip;
    }
    return count;
}

int	
SendTripleBandPlaneDataSequential( xp, p, flo_id, photospace, element, data, 
	size, pixel_stride, left_pad, scanline_pad, width, height )
XParms	xp;
Parms	p;
int	flo_id;
XiePhotospace photospace;
int	element;
char	*data;
int	size;
unsigned char pixel_stride[ 3 ];
unsigned char left_pad[ 3 ];
unsigned char scanline_pad[ 3 ];
XieLTriplet width, height;
{
	void	ScanlinePad();
	int	band1, band2, band3;

	/* calculate band sizes */

	band1 = ( ( left_pad[ 0 ] >> 3 ) + 
		( width[ 0 ] * ( pixel_stride[ 0 ] >> 3 ) ) ) * height[ 0 ];
	ScanlinePad( &band1, scanline_pad[ 0 ] );
	band2 = ( ( left_pad[ 1 ] >> 3 ) + 
		( width[ 1 ] * ( pixel_stride[ 1 ] >> 3 ) ) ) * height[ 1 ];
	ScanlinePad( &band2, scanline_pad[ 1 ] );
	band3 = ( ( left_pad[ 2 ] >> 3 ) + 
		( width[ 2 ] * ( pixel_stride[ 2 ] >> 3 ) ) ) * height[ 2 ];
	ScanlinePad( &band3, scanline_pad[ 0 ] );

	/* fire away! */

	PumpTheClientData( xp, p, flo_id, photospace, element, data, band1, 0 );
	data += band1;
	PumpTheClientData( xp, p, flo_id, photospace, element, data, band2, 1 );
	data += band2;
	PumpTheClientData( xp, p, flo_id, photospace, element, data, band3, 2 );
	data += band3;
}

void
ScanlinePad( value, pad )
int	*value;
int	pad;
{
	int	newval;

	if ( !( *value == 0 || pad == 0 || pad == 1 ) )
	{
		if ( ( *value % pad ) != 0 )
		{
			if ( *value < pad )
			{
				*value = pad;
			}
			else
			{
				newval = pad;
				while ( newval < *value )
					newval += pad;
				*value = newval;
			}
		}
	}
}

int	
PumpTheClientData( xp, p, flo_id, photospace, element, data, size, band_number )
XParms	xp;
Parms	p;
int	flo_id;
XiePhotospace photospace;
int	element;
char	*data;
int	size;
int	band_number;
{
	int	bytes_left, final, nbytes;

	final = 0;
	bytes_left = size;
        while (bytes_left > 0) {
                nbytes = (bytes_left > p->buffer_size) ?
                        p->buffer_size: bytes_left;
                if (nbytes >= bytes_left)
                        final = 1;
                XiePutClientData (
                        xp->d,
                        photospace,
                        flo_id,
                        element,        /* element */
                        final,          /* signal that this is all the data */
                        band_number,    /* 0 for all but triple band data 
					   BandByPlane, which then may be
					   0, 1, or 2 */
                        (unsigned char *)data,
                        nbytes
                );
                bytes_left -= nbytes;
                data += nbytes;
        }
}

int	
ReadNotifyExportData( xp, p, namespace, flo_id, element, elementsz, 
	numels, data, done )
XParms	xp;
Parms	p;
unsigned long namespace;
int	flo_id;
XiePhototag element;
unsigned int elementsz;
unsigned int numels;
char	**data;
int	*done;
{
	char	*cp, *ptr;
	Bool	terminate = False;
	XieExportState new_state_ret;
	unsigned int nbytes;
	unsigned int nbytes_ret;
	Bool	no_errors, reallocFlag;
	unsigned int cnt;
	int bytes;

	*done = 0;
	if ( numels == 0 )
	{
		numels = WaitForXIEEvent( xp, xieEvnNoExportAvailable,
			flo_id, element, False );
		if ( numels == 0 )
			return( -1 );
	}

	nbytes = numels * elementsz; 	
	bytes = nbytes;
	no_errors = True;
	reallocFlag = False;
	cnt = 0;
	if ( *data == ( char * ) NULL )
	{
		if ( ( *data = ( char * ) malloc( nbytes ) ) == ( char * ) NULL )
		{
			fprintf( stderr, "ReadNotifyExportData: couldn't allocate data\n" );
			return( 0 );
		}
	}
	ptr = *data;
	while ( 1 )		
	{  
        	XieGetClientData ( xp->d, namespace, flo_id, element,  
			bytes > 2048 ? 2048 : bytes, terminate, 0,
			&new_state_ret, (unsigned char **)&cp, &nbytes_ret );

		if ( nbytes_ret && reallocFlag )
		{
			*data = realloc( *data, cnt + nbytes_ret );
			if ( *data == ( char * ) NULL )
			{
				/* oh no */
					
				fprintf( stderr, "ReadNotifyExportData: realloc failed\n" );	
				no_errors = False;
				break;
			}	
			ptr = *data + cnt;
		}

/* this rots */

		memcpy( ptr, cp, nbytes_ret * sizeof( char ) ); 
		ptr += nbytes_ret * sizeof( char );
		free( cp );

		cnt += nbytes_ret;	
		bytes -= nbytes_ret;
		if ( new_state_ret == xieValExportEmpty )
			WaitForXIEEvent( xp, xieEvnNoExportAvailable,
				flo_id, element, False );
		else if ( new_state_ret == xieValExportMore ) 
		{
			if ( bytes <= 0 )
				reallocFlag = True;
			bytes = 2048;
		}
		else if ( new_state_ret == xieValExportDone ) 
		{
			*done = 1;
			break;
		}
		else if ( new_state_ret == xieValExportError )
		{
			fprintf( stderr, "ReadNotifyExportData: xieValExportError received from XieGetClientData\n" );
			no_errors = False;
			break;
		}

		if ( bytes <= 0 && reallocFlag == False )
		{
			/* if we get here, we didn't get an ExportDone,
			   and we have no buffer space left. So turn on 
			   the realloc flag. Also, server could be not
                           sending the ExportDone for some reason...
			   if so XIE or xieperf or both may be broken. */
			
			bytes = 2048;
			reallocFlag = True;
		}
	}
	if ( no_errors == False )
	{
		return( -1 );
	}
	return( cnt );
}

int	
ReadNotifyExportTripleData( xp, p, namespace, flo_id, element, elementsz, 
	numels, data, done )
XParms	xp;
Parms	p;
unsigned long namespace;
int	flo_id;
XiePhototag element;
unsigned int elementsz;
unsigned int numels;
char	**data;
int	*done;
{
	char	*cp, *ptr;
	Bool	terminate = False;
	XieExportState new_state_ret;
	unsigned int nbytes;
	unsigned int nbytes_ret;
	Bool	no_errors, reallocFlag;
	unsigned int cnt;
	int bytes;
	int band;

	*done = 0;
	if ( numels == 0 )
	{
		numels = WaitForXIEEvent( xp, xieEvnNoExportAvailable,
			flo_id, element, False );
		if ( numels == 0 )
			return( -1 );
		fprintf( stderr, "There are %d elements available\n", numels );
	}

	nbytes = numels * elementsz; 	
	bytes = nbytes;
	no_errors = True;
	reallocFlag = False;
	cnt = 0;
	if ( *data == ( char * ) NULL )
	{
		if ( ( *data = ( char * ) malloc( nbytes ) ) == ( char * ) NULL )
		{
			fprintf( stderr, "ReadNotifyExportTripleData: couldn't allocate data\n" );
			return( 0 );
		}
	}
	ptr = *data;
	for ( band = 0; band < 3; band++ )
	{
		while ( 1 )		
		{  
			XieGetClientData ( xp->d, namespace, flo_id, element,  
				bytes > 2048 ? 2048 : bytes, terminate, band,
				&new_state_ret, (unsigned char **)&cp, 
				&nbytes_ret );

			if ( nbytes_ret && reallocFlag )
			{
				*data = realloc( *data, cnt + nbytes_ret );
				if ( *data == ( char * ) NULL )
				{
					/* oh no */
						
					fprintf( stderr, "ReadNotifyExportTripleData: realloc failed\n" );	
					no_errors = False;
					break;
				}	
				ptr = *data + cnt;
			}

			memcpy( ptr, cp, nbytes_ret * sizeof( char ) ); 
			ptr += nbytes_ret * sizeof( char );
			free( cp );

			cnt += nbytes_ret;	
			bytes -= nbytes_ret;
			if ( new_state_ret == xieValExportEmpty )
				WaitForXIEEvent( xp, xieEvnNoExportAvailable,
					flo_id, element, False );
			else if ( new_state_ret == xieValExportMore ) 
			{
				if ( bytes <= 0 )
					reallocFlag = True;
				bytes = 2048;
			}
			else if ( new_state_ret == xieValExportDone ) 
			{
				*done = 1;
				break;
			}
			else if ( new_state_ret == xieValExportError )
			{
				fprintf( stderr, "ReadNotifyExportTripleData: xieValExportError received from XieGetClientData\n" );
				no_errors = False;
				break;
			}
			if ( bytes <= 0 && reallocFlag == False )
			{
				bytes = 2048;
				reallocFlag = True;
			}
		}
	}
	if ( no_errors == False )
	{
		return( -1 );
	}
	return( cnt );
}

unsigned int
CheckSum( data, size )
char	*data;
unsigned int size;
{
	unsigned int sum, i;

	sum = 0;
	for ( i = 0; i < size; i++ )
	{
		sum += *(data++);
	}
	return( sum );
}

XiePhotomap
GetXIEFAXPhotomap( xp, p, which, radiometric )
XParms	xp;
Parms	p;
int	which;
Bool 	radiometric;
{
	XIEimage	*image;
        XiePhotospace photospace;
	int	flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieDecodeG42DParam *g42d_decode_params=NULL;
	XieDecodeG32DParam *g32d_decode_params=NULL;
	XieDecodeG31DParam *g31d_decode_params=NULL;
	XieDecodeTIFF2Param *tiff2_decode_params=NULL;
	XieDecodeTIFFPackBitsParam *tiffpb_decode_params=NULL;
	char *decode_params;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
	unsigned char pixel_stride[ 3 ];
	unsigned char left_pad[ 3 ];
	unsigned char scanline_pad[ 3 ];
	XiePhotomap	tmp;
	int	size;

	if ( which == 1 )
	{
		image = p->finfo.image1;

	}
	else if ( which == 2 )
	{
		image = p->finfo.image2;

	}
	else 
	{
		image = p->finfo.image3;
	}
	if ( !image )
		return( XiePhotomap ) NULL;
	
	if ( IsImageInCache( image ) == True )
	{
		return( PhotomapOfImage( image ) );
	}

	if ( !GetImageData( xp, p, which ) )
		return( ( XiePhotomap ) NULL );

	if ( TechniqueSupported( xp, xieValDecode, image->decode ) == False )
		return( XiePhotomap ) NULL;

	size = image->fsize;
	image->chksum = CheckSum( image->data, size );
	width[ 0 ] = image->width[ 0 ];
	height[ 0 ] = image->height[ 0 ];
	levels[ 0 ] = image->levels[ 0 ];

	/* create a photomap */

        tmp = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

	if ( image->decode == xieValDecodeG42D )  {
            g42d_decode_params = XieTecDecodeG42D(
                image->fill_order, 
		True,
		radiometric
            );
	    decode_params = (char *) g42d_decode_params;
	}
	else if ( image->decode == xieValDecodeG32D )  {
            g32d_decode_params = XieTecDecodeG32D(
                image->fill_order, 
		True,
		radiometric
            );
	    decode_params = (char *) g32d_decode_params;
	}
	else if ( image->decode == xieValDecodeTIFF2 )  {
            tiff2_decode_params = XieTecDecodeTIFF2(
                image->fill_order, 
		True,
		radiometric
            );
	    decode_params = (char *) tiff2_decode_params;
	}
	else if ( image->decode == xieValDecodeTIFFPackBits )  {
            tiffpb_decode_params = XieTecDecodeTIFFPackBits(
                image->fill_order, 
		True
            );
	    decode_params = (char *) tiffpb_decode_params;
	}
	else if ( image->decode == xieValDecodeG31D )  {
            g31d_decode_params = XieTecDecodeG31D(
                image->fill_order, 
		True,
		radiometric
            );
	    decode_params = (char *) g31d_decode_params;
	}
	else {
                return( XiePhotomap ) NULL;
	}

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEFAXPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, tmp );
		if ( decode_params )
			free( decode_params );
                return( XiePhotomap ) NULL;
        }

        XieFloImportClientPhoto(&flograph[0],
                image->bandclass,
                width, height, levels,
                False,
                image->decode, (char *)decode_params
        );

        XieFloExportPhotomap(&flograph[1],
                1,              /* source phototag number */
                tmp,
                encode_tech,
                encode_params
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                2               /* number of elements */
        );
        XSync( xp->d, 0 );
       	PumpTheClientData( xp, p, flo_id, photospace, 1, image->data, size, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,2);
        XieDestroyPhotospace(xp->d, photospace);
	free( decode_params );
	AddToCache( image, tmp );
    	return tmp;
}

XiePhotomap
GetXIETriplePhotomap( xp, p, which )
XParms	xp;
Parms	p;
int	which;
{
	XIEimage	*image;
        XiePhotospace photospace;
	int	flo_id, flo_notify;
        XiePhotoElement *flograph;
	char *decode_params=NULL;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
	unsigned char pixel_stride[ 3 ];
	unsigned char left_pad[ 3 ];
	unsigned char scanline_pad[ 3 ];
	XiePhotomap	tmp;
	int	size;

	if ( which == 1 )
	{
		image = p->finfo.image1; 
	}
	else if ( which == 2 )
	{
		image = p->finfo.image2; 
	}
	else 
	{
		image = p->finfo.image3; 
	}
	if ( !image )
		return( XiePhotomap ) NULL;

	if ( IsImageInCache( image ) == True )
	{
		TouchImage( image );
		return( PhotomapOfImage( image ) );
	}

	if ( !GetImageData( xp, p, which ) )
		return( ( XiePhotomap ) NULL );

	if ( TechniqueSupported( xp, xieValDecode, image->decode ) == False )
		return( XiePhotomap ) NULL;
	
	size = image->fsize;
	image->chksum = CheckSum( image->data, size );

	/* create a photomap */

        tmp = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

	width[ 0 ] = image->width[ 0 ];
	width[ 1 ] = image->width[ 1 ];
	width[ 2 ] = image->width[ 2 ];
	height[ 0 ] = image->height[ 0 ];
	height[ 1 ] = image->height[ 1 ];
	height[ 2 ] = image->height[ 2 ];
	levels[ 0 ] = image->levels[ 0 ];
	levels[ 1 ] = image->levels[ 1 ];
	levels[ 2 ] = image->levels[ 2 ];

	if ( image->decode == xieValDecodeUncompressedTriple )
	{
		pixel_stride[ 0 ] = image->pixel_stride[ 0 ];
		pixel_stride[ 1 ] = image->pixel_stride[ 1 ];
		pixel_stride[ 2 ] = image->pixel_stride[ 2 ];
		left_pad[ 0 ] = image->left_pad[ 0 ];
		left_pad[ 1 ] = image->left_pad[ 1 ];
		left_pad[ 2 ] = image->left_pad[ 2 ];
		scanline_pad[ 0 ] = image->scanline_pad[ 0 ];
		scanline_pad[ 1 ] = image->scanline_pad[ 1 ];
		scanline_pad[ 2 ] = image->scanline_pad[ 2 ];

		decode_params = ( char * ) XieTecDecodeUncompressedTriple(
			image->fill_order,
			xieValLSFirst,		
			xieValLSFirst,		
			image->interleave,	
			pixel_stride,
			left_pad,
			scanline_pad
		);
	}
	else if ( image->decode == xieValDecodeJPEGBaseline )
	{
		decode_params = ( char * ) XieTecDecodeJPEGBaseline(
			image->interleave,
			image->band_order
		);	
	}
	else if ( image->decode == xieValDecodeJPEGLossless )
	{
		decode_params = ( char * ) XieTecDecodeJPEGLossless(
			image->interleave,
			image->band_order
		);	
	}
	else
	{
		fprintf( stderr, "GetXIETriplePhotomap: invalid decode technique\n" );
		XieDestroyPhotomap( xp->d, tmp );
                return( XiePhotomap ) NULL;
	}	

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIETriplePhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, tmp );
                return( XiePhotomap ) NULL;
        }

        XieFloImportClientPhoto(&flograph[0],
                image->bandclass,
                width, height, levels,
                False,
                image->decode, (char *)decode_params
        );

        XieFloExportPhotomap(&flograph[1],
                1,              /* source phototag number */
                tmp,
                encode_tech,
                encode_params
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                2               /* number of elements */
        );
        XSync( xp->d, 0 );
	if ( image->interleave == xieValBandByPixel )
	{
		PumpTheClientData( xp, p, flo_id, photospace, 
			1, image->data, size, 0 );
	}
	else
	{
		SendTripleBandPlaneDataSequential( xp, p, flo_id, photospace,
			1, image->data, size, pixel_stride,
			left_pad, scanline_pad, width, height );
	} 
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,2);
        XieDestroyPhotospace(xp->d, photospace);
	if ( decode_params )
		free( decode_params );
	AddToCache( image, tmp );
    	return tmp;
}

XiePhotomap
GetXIEPhotomap( xp, p, which )
XParms	xp;
Parms	p;
int	which;
{
	XIEimage	*image;
        XiePhotospace photospace;
	int	flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieDecodeUncompressedSingleParam *decode_params=NULL;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
	XiePhotomap	tmp;
	int	size;

	if ( which == 1 )
	{
		image = p->finfo.image1; 
	}
	else if ( which == 2 )
	{
		image = p->finfo.image2;
	}
	else 
	{
		image = p->finfo.image3;
	}

	if ( !image )
		return( XiePhotomap ) NULL;

	if ( IsImageInCache( image ) == True )
	{
		TouchImage( image );
		return( PhotomapOfImage( image ) );
	}

	if ( !GetImageData( xp, p, which ) )
		return( ( XiePhotomap ) NULL );

	size = image->fsize;
	image->chksum = CheckSum( image->data, size );

	width[ 0 ] = image->width[ 0 ];
	height[ 0 ] = image->height[ 0 ];
	levels[ 0 ] = image->levels[ 0 ];

	/* create a photomap */

        tmp = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

        decode_params = XieTecDecodeUncompressedSingle(
                image->fill_order,
                image->pixel_order,
                image->pixel_stride[ 0 ],
                image->left_pad[ 0 ],
                image->scanline_pad[ 0 ]
        );

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, tmp );
                return( XiePhotomap ) NULL;
        }

        XieFloImportClientPhoto(&flograph[0],
                image->bandclass,
                width, height, levels,
                False,
                image->decode, (char *)decode_params
        );

        XieFloExportPhotomap(&flograph[1],
                1,              /* source phototag number */
                tmp,
                encode_tech,
                encode_params
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                2               /* number of elements */
        );
        XSync( xp->d, 0 );
       	PumpTheClientData( xp, p, flo_id, photospace, 1, image->data, size, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,2);
        XieDestroyPhotospace(xp->d, photospace);
	if ( decode_params )
		free( decode_params );
	AddToCache( image, tmp );
    	return tmp;
}

XiePhotomap
GetXIEPointPhotomap( xp, p, which, inlevels )
XParms	xp;
Parms	p;
int	which;
int	inlevels;
{
	XIEimage *image;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
        XiePhotospace photospace;
	int flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieLTriplet width, height, levels;
	XiePhotomap XIEPhotomap, tmp;
	XieLut XIELut;
	XieProcessDomain domain;
	XieLut CreatePointLut();

	tmp = GetXIEPhotomap( xp, p, which );

	if ( which == 1 )
		image = p->finfo.image1;
	else if ( which == 2 )
		image = p->finfo.image2;
	else if ( which == 3 )
		image = p->finfo.image3;

	if ( tmp == ( XiePhotomap ) NULL )
		return( tmp );

	XIELut = CreatePointLut( xp, p, image->depth[ 0 ], inlevels );
	if ( XIELut == ( XieLut ) NULL )
		return( ( XiePhotomap ) NULL );

        XIEPhotomap = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(4);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEPointPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
                return( XiePhotomap ) NULL;
        }

        XieFloImportPhotomap(&flograph[0],
		tmp,
                False
        );

	XieFloImportLUT(&flograph[1], XIELut );

        domain.offset_x = 0;
        domain.offset_y = 0;
        domain.phototag = 0;
	
	XieFloPoint(&flograph[2],
                1,
                &domain,
                2,
                0x1
        );
		
        XieFloExportPhotomap(&flograph[3],
                3,              /* source phototag number */
                XIEPhotomap,
                encode_tech,
                encode_params
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                4               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,4);
        XieDestroyPhotospace(xp->d, photospace);
	if ( XIELut )
		XieDestroyLUT( xp->d, XIELut );
    	return XIEPhotomap;
}

XiePhotomap
GetXIEGeometryPhotomap( xp, p, geo, which )
XParms	xp;
Parms	p;
GeometryParms *geo;
int	which;
{
	XIEimage *image;
        XiePhotospace photospace;
	int size, flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieLTriplet width, height, levels;
	static XieConstant constant = { 0.0, 0.0, 0.0 };
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	float coeffs[ 6 ];
	XiePhotomap tmp, XIEPhotomap;

	tmp = GetXIEPhotomap( xp, p, which );

	/* create a photomap */

        XIEPhotomap = XieCreatePhotomap(xp->d);

	if ( XIEPhotomap == ( XiePhotomap ) NULL )
		return( ( XiePhotomap ) NULL );

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEGeometryPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
                return( XiePhotomap ) NULL;
        }

        XieFloImportPhotomap(&flograph[0],
		tmp,
                False
        );

        SetCoefficients( xp, p, geo, coeffs );

        XieFloGeometry(&flograph[1],
                1,
                geo->geoWidth,
                geo->geoHeight,  
                coeffs,
                constant,
                7,
                geo->geoTech,
                ( char * ) NULL 
        ); 

        XieFloExportPhotomap(&flograph[2],
                2,              /* source phototag number */
                XIEPhotomap,
                encode_tech,
                encode_params
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
        XieDestroyPhotospace(xp->d, photospace);
    	return XIEPhotomap;
}

XiePhotomap
GetXIEGeometryWindow( xp, p, w, geo, which )
XParms	xp;
Parms	p;
Window 	w;
GeometryParms *geo;
int	which;
{
	XIEimage *image;
        XiePhotospace photospace;
	int size, flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieLTriplet width, height, levels;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	float coeffs[ 6 ];
	static XieConstant constant = { 0.0, 0.0, 0.0 };
	XieLut XIELut;
	XieProcessDomain domain;
	XiePhotomap tmp, XIEPhotomap;

	switch( which )
	{
	case 1:
		image = p->finfo.image1;
		break;
	case 2:
		image = p->finfo.image2;
		break;
	case 3:
		image = p->finfo.image3;
		break;
	default:
		fprintf( stderr, "Invalid image\n" );
		return( 0 );
	};

	tmp = GetXIEPhotomap( xp, p, which );

	/* create a photomap */

        XIEPhotomap = XieCreatePhotomap(xp->d);

	if ( XIEPhotomap == ( XiePhotomap ) NULL )
		return( ( XiePhotomap ) NULL );

	if ( ( XIELut = CreatePointLut( xp, p, image->depth[ 0 ], 
		xp->vinfo.depth ) ) == ( XieLut ) NULL )
	{
                return 0;
        }

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(5);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEGeometryPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
                return( XiePhotomap ) NULL;
        }

        XieFloImportPhotomap(&flograph[0],
		tmp,
                False
        );

        SetCoefficients( xp, p, geo, coeffs );

        XieFloGeometry(&flograph[1],
                1,
                geo->geoWidth,
                geo->geoHeight,  
                coeffs,
                constant,
                7,
                geo->geoTech,
                ( char * ) NULL 
        ); 

	XieFloImportLUT(&flograph[2], XIELut );

        domain.offset_x = 0;
        domain.offset_y = 0;
        domain.phototag = 0;

        XieFloPoint(&flograph[3],
                2,
                &domain,
                3,
                0x1
        );
		
        XieFloExportDrawable(&flograph[4],
                4,              /* source phototag number */
                w,
		xp->fggc,
		0,
		0
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                5               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,5);
        XieDestroyPhotospace(xp->d, photospace);
	XieDestroyLUT( xp->d, XIELut );
    	return XIEPhotomap;
}

int
GetXIEPixmap( xp, p, pixmap, which )
XParms	xp;
Parms	p;
Pixmap 	pixmap;
int	which;
{
	XIEimage *image;
        XiePhotospace photospace;
	int flo_elements, flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieLut	XIELut;
	XiePhotomap XIEPhotomap;
        XieProcessDomain domain;
	XieLut CreatePointLut();

	XIEPhotomap = GetXIEPhotomap( xp, p, which );
	if ( XIEPhotomap == ( XiePhotomap ) NULL )
		return( 0 );

	switch( which )
	{
	case 1:
		image = p->finfo.image1;
		break;
	case 2:
		image = p->finfo.image2;
		break;
	case 3:
		image = p->finfo.image3;
		break;
	default:
		image = ( XIEimage * ) NULL;
		break;
	}

	if ( image == ( XIEimage * ) NULL )
		return( 0 );

	if ( ( XIELut = CreatePointLut( xp, p, image->depth[ 0 ], xp->vinfo.depth
		 ) ) == ( XieLut ) NULL )
	{
		return 0;
	}

        photospace = XieCreatePhotospace(xp->d);

	flo_elements = 4;
        flograph = XieAllocatePhotofloGraph(flo_elements);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEPixmap: XieAllocatePhotofloGraph failed\n" );
		if ( XIELut )
			XieDestroyLUT( xp->d, XIELut );
                return( 0 );
        }

        XieFloImportPhotomap(&flograph[0],
		XIEPhotomap,
                False
        );

	XieFloImportLUT(&flograph[1], XIELut );

	domain.offset_x = 0;
        domain.offset_y = 0;
        domain.phototag = 0;

	XieFloPoint(&flograph[2],
		1,
		&domain,
		2,
		0x1
	);

        XieFloExportDrawable(&flograph[flo_elements - 1],
		flo_elements - 1,       /* source phototag number */
		pixmap,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	);

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                flo_elements    /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,flo_elements);
	XieDestroyPhotospace( xp->d, photospace );
	if ( XIELut )
		XieDestroyLUT( xp->d, XIELut );
    	return 1;
}

int
GetXIEWindow( xp, p, window, which )
XParms	xp;
Parms	p;
Window 	window;
int	which;
{
        XiePhotospace photospace;
	int flo_elements, flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieLut	XIELut;
	XIEimage *image;
	XiePhotomap XIEPhotomap;
        XieProcessDomain domain;
	XieLut CreatePointLut();

	switch( which )
	{
	case 1:
		image = p->finfo.image1;
		break;
	case 2:
		image = p->finfo.image2;
		break;
	case 3:
		image = p->finfo.image3;
		break;
	default:
		image = ( XIEimage * ) NULL;
		break;
	}

	XIEPhotomap = GetXIEPhotomap( xp, p, which );
	if ( XIEPhotomap == ( XiePhotomap ) NULL )
		return( 0 );
	
	if ( ( XIELut = CreatePointLut( xp, p, image->depth[ 0 ], 
		xp->vinfo.depth ) ) == ( XieLut ) NULL )
	{
                return 0;
        }

        photospace = XieCreatePhotospace(xp->d);

	flo_elements = 4;

        flograph = XieAllocatePhotofloGraph(flo_elements);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEWindow: XieAllocatePhotofloGraph failed\n" );
		if ( XIELut )
			XieDestroyLUT( xp->d, XIELut );
                return( 0 );
        }

        XieFloImportPhotomap(&flograph[0],
		XIEPhotomap,
                False
        );

	XieFloImportLUT(&flograph[1], XIELut );

	domain.offset_x = 0;
        domain.offset_y = 0;
        domain.phototag = 0;

	XieFloPoint(&flograph[2],
		1,
		&domain,
		2,
		0x1
	);

        XieFloExportDrawable(&flograph[3],
		3,     /* source phototag number */
		window,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	);

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                flo_elements    /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,flo_elements);
	XieDestroyPhotospace( xp->d, photospace );

	if ( XIELut )
		XieDestroyLUT( xp->d, XIELut );
    	return 1;
}

int
GetXIEDitheredWindow( xp, p, window, which, level )
XParms	xp;
Parms	p;
Window 	window;
int	which;
int	level;
{
        XiePhotospace photospace;
	int flo_id, flo_notify;
        XiePhotoElement *flograph;
        char *tech_parms=NULL;
	XieLTriplet levels;
	XiePhotomap tmp;

	tmp = GetXIEPhotomap( xp, p, which );
	if ( tmp == ( XiePhotomap ) NULL )
		return( 0 );

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEDitheredPhotomap: XieAllocatePhotofloGraph failed\n" );
                return ( 0 );
        }

        XieFloImportPhotomap(&flograph[0],
		tmp,
                False
        );

        levels[ 0 ] = level;		
        levels[ 1 ] = 0;
        levels[ 2 ] = 0;

	tech_parms = ( char * ) NULL;

	XieFloDither(&flograph[ 1 ],
		1,
		1,
	        levels,
		xieValDitherDefault,
		tech_parms
	);
 
        XieFloExportDrawable(&flograph[2],
                2,              /* source phototag number */
                xp->w,
                xp->fggc,
                0,
		0
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
    	return 1;
}

XiePhotomap
GetXIEDitheredPhotomap( xp, p, which, level )
XParms	xp;
Parms	p;
int	which;
int	level;
{
	XIEimage *image;
        XiePhotospace photospace;
	int flo_id, flo_notify;
        XiePhotoElement *flograph;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
        char *tech_parms=NULL;
	XiePhotomap tmp, XIEPhotomap;
	int size;

	tmp = GetXIEPhotomap( xp, p, which );
	if ( tmp == ( XiePhotomap ) NULL )
		return( ( XiePhotomap ) NULL );

	/* create a photomap */

        XIEPhotomap = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEDitheredPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
                return ( XiePhotomap ) NULL;
        }

        XieFloImportPhotomap(&flograph[0],
		tmp,
                False
        );

        levels[ 0 ] = level;		
        levels[ 1 ] = 0;
        levels[ 2 ] = 0;

	tech_parms = ( char * ) NULL;

	XieFloDither(&flograph[ 1 ],
		1,
		1,
	        levels,
		xieValDitherDefault,
		tech_parms
	);
 
        XieFloExportPhotomap(&flograph[2],
                2,              /* source phototag number */
                XIEPhotomap,
                encode_tech,
                encode_params
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
    	return XIEPhotomap;
}

XiePhotomap
GetXIEDitheredTriplePhotomap( xp, p, which, ditherTech, threshold, levels )
XParms	xp;
Parms	p;
int	which;
int	ditherTech;
int	threshold;
XieLTriplet levels;
{
	XiePhotomap tmp;
	XiePhotomap triple;
        XiePhotospace photospace;
	XiePhotoElement *flograph;
        char *dithertech_parms=NULL;
        static XieEncodeTechnique encode_tech = xieValEncodeServerChoice;
        char *encode_param = ( char * ) NULL;
        int flo_id, flo_notify;

	if ( ( triple = GetXIETriplePhotomap( xp, p, which ) ) ==
                ( XiePhotomap ) NULL )
	{
		return( ( XiePhotomap ) NULL );
	}

	/* create a photomap */

        tmp = XieCreatePhotomap(xp->d);
	if ( tmp == ( XiePhotomap ) NULL )
	{
        	XieDestroyPhotomap( xp->d, triple );
		return( ( XiePhotomap ) NULL );
	}

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEDitheredTriplePhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, tmp );
		XieDestroyPhotomap( xp->d, triple );
                return ( XiePhotomap ) NULL;
        }

	XieFloImportPhotomap(&flograph[0], triple, False);

        dithertech_parms = ( char * ) NULL;
        if ( ditherTech == xieValDitherOrdered )
        {
                dithertech_parms = ( char * )
                        XieTecDitherOrderedParam(threshold);
                if ( dithertech_parms == ( char * ) NULL )
                {
                        fprintf( stderr,
                        "Trouble loading dither technique parameters\n" );
                        XieFreePhotofloGraph(flograph,3);
                        XieDestroyPhotospace( xp->d, photospace );
			XieDestroyPhotomap( xp->d, tmp );
			XieDestroyPhotomap( xp->d, triple );
                        return( 0 );
                }
        }

        XieFloDither( &flograph[ 1 ],
                1,
		0x7,
                levels,
                ditherTech,
                dithertech_parms
        );

        XieFloExportPhotomap(&flograph[2],
                2,              /* source phototag number */
                tmp,
                encode_tech,
                encode_param
        );

        flo_id = 1;
        flo_notify = True;

        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
        XieDestroyPhotospace( xp->d, photospace );
        if ( dithertech_parms )
                free( dithertech_parms );
    	return tmp;
}

XiePhotomap
GetXIEConstrainedPhotomap( xp, p, which, cliplevels, cliptype,
	in_low, in_high, out_low, out_high )
XParms	xp;
Parms	p;
int	which;
XieLTriplet cliplevels;
int	cliptype;
XieConstant in_low,in_high;
XieLTriplet out_low,out_high;
{
	XIEimage *image;
        XiePhotospace photospace;
	int flo_id, flo_notify;
        XiePhotoElement *flograph;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
        char *tech_parms=NULL;
	XiePhotomap tmp, XIEPhotomap;

	tmp = GetXIEPhotomap( xp, p, which );
	if ( tmp == ( XiePhotomap ) NULL )
		return( ( XiePhotomap ) NULL );

	/* create a photomap */

        XIEPhotomap = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEConstrainedPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
                return ( XiePhotomap ) NULL;
        }

        XieFloImportPhotomap(&flograph[0],
		tmp,
                False
        );

    	if ( cliptype == xieValConstrainHardClip )
	{
		tech_parms = ( char * ) NULL;
	}
	else
	{
		tech_parms = ( char * ) XieTecClipScale( in_low, in_high, 
			out_low, out_high);
		if ( tech_parms == ( char * ) NULL )
		{
			fprintf( stderr,
				"GetXIEConstrainedPhotomap: Trouble loading ClipScale technique parameters\n" );
			fprintf( stderr, "Reverting to HardClip technique\n" );
			cliptype = xieValConstrainHardClip;
		}
	}

	XieFloConstrain( &flograph[1],
		1,
		cliplevels,
		cliptype,
		tech_parms
	);

        XieFloExportPhotomap(&flograph[2],
		2,
                XIEPhotomap,
		encode_tech,
		encode_params
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
	if ( tech_parms )
		free( tech_parms );
    	return XIEPhotomap;
}

XiePhotomap
GetXIEConstrainedTriplePhotomap( xp, p, which, cliplevels, cliptype, 
	in_low, in_high, out_low, out_high )
XParms	xp;
Parms	p;
int	which;
XieLTriplet cliplevels;
int 	cliptype;
XieConstant in_low,in_high;
XieLTriplet out_low,out_high;
{
	XiePhotomap tmp, XIEPhotomap;
        XiePhotospace photospace;
	XiePhotoElement *flograph;
	char *tech_parms = ( char * ) NULL;
        static XieEncodeTechnique encode_tech = xieValEncodeServerChoice;
        char *encode_params = ( char * ) NULL;
        int flo_id, flo_notify;

	if ( ( tmp = GetXIETriplePhotomap( xp, p, which ) ) ==
                ( XiePhotomap ) NULL )
	{
		return( ( XiePhotomap ) NULL );
	}

	/* create a photomap */

        XIEPhotomap = XieCreatePhotomap(xp->d);
	if ( XIEPhotomap == ( XiePhotomap ) NULL )
	{
		return( ( XiePhotomap ) NULL );
	}

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEConstrainedTriplePhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
                return ( XiePhotomap ) NULL;
        }

	XieFloImportPhotomap(&flograph[0], tmp, False);

        if ( cliptype == xieValConstrainHardClip )
        {
                tech_parms = ( char * ) NULL;
        }
        else
        {
                tech_parms = ( char * ) XieTecClipScale( in_low, in_high,
                        out_low, out_high);
                if ( tech_parms == ( char * ) NULL )
                {
                        fprintf( stderr,
                                "GetXIEConstrainedPhotomap: Trouble loading ClipScale technique parameters\n" );
                        fprintf( stderr, "Reverting to HardClip technique\n" );
                        cliptype = xieValConstrainHardClip;
                }
        }

        XieFloConstrain( &flograph[1],
                1,
                cliplevels,
                cliptype,
                tech_parms
	);

        XieFloExportPhotomap(&flograph[2],
                2,              /* source phototag number */
                XIEPhotomap,
		encode_tech,
		encode_params
        );

        flo_id = 1;
        flo_notify = True;

        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
        XieDestroyPhotospace( xp->d, photospace );
        if ( tech_parms )
                free( tech_parms );
    	return XIEPhotomap;
}

XiePhotomap
GetXIEConstrainedGeometryTriplePhotomap( xp, p, which, cliplevels, cliptype, 
	in_low, in_high, out_low, out_high, geo )
XParms	xp;
Parms	p;
int	which;
XieLTriplet cliplevels;
int 	cliptype;
XieConstant in_low,in_high;
XieLTriplet out_low,out_high;
GeometryParms *geo;
{
	XiePhotomap tmp, XIEPhotomap;
        XiePhotospace photospace;
	XiePhotoElement *flograph;
	char *tech_parms = ( char * ) NULL;
        static XieEncodeTechnique encode_tech = xieValEncodeServerChoice;
        char *encode_params = ( char * ) NULL;
        int flo_id, flo_notify;
	float coeffs[ 6 ];
	static XieConstant constant = { 0.0, 0.0, 0.0 };

	if ( ( tmp = GetXIETriplePhotomap( xp, p, which ) ) ==
                ( XiePhotomap ) NULL )
	{
		return( ( XiePhotomap ) NULL );
	}

	/* create a photomap */

        XIEPhotomap = XieCreatePhotomap(xp->d);
	if ( XIEPhotomap == ( XiePhotomap ) NULL )
	{
		return( ( XiePhotomap ) NULL );
	}

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(4);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEConstrainedTriplePhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
                return ( XiePhotomap ) NULL;
        }

	XieFloImportPhotomap(&flograph[0], tmp, False);

        SetCoefficients( xp, p, geo, coeffs );

        XieFloGeometry(&flograph[1],
                1,
                geo->geoWidth,
                geo->geoHeight,
                coeffs,
                constant,
                7,
                geo->geoTech,
                ( char * ) NULL
        );

        if ( cliptype == xieValConstrainHardClip )
        {
                tech_parms = ( char * ) NULL;
        }
        else
        {
                tech_parms = ( char * ) XieTecClipScale( in_low, in_high,
                        out_low, out_high);
                if ( tech_parms == ( char * ) NULL )
                {
                        fprintf( stderr,
                                "GetXIEConstrainedPhotomap: Trouble loading ClipScale technique parameters\n" );
                        fprintf( stderr, "Reverting to HardClip technique\n" );
                        cliptype = xieValConstrainHardClip;
                }
        }

        XieFloConstrain( &flograph[2],
                2,
                cliplevels,
                cliptype,
                tech_parms
	);

        XieFloExportPhotomap(&flograph[3],
                3,              /* source phototag number */
                XIEPhotomap,
		encode_tech,
		encode_params
        );

        flo_id = 1;
        flo_notify = True;

        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                4               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,4);
        XieDestroyPhotospace( xp->d, photospace );
        if ( tech_parms )
                free( tech_parms );
    	return XIEPhotomap;
}

XiePhotomap
GetXIEGeometryTriplePhotomap( xp, p, which, geo )
XParms	xp;
Parms	p;
int	which;
GeometryParms *geo;
{
	XiePhotomap tmp, XIEPhotomap;
        XiePhotospace photospace;
	XiePhotoElement *flograph;
	char *tech_parms = ( char * ) NULL;
        static XieEncodeTechnique encode_tech = xieValEncodeServerChoice;
        char *encode_params = ( char * ) NULL;
        int flo_id, flo_notify;
	float coeffs[ 6 ];
	static XieConstant constant = { 0.0, 0.0, 0.0 };

	if ( ( tmp = GetXIETriplePhotomap( xp, p, which ) ) ==
                ( XiePhotomap ) NULL )
	{
		return( ( XiePhotomap ) NULL );
	}

	/* create a photomap */

        XIEPhotomap = XieCreatePhotomap(xp->d);
	if ( XIEPhotomap == ( XiePhotomap ) NULL )
	{
		return( ( XiePhotomap ) NULL );
	}

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEGeometryTriplePhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
                return ( XiePhotomap ) NULL;
        }

	XieFloImportPhotomap(&flograph[0], tmp, False);

        SetCoefficients( xp, p, geo, coeffs );

        XieFloGeometry(&flograph[1],
                1,
                geo->geoWidth,
                geo->geoHeight,
                coeffs,
                constant,
                7,
                geo->geoTech,
                ( char * ) NULL
        );

        XieFloExportPhotomap(&flograph[2],
                2,              /* source phototag number */
                XIEPhotomap,
		encode_tech,
		encode_params
        );

        flo_id = 1;
        flo_notify = True;

        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
        XieDestroyPhotospace( xp->d, photospace );
        if ( tech_parms )
                free( tech_parms );
    	return XIEPhotomap;
}

int
GetXIEDitheredTripleWindow( xp, p, w, which, ditherTech, threshold, levels )
XParms	xp;
Parms	p;
Window 	w;
int	which;
int	ditherTech;
int	threshold;
XieLTriplet levels;
{
	XiePhotomap ditheredPhoto;
        XiePhotospace photospace;
	XiePhotoElement *flograph;
        int flo_id, flo_notify;
	XieColorList clist;
        XWindowAttributes xwa;
        XieColorAllocAllParam *color_parm;

        color_parm = ( XieColorAllocAllParam * ) NULL;
	clist = ( XieColorList ) NULL;
	color_parm = XieTecColorAllocAll( 123 );
	if ( color_parm == ( XieColorAllocAllParam * ) NULL )
	{
		return( 0 );
	}

	if ( ( clist = XieCreateColorList( xp->d ) ) == ( XieColorList ) NULL )
	{
		if ( color_parm )
			free( color_parm );
		return( 0 );
	}
	if ( ( ditheredPhoto = GetXIEDitheredTriplePhotomap( xp, p, which,
		ditherTech, threshold, levels ) ) == ( XiePhotomap ) NULL )
	{
		if ( clist )
			XieDestroyColorList( xp->d, clist );
		if ( color_parm )
			free( color_parm );
		return( 0 );
	}

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEDitheredTripleWindow: XieAllocatePhotofloGraph failed\n" );
		if ( color_parm )
			free( color_parm );
		if ( clist )
			XieDestroyColorList( xp->d, clist );
		return( 0 );
        }

	XieFloImportPhotomap(&flograph[0], ditheredPhoto, False);

        XGetWindowAttributes( xp->d, xp->w, &xwa );
        XieFloConvertToIndex(&flograph[1],
                1,
                xwa.colormap,
                clist,
                False,
                xieValColorAllocAll,
                (char *)color_parm
        );

        XieFloExportDrawable(&flograph[2],
                2,              /* source phototag number */
                xp->w,
		xp->fggc,
                0,
               	0 
        );

        flo_id = 1;
        flo_notify = True;

        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
        XieDestroyPhotospace( xp->d, photospace );
	if ( clist )
		XieDestroyColorList( xp->d, clist );
	if ( color_parm )
		free( color_parm );
	return( 1 );
}

int
GetXIEDitheredStdTripleWindow( xp, p, w, which, ditherTech, threshold, levels,
	stdCmap )
XParms	xp;
Parms	p;
Window 	w;
int	which;
int	ditherTech;
int	threshold;
XieLTriplet levels;
XStandardColormap *stdCmap;
{
	XiePhotomap ditheredPhoto;
        XiePhotospace photospace;
	XiePhotoElement *flograph;
        int flo_id, flo_notify;
	XieColorList clist;
        XWindowAttributes xwa;
        XieColorAllocAllParam *color_parm;
	XieConstant c1;
	float bias;

	if ( ( ditheredPhoto = GetXIEDitheredTriplePhotomap( xp, p, which,
		ditherTech, threshold, levels ) ) == ( XiePhotomap ) NULL )
	{
		if ( clist )
			XieDestroyColorList( xp->d, clist );
		if ( color_parm )
			free( color_parm );
		return( 0 );
	}

        photospace = XieCreatePhotospace(xp->d);

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEDitheredTripleWindow: XieAllocatePhotofloGraph failed\n" );
		return( 0 );
        }

	XieFloImportPhotomap(&flograph[0], ditheredPhoto, False);

       	c1[ 0 ] = stdCmap->red_mult;
	c1[ 1 ] = stdCmap->green_mult;
	c1[ 2 ] = stdCmap->blue_mult;
	bias = ( float ) stdCmap->base_pixel;

	XieFloBandExtract( &flograph[1], 1, 1 << xp->vinfo.depth, bias, c1 );

        XieFloExportDrawable(&flograph[2],
                2,              /* source phototag number */
                xp->w,
		xp->fggc,
                0,
               	0 
        );

        flo_id = 1;
        flo_notify = True;

        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,3);
        XieDestroyPhotospace( xp->d, photospace );
	return( 1 );
}

int
GetFileSize( path )
char	*path;
{
	int	size;

        /* open the file */

        if ( ( size = file_size( path ) ) < 0 )
        {
                fprintf( stderr, "Couldn't stat %s\n", path );
                return 0;
        }
	return( size );
}
 
int
GetImageData( xp, p, which )
XParms	xp;
Parms	p;
int	which;
{
        int     fd;
        int     n;
	int	*size;
	XIEimage *image;
	char	*name;
	char	buf[ 64 ];

	if ( which == 1 )
	{
		image = p->finfo.image1;
	}
	else if ( which == 2 )
	{
		image = p->finfo.image2;
	}	
	else
	{
		image = p->finfo.image3;
	}

	if (!image)
		return( 0 );

        if ( p->buffer_size <= 0 )
        {
                fprintf( stderr, "buffer_size is invalid\n" );
                return 0;
        }

	size = &image->fsize;
	name = image->fname;
	sprintf( buf, "%s/%s", imagepath, name );

	*size = GetFileSize( buf );

	if ( *size == 0 )
		return( 0 );

        if ( ( fd = open( buf, O_RDONLY ) ) == -1 )
        {
                fprintf( stderr, "Couldn't open %s\n", buf );
                goto out;
        }

        /* allocate the data buffer */

	if ( image->data == ( char * ) NULL )
	{
		if ( ( image->data = 
			(char *)malloc( *size ) ) == ( char * ) NULL )
		{
			fprintf( stderr, "Couldn't allocate buffer\n" );
			goto out;
		}
	}

        /* read the data */

        if ( ( n = read( fd, image->data, *size ) ) != *size )
        {
                fprintf( stderr, "Couldn't read data\n" );
                goto out;
        }

        /* close the file */

        close( fd );
	return( 1 );

out:	if ( fd )
	{
		close( fd );
	}
	return 0;
}

XieLut
GetXIELut( xp, p, lut, lutSize, lutLevels )
XParms	xp;
Parms	p;
unsigned char *lut;
int	lutSize;
int	lutLevels;
{
        XiePhotospace photospace;
	int	flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieDataClass	cclass;
	XieOrientation	band_order;
	XieLTriplet	length, levels;
	Bool 	merge;
	XieLTriplet	start;
	XieLut	tmp;

	/* create a LUT for starters */

        tmp = XieCreateLUT(xp->d);

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf(stderr,"GetXIELut: XieAllocatePhotofloGraph failed\n");
		XieDestroyLUT( xp->d, tmp );
                return ( XieLut ) NULL;
        }

	cclass = xieValSingleBand;
	band_order = xieValMSFirst;
	length[ 0 ] = lutSize;
	length[ 1 ] = 0;
	length[ 2 ] = 0;
	levels[ 0 ] = lutLevels;
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;

        XieFloImportClientLUT(&flograph[0],
                cclass,
		band_order,
		length,
		levels
        );

	merge = False;
	start[ 0 ] = 0; 
	start[ 1 ] = 0; 
	start[ 2 ] = 0; 

        XieFloExportLUT(&flograph[1],
                1,              /* source phototag number */
                tmp,
                merge,
                start
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                2               /* number of elements */
        );
        XSync( xp->d, 0 );
        PumpTheClientData( xp, p, flo_id, photospace, 1, lut, lutSize, 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
        XieFreePhotofloGraph(flograph,2);
    	return tmp;
}

XieRoi
GetXIERoi( xp, p, rects, rectsSize )
XParms	xp;
Parms	p;
XieRectangle *rects;
int	rectsSize;
{
        XiePhotospace photospace;
	int	flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieRoi	tmp;

	/* create an ROI */

        tmp = XieCreateROI(xp->d);

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf(stderr,"GetXIERoi: XieAllocatePhotofloGraph failed\n");
		XieDestroyROI( xp->d, tmp );
                return ( XieRoi ) NULL;
        }

        XieFloImportClientROI(&flograph[0],
		rectsSize
        );

        XieFloExportROI(&flograph[1],
                1,              /* source phototag number */
               	tmp 
        );

        flo_id = 1;
        flo_notify = True;
        XieExecuteImmediate(xp->d, photospace,
                flo_id,
                flo_notify,
                flograph,       /* photoflo specification */
                2               /* number of elements */
        );
        XSync( xp->d, 0 );
        PumpTheClientData( xp, p, flo_id, photospace, 1, rects, rectsSize
		* sizeof( XieRectangle ), 0 );
        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo_id, 0, False );
 
        XieFreePhotofloGraph(flograph,2);
    	return tmp;
}

int
EventOrErrorValid( testcp )
unsigned short testcp;
{
	if ( IsEvent( testcp ) && runEvents == False )
		return( 0 );
	else if ( IsError( testcp ) && runErrors == False )
		return( 0 );
	return( 1 );
}

int	
IsDISServer()
{
	return( IsDIS( capabilities ) );
}

int
ServerIsCapable( testcp )
unsigned short testcp;
{
	if ( IsFull( testcp ) && IsDIS( capabilities ) )
		return( 0 );
	return( 1 );
}

int
SetupClipScale( xp, p, image, levels, in_low, in_high, out_low, out_high, parms )
XParms	xp;
Parms   p;
XIEimage *image;
XieLTriplet levels;
XieConstant in_low, in_high;
XieLTriplet out_low, out_high;
XieClipScaleParam **parms;
{
        levels[ 0 ] = ( 1 << xp->vinfo.depth );
        levels[ 1 ] = ( 1 << xp->vinfo.depth );
        levels[ 2 ] = ( 1 << xp->vinfo.depth );

        in_low[ 0 ] = 0.0;
        in_low[ 1 ] = 0.0;
        in_low[ 2 ] = 0.0;
        in_high[ 0 ] = ( float ) ( 1 << image->depth[ 0 ] ) - 1.0;
        in_high[ 1 ] = ( float ) ( 1 << image->depth[ 1 ] ) - 1.0;
        in_high[ 2 ] = ( float ) ( 1 << image->depth[ 2 ] ) - 1.0;
        out_low[ 0 ] = 0;
        out_low[ 1 ] = 0;
        out_low[ 2 ] = 0;
        out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;
        out_high[ 1 ] = ( 1 << xp->vinfo.depth ) - 1; 
        out_high[ 2 ] = ( 1 << xp->vinfo.depth ) - 1; 

	if ( TechniqueSupported( xp, xieValConstrain, xieValConstrainClipScale ) 
		== False )
	{
                fprintf( stderr, "ClipScale constrain tech not supported\n" );
		return( 0 );
	}
	
        *parms = XieTecClipScale( in_low, in_high, out_low, out_high );
        if ( *parms == ( XieClipScaleParam * ) NULL )
        {
                fprintf( stderr, "Couldn't allocate clip scale parameters\n" );
                return( 0 );
        }
        return( 1 );
}

Bool
TechniqueSupported( xp, group, tech )
XParms			xp;
XieTechniqueGroup	group;
unsigned int		tech;
{
	XieTechnique	*techVector;		
	int		numTech, i;
	Bool		gotIt;

	gotIt = False;
	if ( !XieQueryTechniques( xp->d, group, &numTech, &techVector ) )
	{
		fprintf( stderr, 
			"TechniqueSupported: XieQueryTechniques failed\n" );
	}
	else
	{	
		/* search for it */

		for ( i = 0; i < numTech; i++ )
		{
			if ( tech == techVector[ i ].number )
			{
				gotIt = True;
			}
			free( techVector[ i ].name ); 	
		}
		free( techVector );
	}
	
	return( gotIt );
}	

/* list all of the techniques supported. this isn't used by xieperf, but was
   used to validate TechniqueSupported() */

struct _tech {
	char	*name;
	unsigned int tech; 
};

struct _class {
	char *name;
	Bool full;
	XieTechniqueGroup group;
	struct _tech *techs; 
	int	n;
};

static struct _tech ColorAllocTechs[] = {
	{ "ColorAllocAll", xieValColorAllocAll },
	{ "ColorAllocMatch", xieValColorAllocMatch },
	{ "ColorAllocRequantize", xieValColorAllocRequantize } };

static struct _tech ConstrainTechs[] = {
	{ "ConstrainClipScale", xieValConstrainClipScale },
	{ "ConstrainHardClip", xieValConstrainHardClip } };
 
static struct _tech ConvolveTechs[] = {
	{ "ConvolveConstant", xieValConvolveConstant },
	{ "ConvolveReplicate", xieValConvolveReplicate } };
 
static struct _tech DecodeTechs[] = {
	{ "DecodeUncompressedSingle", xieValDecodeUncompressedSingle },
	{ "DecodeUncompressedTriple", xieValDecodeUncompressedTriple },
	{ "DecodeG31D", xieValDecodeG31D },
	{ "DecodeG32D", xieValDecodeG32D },
	{ "DecodeG42D", xieValDecodeG42D },
	{ "DecodeJPEGBaseline", xieValDecodeJPEGBaseline },
	{ "DecodeJPEGLossless", xieValDecodeJPEGLossless },
	{ "DecodeTIFF2", xieValDecodeTIFF2 },
	{ "DecodeTIFFPackBits", xieValDecodeTIFFPackBits } };

static struct _tech DitherTechs[] = {
	{ "DitherErrorDiffusion", xieValDitherErrorDiffusion },
	{ "DitherOrdered", xieValDitherOrdered } };

static struct _tech ColorspaceTechs[] = {
	{ "CIELab", xieValCIELab },
	{ "CIEXYZ", xieValCIEXYZ },
	{ "YCbCr", xieValYCbCr },
	{ "YCC", xieValYCC } };

static struct _tech EncodeTechs[] = {
	{ "EncodeUncompressedSingle", xieValEncodeUncompressedSingle },
	{ "EncodeUncompressedTriple", xieValEncodeUncompressedTriple },
	{ "EncodeG31D", xieValEncodeG31D },
	{ "EncodeG32D", xieValEncodeG32D },
	{ "EncodeG42D", xieValEncodeG42D },
	{ "EncodeJPEGBaseline", xieValEncodeJPEGBaseline },
	{ "EncodeJPEGLossless", xieValEncodeJPEGLossless },
	{ "EncodeTIFF2", xieValEncodeTIFF2 },
	{ "EncodeTIFFPackBits", xieValEncodeTIFFPackBits } };

static struct _tech GeometryTechs[] = {
	{ "GeometryAntialias", xieValGeomAntialias },
	{ "GeometryAntialiasByArea", xieValGeomAntialiasByArea },
	{ "GeometryAntialiasByLowpass", xieValGeomAntialiasByLPF },
	{ "GeometryBilinearInterpolation", xieValGeomBilinearInterp },
	{ "GeometryGaussian", xieValGeomGaussian },
	{ "GeometryNearestNeighbor", xieValGeomNearestNeighbor } };

static struct _tech GamutTechs[] = {
	{ "GamutNone", xieValGamutNone },
	{ "GamutClipRGB", xieValGamutClipRGB } };

static struct _tech HistogramTechs[] = {
	{ "HistogramFlat", xieValHistogramFlat },
	{ "HistogramGaussian", xieValHistogramGaussian },
	{ "HistogramHyperbolic", xieValHistogramHyperbolic } };

static struct _tech WhiteAdjustTechs[] = {
	{ "WhiteAdjustNone", xieValWhiteAdjustNone },
	{ "WhiteAdjustCIELabShift", xieValWhiteAdjustCIELabShift } };

static struct _class classes[] = { 
	{ "ColorAlloc", True, xieValColorAlloc, ColorAllocTechs, 
		sizeof( ColorAllocTechs ) / sizeof( struct _tech )  },
	{ "Constrain", True, xieValConstrain,  ConstrainTechs,
		sizeof( ConstrainTechs ) / sizeof( struct _tech )  },
	{ "Convolve", True, xieValConvolve, ConvolveTechs, 
		sizeof( ConvolveTechs ) / sizeof( struct _tech )  },
	{ "ConvertFromRGB", True, xieValConvertFromRGB, ColorspaceTechs, 
		sizeof( ColorspaceTechs ) / sizeof( struct _tech )  },
	{ "ConvertToRGB", True, xieValConvertToRGB, ColorspaceTechs, 
		sizeof( ColorspaceTechs ) / sizeof( struct _tech )  },
	{ "Decode", False, xieValDecode, DecodeTechs, 
		sizeof( DecodeTechs ) / sizeof( struct _tech )  },
	{ "Dither", True, xieValDither, DitherTechs, 
		sizeof( DitherTechs ) / sizeof( struct _tech )  },
	{ "Encode", False, xieValEncode, EncodeTechs, 
		sizeof( EncodeTechs ) / sizeof( struct _tech )  },
  	{ "Gamut", True, xieValGamut, GamutTechs, 
		sizeof( GamutTechs ) / sizeof( struct _tech )  },
	{ "Geometry", False, xieValGeometry, GeometryTechs, 
		sizeof( GeometryTechs ) / sizeof( struct _tech )  },
	{ "Histogram", True, xieValHistogram, HistogramTechs, 
		sizeof( HistogramTechs ) / sizeof( struct _tech )  },
  	{ "WhiteAdjust", True, xieValWhiteAdjust, WhiteAdjustTechs, 
		sizeof( WhiteAdjustTechs ) / sizeof( struct _tech )  } };
		  
int
ListAllTechs( xp )
XParms	xp;
{
	int	i, j;

	for ( i = 0; i < sizeof( classes ) / sizeof( struct _class ); i++ )
	{
		printf( "Class name: %s\n", classes[ i ].name );
		for ( j = 0; j < classes[ i ].n; j++ )
		{
			printf( "\tTechnique %s:", classes[ i ].techs[ j ].name );
			if (classes[ i ].full==True && IsDIS( capabilities ))
			{
				printf( " is not supported\n" ); 
			}
			else if ( TechniqueSupported( xp, classes[ i ].group,
				classes[ i ].techs[ j ].tech ) == True )
			{
				printf( " is supported\n" );
			}
			else
			{
				printf( " is not supported\n" );
			}
		}
	}
	printf( "\n" );
} 

void
FillHisto( histos, size, levels )
XieHistogramData histos[];
int	size;
int	levels;
{
	int	i;
	int	sy;

	sy = levels / size;
	for ( i = 0; i < size; i++ )
	{
		histos[ i ].value = i + 40;
		histos[ i ].count = i * sy;
	}	
}

void
DrawHistogram( xp, w, histos, size, levels )
XParms  xp;
Window	w;
XieHistogramData histos[];
int     size;
unsigned long levels;
{
        unsigned long maxcount;
        int     i;
        float   sx, sy;
        XRectangle *rects;
        short   yadd, xadd;
        char    buf[ 32 ];

        maxcount = 0;
	levels = 0;
        for ( i = 0; i < size; i++ )
        {
                if ( histos[ i ].count > maxcount )
                        maxcount = histos[ i ].count;
                if ( histos[ i ].value > levels  )
                        levels = histos[ i ].value;
        }

        if ( maxcount == 0 )
                return;

        xadd = ( short ) ( ( float ) MONWIDTH * 0.15 );
        yadd = xadd; 

        sx = ( float ) ( MONWIDTH - xadd ) / ( float ) ( levels + 1 ); 
        sy = ( float ) ( MONHEIGHT - yadd ) / ( float ) maxcount;

        XClearWindow( xp->d, w );

        /* label x */

        XDrawImageString( xp->d, w, tgc, xadd,
                MONHEIGHT - 3, "0", 1 );
        sprintf( buf, "%d", levels );
        XDrawImageString( xp->d, w, tgc, MONWIDTH -
                strlen( buf ) * 14, MONHEIGHT - 3, buf, strlen( buf ) );

        /* label y */

        sprintf( buf, "%d", maxcount );
        XDrawImageString( xp->d, w, tgc, 3,
                20, buf, strlen( buf ) );
        XDrawImageString( xp->d, w, tgc, 3,
                MONHEIGHT - yadd, "0", 1 );

	/* y axis */

        XDrawLine( xp->d, w, tgc, xadd, MONHEIGHT - yadd, 
		xadd, 0 );

	/* x axis */

        XDrawLine( xp->d, w, tgc, xadd, MONHEIGHT - yadd,
                MONWIDTH, MONHEIGHT - yadd );

        rects = ( XRectangle * ) malloc( sizeof( XRectangle ) * size );
        if ( rects == ( XRectangle * ) NULL )
                return;

        /* create the rectangles */

        for ( i = 0; i < size; i++ )
        {
                rects[ i ].width = ( short ) ceil( sx );
                rects[ i ].height = ( short ) ( sy * histos[ i ].count );
                rects[ i ].x = xadd + ( short ) ( sx * histos[ i ].value );
                rects[ i ].y = ( short ) MONHEIGHT - yadd - rects[ i ].height;
        }

        /* draw it */

        XFillRectangles( xp->d, w, tgc, rects, size );
        XSync( xp->d, 0 );

        free( rects );
}

int
GetStandardColormap( xp, stdColormap, atom )
XParms	xp;
XStandardColormap *stdColormap;
Atom	atom;
{
	int	status, i;
	int	numberColormaps;
	XStandardColormap *colormapsReturned;
	VisualID	visualId;

	visualId = XVisualIDFromVisual( xp->vinfo.visual );
	status = XmuLookupStandardColormap( xp->d, 
		DefaultScreen( xp->d ),
		visualId,	
		xp->vinfo.depth,
		atom,
		False,		/* Don't replace existing cmap */
		True );
	if ( status != 0 )
	{
		status = XGetRGBColormaps( xp->d,
			RootWindow( xp->d, DefaultScreen( xp->d ) ),
			&colormapsReturned,
			&numberColormaps,
			atom );
		if ( status != 0 )
		{
			for ( i = 0; i < numberColormaps; i++ )
			{
				if (visualId == colormapsReturned[i].visualid)
				{
					*stdColormap = colormapsReturned[ i ];
					free( colormapsReturned );
					return( True );
				}
			}
			free( colormapsReturned );
		}
	}
	return( False );
}

#define SETLUT if ( lutCellSize == sizeof( char ) ) {\
        *( ( char * ) ptr ) = val; \
	ptr+=sizeof(char); } \
else if ( lutCellSize == sizeof( short ) ) { \
        *( ( short * ) ptr ) = val; \
	ptr+=sizeof(short); } \
else { \
        *( ( long * ) ptr) = val; \
	ptr+=sizeof(long); }

XieLut
CreatePointLut( xp, p, indepth, outdepth )
XParms	xp;
Parms	p;
int	indepth;
int	outdepth;
{
        char    *lut, *ptr; 
        int     lutSize, lutLevels;
        int     lutCellSize;
        int     i, j, val;
        int     step, increment;
	XieLut 	retval;

	lutSize = 1 << indepth;
	lutCellSize = ( outdepth + 7 ) >> 3;  /* bytes, shorts, ? */

	if ( lutCellSize != sizeof( char ) && lutCellSize != sizeof( short )
		 && lutCellSize != sizeof( long ) )
	{
		fprintf( stderr, "CreatePointLut: cell size not supported\n" );
		return( ( XieLut ) NULL );
	}

        if ( indepth == outdepth )
        {
                increment = 1;
                step = 1;
        }
        else if ( indepth > outdepth )
        {
                increment = 1; 
                step = ( 1 << indepth ) / ( 1 << outdepth );
        }
        else /* outdepth > indepth */
        {
		increment = (( 1 << outdepth ) - 1 ) / (( 1 << indepth ) - 1);
                step = 1;
        }

	lut = (char *) malloc( lutSize * lutCellSize );
	ptr = ( void * ) lut;
	if ( lut == ( char * ) NULL )
		return( ( XieLut ) NULL );

	/* initialize the lut */

	val = 0;
	j = 0;
        for ( i = 0; i < lutSize; i++ )
        {
                SETLUT
                j++;
                if ( j == step )
                {
                        j = 0;
                        val += increment;
                }
        }
	retval = GetXIELut( xp, p, lut, lutSize, 1 << outdepth );
	free( lut );
	return( retval );
}

