/* $XConsortium: xieperf.c,v 1.11 93/09/14 14:28:40 rws Exp $ */

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

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif


#ifndef VMS
#include <X11/Xatom.h>
#else
#include <decw$include/Xatom.h>
#endif
#include "xieperf.h"
#ifndef O_BINARY
#define O_BINARY 0
#endif

/* Only for working on ``fake'' servers, for hardware that doesn't exist */
static Bool     drawToFakeServer = False;
static Pixmap   tileToQuery     = None;

static Bool     labels		= False;
static int      repeat		= 5;
static int	seconds		= 5;
Bool 	dontClear;
unsigned short capabilities = 0;
unsigned short class_request = SUBSET_FULL;

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
static int maxcoloridx;
static int timeout = 60;	/* in seconds */

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

int gettimeofday(tp)
    struct timeval *tp;
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
#if defined(SVR4) || defined(WIN32) || defined(VMS)
    gettimeofday(&start);
#else
    struct timezone foo;
    gettimeofday(&start, &foo);
#endif
}

double ElapsedTime(correction)
    double correction;
{
    struct timeval stop;
#if defined(SVR4) || defined(WIN32) || defined(VMS)
    gettimeofday(&stop);
#else
    struct timezone foo;
    gettimeofday(&stop, &foo);
#endif
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
"    -display <host:display>   the X server to contact",
"    -images <path>            path to images supplied with this software,", 
"                              defaults to ./images if flag not specified",
"    -timeout <s>              timeout value for certain tests (default=60)",
"    -sync                     do the tests in synchronous mode",
"    -repeat <n>               do tests <n> times (default = 5)",
"    -time <s>                 do tests for <s> seconds each (default = 5)",
"    -all                      do all tests",
"    -labels",
"    -DIS		       run DIS subset tests only",			
"    -range <test1>[,<test2>]  like all, but do <test1> to <test2>",
"    -reps <n>                 fix the rep count (default = auto scale)",
NULL};

    fflush(stdout);
    fprintf(stderr, "usage: %s [-options ...]\n", program_name);
    for (cpp = help_message; *cpp; cpp++) {
	fprintf(stderr, "%s\n", *cpp);
    }
    while (test[i].option != NULL) {
        fprintf(stderr, "    %-24s   %s\n",
		test[i].option, test[i].label);
        i++;
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

    xswa.background_pixel = xp->background;
    xswa.border_pixel = xp->foreground;
    xswa.override_redirect = True;
    xswa.backing_store = False;
    xswa.save_under = False;
    w = XCreateWindow(xp->d, DefaultRootWindow(xp->d), x, y, width, height, 1,
        xp->vinfo.depth, CopyFromParent, xp->vinfo.visual,
	CWBackPixel | CWBorderPixel /* | CWColormap */ | CWOverrideRedirect 
	| CWBackingStore | CWSaveUnder, &xswa);
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

    xswa.background_pixel = xp->background;
    xswa.border_pixel = xp->foreground;
    xswa.colormap = cmap;
    xswa.override_redirect = True;
    xswa.backing_store = False;
    xswa.save_under = False;
    w = XCreateWindow(xp->d, DefaultRootWindow(xp->d), x, y, width, height, 1,
        xp->vinfo.depth, CopyFromParent, xp->vinfo.visual,
	CWBackPixel | CWBorderPixel | CWColormap | CWOverrideRedirect 
	| CWBackingStore | CWSaveUnder, &xswa);
    return w;
}

void
InstallXIECmap( display )
Display	*display;
{
	XInstallColormap( display, cmap );	
}

