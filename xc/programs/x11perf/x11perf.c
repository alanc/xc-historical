/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#include <stdio.h>
#ifndef VMS
#include <X11/Xatom.h>
#include <sys/time.h>
#else
#include <decw$include/Xatom.h>
#endif
#include "x11perf.h"

static Bool drawToFakeServer = False;
static Pixmap tileToQuery = None;

static double syncTime = 0.0;

static int saveargc;
static char **saveargv;

static char *foreground = NULL;
static char *background = NULL;

static Bool *doit;

static XRectangle ws[] = {  /* Clip rectangles */
    {240, 240, 120, 120},
    { 40, 190, 120, 120},
    {440, 290, 120, 120}

};
#define MAXCLIP     (sizeof(ws) / sizeof(ws[0]))
static Window clipWindows[MAXCLIP];

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

    double exponent;

    exponent = 1.0;
    if (d >= 1000.0) {
	do {
	    exponent *= 10.0;
	} while (d/exponent >= 1000.0);
	d = (double)((int) (d/exponent + 0.5));
	d *= exponent;
    } else {
	while (d*exponent < 100.0) {
	    exponent *= 10.0;
	}
	d = (double)((int) (d*exponent + 0.5));
	d /= exponent;
    }
    return d;
}


void ReportTimes(usecs, n, str)
    double  usecs;
    int     n;
    char    *str;
{
    double msecsperobj, objspersec;

    msecsperobj = usecs / (1000.0 * (double)n);
    objspersec = (double) n * 1000000.0 / usecs;

    /* Round obj/sec to 3 significant digits.  Leave msec untouched, to allow
	averaging results from several repetitions. */
    objspersec =  RoundTo3Digits(objspersec);

    printf("%6d reps @ %7.4f msec (%6.1f/sec): %s\n", 
	n, msecsperobj, objspersec, str);
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

/************************************************
*		Performance stuff		*
************************************************/


void usage()
{
    char    **cpp;
    int     i = 0;
    static char *help_message[] = {
"where options include:",
"    -display host:dpy		the X server to contact",
"    -sync			do the tests in synchronous mode",
"    -repeat <n>		do tests <n> times (default = 5)",
"    -time <s>			do tests for <s> seconds each (default = 5)",
"    -draw			draw after each test -- pmax only",
"    -all			do all tests",
"    -fg			the foreground color to use",
"    -bg		        the background color to use",
NULL};

    fflush(stdout);
    fprintf(stderr, "usage: %s [-options ...]\n", program_name);
    for (cpp = help_message; *cpp; cpp++) {
	fprintf(stderr, "%s\n", *cpp);
    }
    while (test[i].option != NULL) {
        fprintf(stderr, "    %-28s   %s\n",
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

Bool NullInitProc(xp, p)
    XParms  xp;
    Parms   p;
{
    return True;
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

    image = XGetImage(xp->d, xp->w, 1, 1, 1, 1, ~0, ZPixmap);
    XDestroyImage(image);
}

void DoHardwareSync(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;
    
    for (i = 0; i != p->reps; i++) {
	HardwareSync(xp);
    }
}

static Test syncTest = {
    "syncTime", "Internal test for finding how long HardwareSync takes",
    NullInitProc, DoHardwareSync, NullProc, NullProc, False, 0,
    {5000, 1}
};


static Window CreatePerfWindow(xp, x, y, width, height)
    XParms  xp;
    int     width, height, x, y;
{
    XSetWindowAttributes xswa;
    Window w;

    xswa.override_redirect = True;
    w = XCreateSimpleWindow (xp->d, RootWindow (xp->d, 0),
	x, y, width, height, 1, xp->foreground, xp->background);
    XChangeWindowAttributes (xp->d, w, CWOverrideRedirect, &xswa);
    XMapWindow (xp->d, w);
    return w;
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


DoTest(xp, test, label)
    XParms  xp;
    Test    *test;
    char    *label;
{
    double  time;
    int     ret_width, ret_height;

    HardwareSync (xp);
    InitTimes ();
    (*test->proc) (xp, &test->parms);
    HardwareSync(xp);

    time = ElapsedTime(syncTime);
    ReportTimes (time, test->parms.reps * test->parms.objects, label);
    if (drawToFakeServer)
        XQueryBestSize(xp->d, TileShape, tileToQuery,
		       32, 32, &ret_width, &ret_height);
    (*test->passCleanup) (xp, &test->parms);
}


Bool CalibrateTest(xp, test, seconds, usecperobj)
    XParms  xp;
    Test    *test;
    int     seconds;
    double  *usecperobj;
{
#define goal    1500000.0   /* Try to get up to 1.5 seconds		    */
#define enough  1000000.0   /* But settle for 1.0 seconds		    */
#define tick      10000.0   /* Assume clock not faster than .01 seconds     */

    double  usecs;
    int     reps, exponent;

    /* Attempt to get an idea how long each rep lasts by getting enough
       reps to last more than a second.

       If init call to test ever fails, return False and test will be skipped.
    */

    reps = 1;
    for (;;) {
	test->parms.reps = reps;
	XDestroySubwindows(xp->d, xp->w);
	XClearWindow(xp->d, xp->w);
	if (! ((*test->init) (xp, &test->parms))) {
	    return False;
	}
	/* Create clip windows if requested */
	CreateClipWindows(xp, test->clips);
	HardwareSync(xp);
	InitTimes();
	(*test->proc) (xp, &test->parms);
	HardwareSync(xp);
	usecs = ElapsedTime(syncTime);
	(*test->passCleanup) (xp, &test->parms);
	(*test->cleanup) (xp, &test->parms);
	DestroyClipWindows(xp, test->clips);

	if (reps > test->parms.reps) {
	    /* The test can't do as many reps as we asked for.  Give up */
	    *usecperobj =  usecs / (double)test->parms.reps;
	    return True;
	}
	/* Did we go long enough? */
	if (usecs >= enough) break;

	/* Don't let too short a clock make new reps wildly high */
	if (usecs < tick) usecs = tick;

	/* Try to get up to goal seconds. */
	reps = (int) (goal * (double)reps / usecs) + 1;
    }

    *usecperobj = usecs / (double)reps;
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
    test->parms.reps = reps;
    return True;
} /* CalibrateTest */


void CreatePerfGCs(xp)
    XParms  xp;
{
    XGCValues gcv;

    gcv.foreground = xp->background;
    gcv.background = xp->foreground;
    xp->bggc = XCreateGC(xp->d, xp->w, GCForeground | GCBackground , &gcv);

    gcv.foreground = xp->foreground;
    gcv.background = xp->background;
    xp->fggc = XCreateGC(xp->d, xp->w, GCForeground | GCBackground , &gcv);
}

void DestroyPerfGCs(xp)
    XParms(xp);
{
    XFreeGC(xp->d, xp->fggc);
    XFreeGC(xp->d, xp->bggc);
}

int AllocateColor(display, name, pixel)
    Display     *display;
    char	*name;
    int		pixel;
{
    XColor      color;
    Colormap    cmap;

    if (name != NULL) {
	cmap = XDefaultColormap(display, DefaultScreen(display));

	/* Try to parse color name */
	if (XParseColor(display, cmap, name, &color)) {
	    if (XAllocColor(display, cmap, &color)) {
		pixel = color.pixel;
	    } else {
		(void) fprintf(stderr,
		    "Can't allocate colormap entry for color %s\n", name);
	    }
	} else {
	    (void) fprintf(stderr, "Can't parse color name %s\n", name);
	}
    }
    return pixel;
} /* AllocateColor */


void DisplayStatus(d, w, tgc, message, test)
    Display *d;
    Window  w;
    GC      tgc;
    char    *message;
    char    *test;
{
    char    s[500];

    XClearWindow(d, w);
    sprintf(s, "%s %s", message, test);
    /* We should really look at the height, descent of the font, etc. but
       who cares.  This works. */
    XDrawString(d, w, tgc, 10, 13, s, strlen(s));
}


main(argc, argv)
    int argc;
    char **argv;

{
    int     i, j;
    int     numTests;       /* Even though the linker knows, we don't. */
    char    hostname[100];
    char   *displayName;
    XParmRec    xparms;
    int     repeat = 5;
    int     seconds = 5;
    Bool    foundOne = False;
    Bool    synchronous = False;
    Window  status;
    GC      tgc;	    

    /* ScreenSaver state */
    int ssTimeout, ssIntervalReturn, ssPreferBlanking, ssAllowExposures;

    /* Save away argv, argc, for usage to print out */
    saveargc = argc;
    saveargv = (char **) malloc(argc * sizeof(char *));
    for (i = 0; i != argc; i++) {
	saveargv[i] = argv[i];
    }

    /* Count number of tests */
    ForEachTest(numTests);
    doit = (Bool *)calloc(numTests, sizeof(Bool));

    /* Parse arguments */
    program_name = argv[0];
    displayName = Get_Display_Name (&argc, argv);
    for (i = 1; i != argc; i++) {
	if (strcmp (argv[i], "-all") == 0) {
	    ForEachTest (j)
		doit[j] = True;
	    foundOne = True;
	} else if (strcmp (argv[i], "-sync") == 0) {
	    synchronous = True;
	} else if (strcmp (argv[i], "-draw") == 0) {
	    drawToFakeServer = True;
	} else if (strcmp (argv[i], "-repeat") == 0) {
	    if (argc <= i)
		usage ();
	    repeat = atoi (argv[++i]);
	    if (repeat <= 0)
	       usage ();
	} else if (strcmp (argv[i], "-time") == 0) {
	    if (argc <= i)
		usage ();
	    seconds = atoi (argv[++i]);
	    if (seconds <= 0)
	       usage ();
	} else if (strcmp(argv[i], "-fg") == 0) {
	    if (argc <= i)
		usage ();
	    i++;
	    foreground = argv[i];
        } else if (strcmp(argv[i], "-bg") == 0) {
	    if (argc <= i)
		usage ();
	    i++;
	    background = argv[i];
	} else {
	    ForEachTest (j) {
		if (strcmp (argv[i], test[j].option) == 0) {
		    doit[j] = True;
		    goto LegalOption;
		}
	    }
	    usage ();
	LegalOption: 
		foundOne = True;
	}
    }
    if (!foundOne)
	usage ();
    xparms.d = Open_Display (displayName);
#ifndef VMS
    gethostname (hostname, 100);
    printf ("%s server on %s from %s\n",
	    ServerVendor (xparms.d), DisplayString (xparms.d), hostname);
#else
    printf ("%s server on %s\n",
	    ServerVendor (xparms.d), DisplayString (xparms.d));
#endif
    PrintTime ();

/* ||| Doesn't seem to work so well.
    XGetScreenSaver(xparms.d, &ssTimeout, &ssIntervalReturn, &ssPreferBlanking,
	&ssAllowExposures);
    XForceScreenSaver(xparms.d, ScreenSaverReset);
    XSetScreenSaver(xparms.d, 0, 0, DefaultBlanking, DefaultExposures);
*/

    if (drawToFakeServer) {
        tileToQuery =
	    XCreatePixmap(xparms.d, RootWindow (xparms.d, 0), 32, 32, 1);
    }


    xparms.foreground =
	AllocateColor(xparms.d, foreground, BlackPixel(xparms.d, 0));
    xparms.background =
	AllocateColor(xparms.d, background, WhitePixel(xparms.d, 0));
    xparms.w = CreatePerfWindow(&xparms, 2, 2, WIDTH, HEIGHT);
    status = CreatePerfWindow(&xparms, 2, HEIGHT+5, WIDTH, 20);
    tgc = XCreateGC(xparms.d, status, 0, NULL);
    

    if (synchronous)
	XSynchronize (xparms.d, True);

    /* Figure out how long to call HardwareSync, so we can adjust for that
       in our total elapsed time */
    (void) CalibrateTest(&xparms, &syncTest, 1, &syncTime);
    printf("Sync time adjustment is %6.4f msecs.\n\n", syncTime/1000);

    ForEachTest (i) {
	if (doit[i]) {
	    int     child = 0;
	    char    label[100];
	    int     reps = test[i].parms.reps;
	    double  junk;

	    CreatePerfGCs(&xparms);
	    while (1) {
		if (test[i].children) {
		    test[i].parms.objects = subs[child];
		    if (test[i].parms.objects == 0)
			break;
		    sprintf (label, "%s (%d kids)",
			    test[i].label, test[i].parms.objects);
		} else {
		    strcpy (label, test[i].label);
		}
		DisplayStatus(xparms.d, status, tgc, "Calibrating", label);
		if (CalibrateTest(&xparms, &test[i], seconds, &junk)) {
		    DisplayStatus(xparms.d, status, tgc, "Testing", label);
		    XDestroySubwindows(xparms.d, xparms.w);
		    XClearWindow(xparms.d, xparms.w);
		    (void)(*test[i].init) (&xparms, &test[i].parms);
		    /* Create clip windows if requested */
		    CreateClipWindows(&xparms, test[i].clips);

		    for (j = 0; j != repeat; j++)
			DoTest (&xparms, &test[i], label);
		    (*test[i].cleanup) (&xparms, &test[i].parms);
		    DestroyClipWindows(&xparms, test[i].clips);
		} else {
		    /* Test failed to initialize properly */
		}
		printf ("\n");
		if (!test[i].children)
		    break;
		child++;
	    }
	    DestroyPerfGCs(&xparms);
	}
    }
    XDestroyWindow(xparms.d, xparms.w);
/*
    XSetScreenSaver(xparms.d, ssTimeout, ssIntervalReturn, ssPreferBlanking,
	ssAllowExposures);
*/
}

