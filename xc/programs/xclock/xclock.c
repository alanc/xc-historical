/* xclock -- 
 *  Hacked from Tony Della Fera's much hacked clock program.
 */
#ifndef lint
static char rcsid[] = "$Header: xclock.c,v 1.7 87/12/08 08:18:34 swick Locked $";
#endif  lint

#include <X/Xatom.h>
#include <X/Xlib.h>
#include <X/Intrinsic.h>
#include <X/Atoms.h>
#include <X/Shell.h>
#include <X/Clock.h>
#include <X/Cardinals.h>
#include "clock.bit"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-chime",	"*clock.chime",		XrmoptionNoArg,		"TRUE"},
{"-hd",		"*clock.hand",		XrmoptionSepArg,	NULL},
{"-hands",	"*clock.hand",		XrmoptionSepArg,	NULL},
{"-hl",		"*clock.high",		XrmoptionSepArg,	NULL},
{"-highlight",	"*clock.high",		XrmoptionSepArg,	NULL},
{"-update",	"*clock.update",	XrmoptionSepArg,	NULL},
{"-padding",	"*clock.padding",	XrmoptionSepArg,	NULL},
{"-d",		"*clock.analog",	XrmoptionNoArg,		"FALSE"},
{"-digital",	"*clock.analog",	XrmoptionNoArg,		"FALSE"},
{"-analog",	"*clock.analog",	XrmoptionNoArg,		"TRUE"},
};


/*
 * Report the syntax for calling xclock.
 */
Syntax(call)
	char *call;
{
	(void) printf ("Usage: %s [-analog] [-bw <pixels>] [-digital]\n", call);
	(void) printf ("       [-fg <color>] [-bg <color>] [-hl <color>] [-bd <color>]\n");
	(void) printf ("       [-fn <font_name>] [-help] [-padding <pixels>]\n");
	(void) printf ("       [-rv] [-update <seconds>] [-d [<host>]:[<vs>]]\n");
	(void) printf ("       [-g =[<width>][x<height>][<+-><xoff>[<+-><yoff>]]]\n\n");
	exit(0);
}

void main(argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel;
    Arg arg;

    toplevel = XtInitialize(NULL, "XClock", options, XtNumber(options), &argc, argv);
    if (argc != 1) Syntax(argv[0]);

    arg.name = XtNiconPixmap;
    arg.value = (XtArgVal) XCreateBitmapFromData (XtDisplay(toplevel),
         XtScreen(toplevel)->root, clock_bits, clock_width, clock_height);
    XtSetValues (toplevel, &arg, ONE); 

    XtCreateManagedWidget ("clock", clockWidgetClass, toplevel, NULL, ZERO);
    XtRealizeWidget (toplevel);
    XtMainLoop();
}