void
UninstallXIECmap( display )
Display	*display;
{
	XUninstallColormap( display, cmap );
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
	clipWindows[j] = CreatePerfWindow(xp,
	    xwa.x + ws[j].x, xwa.y + ws[j].y, ws[j].width, ws[j].height);
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
    XParms(xp);
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


void ProcessTest(xp, test, func, pm, label)
    XParms  xp;
    Test    *test;
    int     func;
    unsigned long   pm;
    char    *label;
{
    double  time, totalTime;
    int     reps, fooreps;
    int     j;

    CreatePerfGCs(xp, func, pm);
    DisplayStatus(xp->d, "Calibrating", label);
    reps = CalibrateTest(xp, test, seconds, &time);
    if (reps != 0) {
	DisplayStatus(xp->d, "Testing", label);
	XDestroySubwindows(xp->d, xp->w);
	XClearWindow(xp->d, xp->w);
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
	} else if (strcmp(argv[i], "-DIS") == 0) {
		class_request = SUBSET_DIS;
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
	   assemble data from different x11perf runs into a nice format */
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
    xparms.d = Open_Display (displayName);
    screen = DefaultScreen(xparms.d);

    /* check for presence of XIE */

    printf("xieperf - XIE performance program, version 0.9\n");
    if ( !XieInitialize(xparms.d, &xieInfo ) ) {
	printf("\nXIE not supported on this display!\n");
        exit(1);
    }
    printf("\nXIE V%d.%d\n", xieInfo->server_major_rev, 
	xieInfo->server_minor_rev);

    capabilities |= ( xieInfo->service_class << 8 );	
    if ( IsFull( class_request ) && IsDIS( capabilities ) )
    {
	printf( "Service class is DIS - running DIS tests only\n" );
        class_request = SUBSET_DIS;
    }

    InitEvents( &xparms, xieInfo );
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

    cmap = XCreateColormap(xparms.d, DefaultRootWindow(xparms.d),
		XDefaultVisual(xparms.d, screen), AllocNone );
    xparms.vinfo = *vinfolist;
    XFree(vinfolist);
    if (!foreground) foreground = "Black";
    if (!background) background = "White";

    /* set depth capabilities. If it is not 8, 16, 24, then assume is mono */

    if ( xparms.vinfo.depth == 8 )
	capabilities |= CAPA_COLOR_8;
    else if ( xparms.vinfo.depth == 16 )
	capabilities |= CAPA_COLOR_16;
    else if ( xparms.vinfo.depth == 24 )
	capabilities |= CAPA_COLOR_24;

    XmuGetHostname(hostname, 100);
    printf ("%s server on %s\nfrom %s\n",
	    ServerVendor (xparms.d), DisplayString (xparms.d), hostname);
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
#ifdef SIGQUIT
    (void) signal(SIGQUIT, Cleanup);
#endif
    (void) signal(SIGTERM, Cleanup);
#ifdef SIGHUP
    (void) signal(SIGHUP, Cleanup);
#endif
    XSetScreenSaver(xparms.d, 8 * 3600, ssInterval, ssPreferBlanking, 
	ssAllowExposures);

    if (drawToFakeServer) {
        tileToQuery =
	    XCreatePixmap(xparms.d, DefaultRootWindow (xparms.d), 32, 32, 1);
    }
    AllocateRest( &xparms );
    xparms.foreground = 0;
    xparms.background = maxcoloridx;
    xparms.w = CreatePerfWindow(&xparms, 2, 2, WIDTH, HEIGHT);
    drawableWindow = CreatePerfWindowUnmapped(&xparms, 610, 0, WIDTH, HEIGHT);
    status = CreatePerfWindow(&xparms, 2, HEIGHT+5, WIDTH, 20);
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
    XWarpPointer(xparms.d, None, DefaultRootWindow( xparms.d ), 0, 0, 0, 0, 
	0, HEIGHT+40);

    /* Figure out how long to call HardwareSync, so we can adjust for that
       in our total elapsed time */
    (void) CalibrateTest(&xparms, &syncTest, 1, &syncTime);
    printf("Sync time adjustment is %6.4f msecs.\n\n", syncTime/1000);

    ForEachTest (i) {
	int child;
	char label[200];

	if (doit[i] && (test[i].versions & xparms.version) && ServerIsCapable( test[ i ].parms.description ) ) {
	    	strcpy (label, test[i].label);
		ProcessTest(&xparms, &test[i], GXcopy, ~0, label);
	} /* if doit */
    } /* ForEachTest */

    XDestroyWindow(xparms.d, xparms.w);
    /* Restore ScreenSaver to original state. */
    XSetScreenSaver(xparms.d, ssTimeout, ssInterval, ssPreferBlanking,
	ssAllowExposures);
    XCloseDisplay(xparms.d);
}

#define MAXCOLORS 256

int
AllocateRest( xp )
XParms	xp;
{
	int	i, n_colors;
	long	intensity;
	unsigned long pixels[ MAXCOLORS ];
	XColor	gray[ MAXCOLORS ];

	/* if we are monochrome, why bother */

	if ( xp->vinfo.depth == 1 )
		return;
	
	/* get what we can, and remember it. we should get all since
	   we created the colormap */

	i = MAXCOLORS;
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

       	for (i=0; i<n_colors; ++i) {
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
        	XInstallColormap(xp->d,cmap);
        	XSync(xp->d,0);
	}	
	else
	{
		fprintf( stderr, "Couldn't allocate colors in colormap\n" );
		exit( 0 );
	}
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
PumpTheClientData( xp, p, flo_id, photospace, element, data, size )
XParms	xp;
Parms	p;
int	flo_id;
XiePhotospace photospace;
int	element;
char	*data;
int	size;
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
                        element,              /* element */
                        final,          /* signal that this is all the data */
                        0,              /* band_number */
                        (unsigned char *)data,
                        nbytes
                );
                bytes_left -= nbytes;
                data += nbytes;
        }
}

