#ifndef lint
static char rcsid[] = "$XConsortium: oclock.c,v 1.9 90/04/30 13:52:09 keith Exp $";
#endif /* lint */

#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include "Clock.h"
#include <stdio.h> 

#include "oclock.bit"
#include "oclmask.bit"

extern void exit();
static void quit();

static XtActionsRec actions[] = {
    {"quit",	quit}
};

static Atom wm_delete_window;

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

/* Exit with message describing command line format */

void usage()
{
    fprintf(stderr,
"usage: oclock\n");
    fprintf (stderr, 
"       [-geometry [{width}][x{height}][{+-}{xoff}[{+-}{yoff}]]] [-display [{host}]:[{vs}]]\n");
    fprintf(stderr,
"       [-fg {color}] [-bg {color}] [-bd {color}] [-bw {pixels}]\n");
    fprintf(stderr,
"       [-minute {color}] [-hour {color}] [-jewel {color}]\n");
    fprintf(stderr,
"       [-backing {backing-store}]\n");
    exit(1);
}

static XrmOptionDescRec options[] = {
{"-fg",		"*Foreground",		XrmoptionSepArg,	NULL},
{"-bg",		"*Background",		XrmoptionSepArg,	NULL},
{"-foreground",	"*Foreground",		XrmoptionSepArg,	NULL},
{"-background",	"*Background",		XrmoptionSepArg,	NULL},
{"-minute",	"*clock.minute",	XrmoptionSepArg,	NULL},
{"-hour",	"*clock.hour",		XrmoptionSepArg,	NULL},
{"-jewel",	"*clock.jewel",		XrmoptionSepArg,	NULL},
{"-backing",	"*clock.backingStore",	XrmoptionSepArg,	NULL},
#ifdef SHAPE
{"-shape",	"*clock.shapeWindow",	XrmoptionNoArg,		"TRUE"},
{"-noshape",	"*clock.shapeWindow",	XrmoptionNoArg,		"FALSE"},
{"-transparent","*clock.transparent",	XrmoptionNoArg,		"TRUE"},
#endif
};

void main(argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel;
    Widget clock;
    Arg arg[2];
    int	i;
    
    toplevel = XtInitialize("main", "Clock", options, XtNumber (options),
				    (Cardinal *) &argc, argv);

    if (argc != 1) usage();

    XtAppAddActions
	(XtWidgetToApplicationContext(toplevel), actions, XtNumber(actions));
    XtOverrideTranslations
	(toplevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));

    i = 0;
    XtSetArg (arg[i], XtNiconPixmap, 
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     oclock_bits, oclock_width, oclock_height));
    i++;
    XtSetArg (arg[i], XtNiconMask,
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     oclmask_bits, oclmask_width, oclmask_height));
    i++;
    XtSetValues (toplevel, arg, i);

    clock = XtCreateManagedWidget ("clock", clockWidgetClass, toplevel, NULL, 0);
    XtRealizeWidget (toplevel);

    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                            &wm_delete_window, 1);

    XtMainLoop();
}

static void quit(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (event->type == ClientMessage && 
	event->xclient.data.l[0] != wm_delete_window) {
	XBell(XtDisplay(w), 0);
    } else {
	XCloseDisplay(XtDisplay(w));
	exit(0);
    }
}
