#include <stdio.h>
#ifndef VMS
#include <X11/Xatom.h>
#include <sys/time.h>
#else
#include <decw$include/Xatom.h>
#endif
#include "x11perf.h"

static Bool drawToGPX = False;
static Pixmap tileToQuery = None;

static char *foreground = NULL;
static char *background = NULL;
int fgPixel, bgPixel;

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

    if (firsttime)
    {
        sys$gettim(&basetime);
        firsttime = False;
    }
    sys$gettim(&current_time);
    resultant.high = current_time.high - basetime.high;
    resultant.low = current_time.low - basetime.low;
    if (current_time.low < basetime.low)
    {
        resultant.high -= 1;
    }
    status = lib$ediv( &(10000000), &resultant, &tp->tv_sec, &tp->tv_usec);
    tp->tv_usec /= 10;
    return 0;
}

#endif

/*             time related stuff */

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

void GetTime (usec)
    int *usec;
{
    struct timeval stop;
    struct timezone foo;
    
    gettimeofday(&stop, &foo);
    if (stop.tv_usec < start.tv_usec)
    {
        stop.tv_usec += 1000000;
	stop.tv_sec -= 1;
    }
    *usec = stop.tv_usec - start.tv_usec +
            (1000000 * (stop.tv_sec - start.tv_sec));
}

void ReportTimes(r, n, str)
    int  r;
    int n;
    char *str;
{
    r /= n;
    printf("%6d repetitions @ %2d.%03d millisec/%s\n", n, r/1000, r%1000, str);
}

/*					generic X stuff */
static char *program_name;
void usage();

/*
 * Get_Display_Name (argc, argv) Look for -display, -d, or host:dpy (obselete)
 * If found, remove it from command line.  Don't go past a lone -.
 */