int	
ReadNotifyExportData( xp, p, namespace, flo_id, element, nbytes )
XParms	xp;
Parms	p;
unsigned long namespace;
int	flo_id;
XiePhototag element;
unsigned int nbytes;
{
	char	*data = p->data;
	char	*cp;
	Bool	terminate = False;
	XieExportState new_state_ret;
	unsigned int nbytes_ret;
	Bool	no_errors;
	unsigned int bytes;

	bytes = nbytes;
	no_errors = True;
	while ( bytes > 0 )
	{  
        	XieGetClientData ( xp->d, namespace, flo_id, element,  
			bytes > 2048 ? 2048 : bytes, terminate, 0,
			&new_state_ret, (unsigned char **)&cp, &nbytes_ret );

/* this rots */

		memcpy( data, cp, nbytes_ret * sizeof( char ) ); 
		data += nbytes_ret * sizeof( char );
		free( cp );

		bytes -= nbytes_ret;
		if ( new_state_ret == xieValExportEmpty )
			WaitForExportClientData( xp, flo_id, element );		
		else if ( new_state_ret == xieValExportDone ) 
			break;
		else if ( new_state_ret == xieValExportError )
		{
			no_errors = False;
			break;
		}
	}
	if ( no_errors == False )
	{
		return( -1 );
	}
	return( nbytes - bytes );
}

static XieExtensionInfo *xieInfo=NULL;

Bool 
event_check(display,event,arg)
Display *display;
XEvent *event;
char *arg;
{
	int xie_event;

        if (xieInfo == NULL)
                return(False);

	if ( event == ( XEvent * ) NULL )
		return( False );

        xie_event = event->type - xieInfo->first_event;
        if (xie_event <= 0)
                return(False);
        if (xie_event > xieEvnNoPhotofloDone)
                return(False);
        return(True);
}

static int sigFloFinishedSeen, sigExportClientSeen;

#ifdef SIGALRM

#ifdef SIGNALRETURNSINT
int
#else
void
#endif
SigFloFinishedHandler(sig)
    int sig;
{
	sigFloFinishedSeen = 1;
}

#ifdef SIGNALRETURNSINT
int
#else
void
#endif
SigExportClientHandler(sig)
    int sig;
{
	sigExportClientSeen = 1;
}
#endif

int
WaitForFloToFinish( xp, flo_id )
XParms	xp;
int	flo_id;
{
	XEvent event;
	Bool yup;
	int retval;
	int xie_event;
	XiePhotofloDoneEvent    *floev= (XiePhotofloDoneEvent    *) &event;

	retval = 1;

	/* if we don't see the event after, say, 1 minute then 
	   something is wrong */

#ifdef SIGALRM
	signal( SIGALRM, SigFloFinishedHandler );
	alarm( timeout );
#endif
	sigFloFinishedSeen = 0;
	while ( 1 )
	{
		while ( sigFloFinishedSeen == 0 && 
			!( yup = XCheckIfEvent(xp->d,&event,event_check,0) ) );
		if ( sigFloFinishedSeen == 1 )
		{
			retval = 0;
			fprintf( stderr, "Timed out on receiving XiePhotofloDoneEvent\n" ); 
			break;
		}
		xie_event = event.type - xieInfo->first_event;
		if ( xie_event == xieEvnNoPhotofloDone )
		{
			if ( floev->flo_id != flo_id )
				continue;
			else
				break;
		}
		else
			continue;
	}

	/* turn off the signal */
#ifdef SIGALRM
	alarm( 0 );
#endif
	sigFloFinishedSeen = 0;
	return( retval );
}

int
WaitForExportClientData( xp, flo_id, element )
XParms	xp;
int	flo_id;
XiePhototag element;
{
	XEvent event;
	int retval;
	Bool yup;
	int xie_event;
	XieExportAvailableEvent    *floev= (XieExportAvailableEvent *) &event;

	/* if we don't see the event after, say, 1 minutes then 
	   something is ( terribly ) wrong */

	retval = 1;
#ifdef SIGALRM
	signal( SIGALRM, SigExportClientHandler );
	alarm( timeout );
#endif
	sigExportClientSeen = 0;
	while ( 1 )
	{
		while ( sigExportClientSeen == 0 && 
			!( yup = XCheckIfEvent(xp->d,&event,event_check,0) ) );
		if ( sigExportClientSeen == 1 )
		{
			retval = 0;
			fprintf( stderr, "Timed out on receiving XieExportAvailableEvent\n" ); 
			break;
		}
		xie_event = event.type - xieInfo->first_event;
		if ( xie_event == xieEvnNoExportAvailable )
		{
			if ( floev->flo_id != flo_id || floev->src != element )
				continue;
			else
				break;
		}
		else
		{
			continue;
		}
	}

	/* turn off the signal */
#ifdef SIGALRM
	alarm( 0 );
#endif
	sigExportClientSeen = 0;
	return( retval );
}