char *Get_Display_Name(pargc, argv)
    int *pargc;  /* MODIFIED */
    char **argv; /* MODIFIED */
{
    int argc = *pargc;
    char **pargv = argv+1;
    char *displayname = NULL;
    int i;

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (!strcmp (arg, "-display") || !strcmp (arg, "-d")) {
	    if (++i >= argc) usage ();

	    displayname = argv[i];
	    *pargc -= 2;
	    continue;
	}
	if (!strcmp(arg,"-")) {
		while (i<argc)
			*pargv++ = argv[i++];
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

/*		stuff just for perf */

void usage()
{
    char **cpp;
    int i = 0;
    static char *help_message[] = {
"where options include:",
"    -display host:dpy		the X server to contact",
"    -sync			do the tests in synchronous mode",
"    -repeat <n>\t		do tests <n> times (default = 5)",
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
    while (test[i].option != NULL)
    {
        fprintf(stderr, "    %-28s   %s\n",
		test[i].option, test[i].label);
        i++;
    }
    fprintf(stderr, "\n");
    exit (1);
}

/* Main window created (if any) that the current benchmark draws to. */
static Window perfWindow;

DoTest(d, test, label)
    Display *d;
    Test *test;
    char *label;
{
    int     r;
    int     ret_width, ret_height;

    XSync (d, 0);
    InitTimes ();
    (*test->proc) (d, &test->parms);
    if (perfWindow == NULL) {
	XSync (d, 0);
    } else {
	/*
	 * Some graphics hardware allows the server to claim it is done,
	 * while in reality the hardware is busily working away.  So fetch
	 * a pixel from the window that was drawn to, which should be
	 * enough to make the server wait for the graphics hardware.
	 */
	XImage *i;
	i = XGetImage(d, perfWindow, 1, 1, 1, 1, ~0, ZPixmap);
	XDestroyImage(i);
    }

    GetTime (&r);
    ReportTimes (r, test->parms.reps * test->parms.objects, label);
    if (drawToGPX)
        XQueryBestSize(d, TileShape, tileToQuery,
		       32, 32, &ret_width, &ret_height);
    if (test->passCleanup != NULL)
	(*test->passCleanup) (d, &test->parms);
}


Window CreatePerfWindow(d, x, y, width, height)
    Display *d;
    int width, height, x, y;
{
    XSetWindowAttributes xswa;
    Window w;

    xswa.override_redirect = True;
    w = XCreateSimpleWindow (
	    d, RootWindow (d, 0), x, y, width, height, 1, fgPixel, bgPixel);
    XChangeWindowAttributes (d, w, CWOverrideRedirect, &xswa);
    XMapWindow (d, w);
    return w;
}



void CreatePerfStuff(d, count, width, height, w, bggc, fggc)
    Display *d;
    int width, height, count;
    Window *w;
    GC *bggc, *fggc;
{
    XGCValues gcv;
    XSetWindowAttributes xswa;
    int i;

    xswa.override_redirect = True;
    for (i = 0; i < count; i++) {
	w[i] = CreatePerfWindow(d, 50+i*width, 50+i*height, width, height);
    }

    if (count != 0) {
	/* Stash away main graphics window */
	perfWindow = w[0];
    }

    if (bggc != NULL) {
#ifdef testingxor
	gcv.foreground = bgPixel ^ fgPixel;
	gcv.background = ~gcv.foreground;
	gcv.function = GXinvert;
	gcv.plane_mask = bgPixel ^ fgPixel;
	*bggc = XCreateGC(d, w[0],
	  GCForeground | GCBackground | GCFunction | GCPlaneMask, &gcv);
#else
# ifdef testingandor
	gcv.foreground = 1;
	gcv.background = 0;
	gcv.function = GXor;
	gcv.plane_mask = 1;
	*bggc = XCreateGC(d, w[0],
	  GCForeground | GCBackground | GCFunction | GCPlaneMask, &gcv);
# else
	gcv.foreground = bgPixel;
	gcv.background = fgPixel;
	*bggc = XCreateGC(d, w[0], GCForeground | GCBackground , &gcv);
# endif
#endif
    }
    if (fggc != NULL) {
#ifdef testingxor
	gcv.foreground = bgPixel ^ fgPixel;
	gcv.background = ~gcv.foreground;
	gcv.function = GXxor;
	*fggc = XCreateGC(d, w[0],
	  GCForeground | GCBackground | GCFunction, &gcv);
#else
# ifdef testingandor
	gcv.foreground = 0;
	gcv.background = 1;
	gcv.function = GXand;
	*fggc = XCreateGC(d, w[0],
	  GCForeground | GCBackground | GCFunction, &gcv);
# else
	gcv.foreground = fgPixel;
	gcv.background = bgPixel;
	*fggc = XCreateGC(d, w[0], GCForeground | GCBackground , &gcv);
# endif
#endif
    }
}

Window root;
main(argc, argv)
    int argc;
    char **argv;

{
    int     i, j;
    char    hostname[100];
    char   *displayName;
    Display * display;
    int     repeat = 5;
    Bool foundOne = False;
    Bool synchronous = False;

    program_name = argv[0];
    displayName = Get_Display_Name (&argc, argv);
    for (i = 1; i < argc; i++) {
	if (strcmp (argv[i], "-all") == 0) {
	    ForEachTest (j)
		test[j].doit = True;
	    foundOne = True;
	} else if (strcmp (argv[i], "-sync") == 0) {
	    synchronous = True;
	} else if (strcmp (argv[i], "-draw") == 0) {
	    drawToGPX = True;
	} else if (strcmp (argv[i], "-repeat") == 0) {
	    if (argc <= i)
		usage ();
	    repeat = atoi (argv[++i]);
	    if (repeat == 0)
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
		    test[j].doit = True;
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
    display = Open_Display (displayName);
#ifndef VMS
    gethostname (hostname, 100);
    printf ("%s server on %s from %s\n",
	    ServerVendor (display), DisplayString (display), hostname);
#else
    printf ("%s server on %s\n",
	    ServerVendor (display), DisplayString (display));
#endif
    PrintTime ();

    root = RootWindow (display, 0);
    if (drawToGPX) {
        tileToQuery = XCreatePixmap(display, root, 32, 32, 1);
    }


    fgPixel = BlackPixel(display, 0);
    bgPixel = WhitePixel(display, 0);
    if (foreground != NULL) {
	/* Try to allocate a foreground color as specified */
	XColor def, cdef;

	if (XAllocNamedColor(display,
		XDefaultColormap(display, DefaultScreen(display)),
		foreground, &def, &cdef)) {
	    fgPixel = def.pixel;
	} else {
	    (void) fprintf(stderr, "Cannot allocate color %s\n", foreground);
	}
    }
    if (background != NULL) {
	/* Try to allocate a background color as specified */
	XColor def, cdef;
	if (XAllocNamedColor(display,
		XDefaultColormap(display, DefaultScreen(display)),
		background, &def, &cdef)) {
	    bgPixel = def.pixel;
	} else {
	    (void) fprintf(stderr, "Cannot allocate color %s\n", background);
	}
    }

    if (synchronous)
	XSynchronize (display, True);
    ForEachTest (i) {
	if (test[i].doit) {
	    int     child = 0;
	    char    label[100];
	    int     reps = test[i].parms.reps;
	    while (1) {
		if (test[i].children) {
		    test[i].parms.objects = subs[child];
		    if (test[i].parms.objects == 0)
			break;
		    if (reps > 10)
			test[i].parms.reps = reps/test[i].parms.objects;
		    sprintf (label, "%s (%d children)",
			    test[i].label, test[i].parms.objects);
		}
		else
		    strcpy (label, test[i].label);
		perfWindow = NULL;
		if (test[i].init != NullProc)
		    (*test[i].init) (display, &test[i].parms);
		for (j = 0; j < repeat; j++)
		    DoTest (display, &test[i], label);
		if (test[i].cleanup != NullProc)
		    (*test[i].cleanup) (display, &test[i].parms);
		printf ("\n");
		if (!test[i].children)
		    break;
		child++;
	    }
	}
    }
}