InitEvents(display,info)
Display *display;
XieExtensionInfo *info;
{
        xieInfo = info;
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
GetXIEFAXPhotomap( xp, p, which )
XParms	xp;
Parms	p;
int	which;
{
	XIEimage	*image;
        XiePhotospace photospace;
	int	flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieDecodeG42DParam *g42d_decode_params=NULL;
	XieDecodeG32DParam *g32d_decode_params=NULL;
	XieDecodeG31DParam *g31d_decode_params=NULL;
	char *decode_params;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
	unsigned char pixel_stride[ 3 ];
	unsigned char left_pad[ 3 ];
	unsigned char scanline_pad[ 3 ];
	XiePhotomap	tmp;
	int	size;

	if ( !GetImageData( xp, p, which ) )
		return( ( XiePhotomap ) NULL );

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
	size = image->fsize;
	image->chksum = CheckSum( p->data, size );
	width[ 0 ] = image->width;
	height[ 0 ] = image->height;
	levels[ 0 ] = image->levels;

	/* create a photomap */

        tmp = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	if ( image->decode == xieValDecodeG42D )  {
            g42d_decode_params = XieTecDecodeG42D(
                image->fill_order, 
		True	/* XXX needs config */
            );
	    decode_params = (char *) g42d_decode_params;
	}
	else if ( image->decode == xieValDecodeG32D )  {
            g32d_decode_params = XieTecDecodeG32D(
                image->fill_order, 
		True	/* XXX needs config */
            );
	    decode_params = (char *) g32d_decode_params;
	}
	else if ( image->decode == xieValDecodeG31D )  {
            g31d_decode_params = XieTecDecodeG31D(
                image->fill_order, 
		True	/* XXX needs config */
            );
	    decode_params = (char *) g31d_decode_params;
	}
	else {
	   fprintf(stderr, " %s(%d), unexpected decode, %d\n",
		__FILE__,__LINE__,image->decode);
	   exit(1);
	}

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEFAXPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, tmp );
                return( XiePhotomap ) NULL;
        }

        XieFloImportClientPhoto(&flograph[0],
                image->data_class,
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
       	PumpTheClientData( xp, p, flo_id, photospace, 1, p->data, size );
        WaitForFloToFinish( xp, flo_id );
        XieFreePhotofloGraph(flograph,2);
        XieDestroyPhotospace(xp->d, photospace);
	free( decode_params );
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
	XieDecodeUncompressedTripleParam *decode_params=NULL;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
	unsigned char pixel_stride[ 3 ];
	unsigned char left_pad[ 3 ];
	unsigned char scanline_pad[ 3 ];
	XiePhotomap	tmp;
	int	size;

	if ( !GetImageData( xp, p, which ) )
		return( ( XiePhotomap ) NULL );

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
	
	size = image->fsize;
	image->chksum = CheckSum( p->data, size );

	/* create a photomap */

        tmp = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	width[ 0 ] = width[ 1 ] = width[ 2 ] = image->width;
	height[ 0 ] = height[ 1 ] = height[ 2 ] = image->height;
	levels[ 0 ] = levels[ 1 ] = levels[ 2 ] = image->levels;

	pixel_stride[ 0 ] = pixel_stride[ 1 ] = pixel_stride[ 2 ] =
		image->pixel_stride;
	left_pad[ 0 ] = left_pad[ 1 ] = left_pad[ 2 ] =
		image->left_pad;
	scanline_pad[ 0 ] = scanline_pad[ 1 ] = scanline_pad[ 2 ] =
		image->scanline_pad;

        decode_params = XieTecDecodeUncompressedTriple(
                image->fill_order,
		xieValLSFirst,		/* XXX needs config */
		xieValLSFirst,		/* XXX needs config */
		xieValBandByPixel,	/* XXX needs config */
                pixel_stride,
                left_pad,
                scanline_pad
        );

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIETriplePhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, tmp );
                return( XiePhotomap ) NULL;
        }

        XieFloImportClientPhoto(&flograph[0],
                image->data_class,
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
       	PumpTheClientData( xp, p, flo_id, photospace, 1, p->data, size );
        WaitForFloToFinish( xp, flo_id );
        XieFreePhotofloGraph(flograph,2);
        XieDestroyPhotospace(xp->d, photospace);
	free( decode_params );
    	return tmp;
}

static XieLTriplet MONOlevels;
static XieConstrainTechnique MONOtech = xieValConstrainClipScale;
static XieClipScaleParam *MONOparms;
static XieConstant MONOin_low,MONOin_high;
static XieLTriplet MONOout_low,MONOout_high;
static int monoflag = 0;

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

	if ( !GetImageData( xp, p, which ) )
		return( ( XiePhotomap ) NULL );

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
	size = image->fsize;
	image->chksum = CheckSum( p->data, size );

	width[ 0 ] = image->width;
	height[ 0 ] = image->height;
	levels[ 0 ] = image->levels;

	/* create a photomap */

        tmp = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

        decode_params = XieTecDecodeUncompressedSingle(
                image->fill_order,
                image->pixel_order,
                image->pixel_stride,
                image->left_pad,
                image->scanline_pad
        );

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEPhotomap: XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, tmp );
                return( XiePhotomap ) NULL;
        }

        XieFloImportClientPhoto(&flograph[0],
                image->data_class,
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
       	PumpTheClientData( xp, p, flo_id, photospace, 1, p->data, size );
        WaitForFloToFinish( xp, flo_id );
        XieFreePhotofloGraph(flograph,2);
        XieDestroyPhotospace(xp->d, photospace);
	free( decode_params );
    	return tmp;
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
	int	flo_elements, flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieDecodeUncompressedSingleParam *decode_params=NULL;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
	int	size;

	p->data = ( char * ) NULL;
	if ( !GetImageData( xp, p, which ) )
		return( 0 );

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
		return( 0 );
	size = image->fsize;
	image->chksum = CheckSum( p->data, size );

	MONOparms = NULL;
	monoflag = 0;
	if ( xp->vinfo.depth == 1 )
	{
		monoflag = 1;
		if ( !SetupMonoClipScale( image, MONOlevels, MONOin_low, 
			MONOin_high, MONOout_low, MONOout_high, &MONOparms ) )
		{
			if ( p->data )
			{
				free( p->data );
				p->data = ( char * ) NULL;
			}
			return 0;
		}
	}
	else
		monoflag = 0;

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	width[ 0 ] = image->width;
	height[ 0 ] = image->height;
	levels[ 0 ] = image->levels;

        decode_params = XieTecDecodeUncompressedSingle(
                image->fill_order,
                image->pixel_order,
                image->pixel_stride,
                image->left_pad,
                image->scanline_pad
        );

	if ( monoflag )
	{
		flo_elements = 3;
	}
	else
	{
		flo_elements = 2;
	}
        flograph = XieAllocatePhotofloGraph(flo_elements);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEPixmap: XieAllocatePhotofloGraph failed\n" );
		if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
		if ( MONOparms )
			free( MONOparms );
		free( decode_params );
                return( 0 );
        }

        XieFloImportClientPhoto(&flograph[0],
                image->data_class,
                width, height, levels,
                False,
                image->decode, (char *)decode_params
        );

	if ( monoflag)
	{ 
	       XieFloConstrain(&flograph[1],
			1,
			MONOlevels,
			MONOtech,
			(char *)MONOparms
		);
	}

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
	PumpTheClientData( xp, p, flo_id, photospace, 1, p->data, size );
        WaitForFloToFinish( xp, flo_id );
        XieFreePhotofloGraph(flograph,flo_elements);
	XieDestroyPhotospace( xp->d, photospace );
	free( decode_params );
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	if ( MONOparms )
		free( MONOparms );
    	return 1;
}

int
GetXIEWindow( xp, p, window, which )
XParms	xp;
Parms	p;
Window 	window;
int	which;
{
	XIEimage	*image;
        XiePhotospace photospace;
	int	flo_elements, flo_id, flo_notify;
        XiePhotoElement *flograph;
	XieDecodeUncompressedSingleParam *decode_params=NULL;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XieLTriplet width, height, levels;
	int	size;

	p->data = ( char * ) NULL;
	if ( !GetImageData( xp, p, which ) )
		return( 0 );

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
		return( 0 );
	size = image->fsize;
	image->chksum = CheckSum( p->data, size );

	MONOparms = NULL;
        if ( xp->vinfo.depth == 1 )
        {
                monoflag = 1;
                if ( !SetupMonoClipScale( image, MONOlevels, MONOin_low,
                        MONOin_high, MONOout_low, MONOout_high, &MONOparms ) )
		{
			if ( p->data )
			{
				free( p->data );
				p->data = ( char * ) NULL;
			}
                        return 0;
		}
        }
        else
                monoflag = 0;

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	width[ 0 ] = image->width;
	height[ 0 ] = image->height;
	levels[ 0 ] = image->levels;

        decode_params = XieTecDecodeUncompressedSingle(
                image->fill_order,
                image->pixel_order,
                image->pixel_stride,
                image->left_pad,
                image->scanline_pad
        );

        if ( monoflag )
        {
                flo_elements = 3;
        }
        else
        {
                flo_elements = 2;
        }

        flograph = XieAllocatePhotofloGraph(flo_elements);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEWindow: XieAllocatePhotofloGraph failed\n" );
		if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
		if ( MONOparms )
			free( MONOparms );
		free( decode_params );
                return( 0 );
        }

        XieFloImportClientPhoto(&flograph[0],
                image->data_class,
                width, height, levels,
                False,
                image->decode, (char *)decode_params
        );

        if ( monoflag )
	{
		XieFloConstrain(&flograph[1],
			1,
			MONOlevels,
			MONOtech,
			(char *)MONOparms
		);
	}

        XieFloExportDrawable(&flograph[flo_elements - 1],
		flo_elements - 1,     /* source phototag number */
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
	PumpTheClientData( xp, p, flo_id, photospace, 1, p->data, size );
        WaitForFloToFinish( xp, flo_id );
        XieFreePhotofloGraph(flograph,flo_elements);
	XieDestroyPhotospace( xp->d, photospace );
	free( decode_params );
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	if ( MONOparms )
		free( MONOparms );
    	return 1;
}

XiePhotomap
GetXIEBitonalPhotomap( xp, p, which )
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
        char    *tech_parms=NULL;
	XiePhotomap tmp;
	int	size;

	p->data = ( char * ) NULL;
	if ( !GetImageData( xp, p, which ) )
		return ( XiePhotomap ) NULL;

	if ( which == 1 )
	{
		image = p->finfo.image1;
	}
	else if ( which == 2 )
	{
		image = p->finfo.image2;
	}
	else if ( which == 3 )
	{
		image = p->finfo.image3;
	}
	if ( !image )
		return( XiePhotomap ) NULL;
	size = image->fsize;
	image->chksum = CheckSum( p->data, size );

	width[ 0 ] = image->width;
	height[ 0 ] = image->height;
	levels[ 0 ] = image->levels;

	/* create a photomap */

        tmp = XieCreatePhotomap(xp->d);

	/* get the data from the client into the photomap */

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

        decode_params = XieTecDecodeUncompressedSingle(
                image->fill_order,
                image->pixel_order,
                image->pixel_stride,
                image->left_pad,
                image->scanline_pad
        );

        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "GetXIEBitonalPhotomap: XieAllocatePhotofloGraph failed\n" );
		free( decode_params );
		if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
		XieDestroyPhotomap( xp->d, tmp );
                return ( XiePhotomap ) NULL;
        }

        XieFloImportClientPhoto(&flograph[0],
                image->data_class,
                width, height, levels,
                False,
                image->decode, (char *)decode_params
        );

        levels[ 0 ] = 2;		/* bitonal */
        levels[ 1 ] = 0;
        levels[ 2 ] = 0;

	tech_parms = ( char * ) NULL;

	XieFloDither(&flograph[ 1 ],
		1,
	        levels,
		xieValDitherErrorDiffusion,
		tech_parms
	);
 
        XieFloExportPhotomap(&flograph[2],
                2,              /* source phototag number */
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
                3               /* number of elements */
        );
        XSync( xp->d, 0 );
	PumpTheClientData( xp, p, flo_id, photospace, 1, p->data, size );
        WaitForFloToFinish( xp, flo_id );
        XieFreePhotofloGraph(flograph,3);
	free( decode_params );
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
    	return tmp;
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
	char	*name;
	char	buf[ 64 ];
	XIEimage *image;

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
	size = &image->fsize;
	name = image->fname;

        /* validate test parameters XXX need to be more complete */

        if ( p->buffer_size <= 0 )
        {
                fprintf( stderr, "buffer_size is invalid\n" );
                return 0;
        }

	sprintf( buf, "%s/%s", imagepath, name );

        /* open the file */

        if ( ( *size = file_size( buf ) ) < 0 )
        {
                fprintf( stderr, "Couldn't stat %s\n", buf );
                return 0;
        }
        if ( ( fd = open( buf, O_RDONLY|O_BINARY ) ) == -1 )
        {
                fprintf( stderr, "Couldn't open %s\n", buf );
                goto out;
        }

        /* allocate the data buffer */

        if ( ( p->data = (char *)malloc( *size ) ) == ( char * ) NULL )
        {
                fprintf( stderr, "Couldn't allocate buffer\n" );
                goto out;
        }

        /* read the data */

        if ( ( n = read( fd, p->data, *size ) ) != *size )
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
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	return 0;
}

void
CloseXIEPhotomap( xp, p, pmap )
XParms	xp;
Parms	p;
XiePhotomap pmap;
{
	XieDestroyPhotomap( xp->d, pmap );
	free( p->data );
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
	XieDataClass	data_class;
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

	data_class = xieValSingleBand;
	band_order = xieValLSFirst;
	length[ 0 ] = lutSize;
	length[ 1 ] = 0;
	length[ 2 ] = 0;
	levels[ 0 ] = lutLevels;
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;

        XieFloImportClientLUT(&flograph[0],
                data_class,
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
        PumpTheClientData( xp, p, flo_id, photospace, 1, lut, lutSize );
        WaitForFloToFinish( xp, flo_id );
        XieFreePhotofloGraph(flograph,2);
    	return tmp;
}

void
CloseXIELut( xp, p, lut )
XParms	xp;
Parms	p;
XieLut	lut;
{
	XieDestroyLUT( xp->d, lut );
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
		* sizeof( XieRectangle ) );
        WaitForFloToFinish( xp, flo_id );
 
        XieFreePhotofloGraph(flograph,2);
    	return tmp;
}

void
CloseXIERoi( xp, p, roi )
XParms	xp;
Parms	p;
XieRoi	roi;
{
	XieDestroyROI( xp->d, roi );
}

int
ServerIsCapable( testcp )
unsigned short testcp;
{
	if ( IsColor8( testcp ) && !IsColor8( capabilities ) )
	{
		return( 0 );
	}
	if ( IsColor16( testcp ) && !IsColor16( capabilities ) )
	{
		return( 0 );
	}
	if ( IsColor24( testcp ) && !IsColor24( capabilities ) )
	{
		return( 0 );
	}
	if ( ( testcp & SUBSET_MASK ) != ( class_request & SUBSET_MASK ) )
	{
		return( 0 );
	}
	return( 1 );
}

int
SetupMonoClipScale( image, levels, in_low, in_high, out_low, out_high, parms )
XIEimage    *image;
XieLTriplet levels;
XieConstant in_low, in_high;
XieLTriplet out_low, out_high;
XieClipScaleParam **parms;
{
        levels[ 0 ] = 2;
        levels[ 1 ] = 0;
        levels[ 2 ] = 0;

        in_low[ 0 ] = 0.0;
        in_high[ 0 ] = ( float ) ( 1 << image->depth ) - 1.0;
        out_low[ 0 ] = 0;
        out_high[ 0 ] = 1;

        *parms = XieTecClipScale( in_low, in_high, out_low, out_high );
        if ( *parms == ( XieClipScaleParam * ) NULL )
        {
                fprintf( stderr, "Couldn't allocate clip scale parameters\n" );
                return( 0 );
        }
        return( 1 );
}

int
SetupFaxClipScale( xp, p, levels, in_low, in_high, out_low, out_high, parms )
XParms	xp;
Parms   p;
XieLTriplet levels;
XieConstant in_low, in_high;
XieLTriplet out_low, out_high;
XieClipScaleParam **parms;
{
        levels[ 0 ] = ( 1 << xp->vinfo.depth );
        levels[ 1 ] = 0;
        levels[ 2 ] = 0;

        in_low[ 0 ] = 0.0;
        in_high[ 0 ] = 1.0;
        out_low[ 0 ] = 0;
        out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;

        *parms = XieTecClipScale( in_low, in_high, out_low, out_high );
        if ( *parms == ( XieClipScaleParam * ) NULL )
        {
                fprintf( stderr, "Couldn't allocate clip scale parameters\n" );
                return( 0 );
        }
        return( 1 );
}

int
GetTimeout()
{
	return(	timeout );
}
