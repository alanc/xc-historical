/*
 * xditview -- 
 *
 *   Display ditroff output in an X window
 */

#ifndef lint
static char rcsid[] = "$XConsortium: Exp $";
#endif  lint

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/VPaned.h>
#include <X11/Viewport.h>
#include <X11/Box.h>
#include <X11/Command.h>
#include <X11/Dialog.h>
#include <X11/Label.h>
#include "libXdvi/Dvi.h"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-page",	    "*dvi.pageNumber",	    XrmoptionSepArg,	"1"},
{"-backingStore",   "*dvi.backingStore",    XrmoptionSepArg,	"default"},
};

/*
 * Report the syntax for calling xditview.
 */
static
Syntax(call)
	char *call;
{
	(void) printf ("Usage: %s [-fg <color>] [-bg <color>]\n", call);
	(void) printf ("       [-bd <color>] [-bw <pixels>] [-help]\n");
	(void) printf ("       [-display displayname] [-geometry geom]\n");
	(void) printf ("       [-page <page-number>] [-backing <backing-store>]\n\n");
	exit(1);
}

static void	NewFile ();
static Widget	toplevel, vpaned, viewport, box, dvi;
static Widget	nextPage, prevPage, selectPage, open, quit, page;

static char	pageLabel[256] = "Page number: <none>";

void main(argc, argv)
    int argc;
    char **argv;
{
    Arg		    arg;
    Pixmap	    icon_pixmap = None;
    char	    *file_name = 0;
    static void	    commandCall();
    static Arg	    viewportArgs[] = {
			{XtNallowVert, True},
			{XtNallowHoriz, True},
			{XtNskipAdjust, False},
			{XtNwidth, 500},
			{XtNheight, 700},
    };
    static Arg	    boxArgs[] = {
			{XtNskipAdjust, True},
    };
    static XtCallbackRec commandCallback[] = {
			{ commandCall, (caddr_t) 0, },
			{ NULL, NULL }
    };
    static Arg	    commandArgs[] = {
			{XtNcallback, (int) commandCallback},
    };
    static Arg	    vPanedArgs[] = {
			{XtNallowResize, True},
			{XtNskipAdjust, True},
    };
    static Arg	    labelArgs[] = {
			{XtNlabel, (int) pageLabel},
    };
    int		    height;

    toplevel = XtInitialize("main", "XDvi",
			    options, XtNumber (options),
 			    &argc, argv);
    if (argc > 2)
	Syntax(argv[0]);
    if (argc > 1)
	file_name = argv[1];

    vpaned = XtCreateManagedWidget("vpaned", vPanedWidgetClass, toplevel,
				    vPanedArgs, XtNumber (vPanedArgs));
    XtPanedSetRefigureMode (vpaned, FALSE);
    box = XtCreateManagedWidget ("box", boxWidgetClass, vpaned,
				    boxArgs, XtNumber (boxArgs));
    nextPage = XtCreateManagedWidget ("Next Page", commandWidgetClass, box,
					commandArgs, XtNumber (commandArgs));
    prevPage = XtCreateManagedWidget ("Previous Page", commandWidgetClass, box,
					commandArgs, XtNumber (commandArgs));
    selectPage = XtCreateManagedWidget ("Select Page", commandWidgetClass, box,
					commandArgs, XtNumber (commandArgs));
    open = XtCreateManagedWidget ("Open", commandWidgetClass, box,
				        commandArgs, XtNumber (commandArgs));
    quit = XtCreateManagedWidget ("Quit", commandWidgetClass, box,
					commandArgs, XtNumber (commandArgs));
    page = XtCreateManagedWidget ("Page", labelWidgetClass, box,
					labelArgs, XtNumber (labelArgs));
    viewport = XtCreateManagedWidget("viewport", viewportWidgetClass, vpaned,
				     viewportArgs, XtNumber (viewportArgs));
    dvi = XtCreateManagedWidget ("dvi", dviWidgetClass, viewport, NULL, 0);

    XtPanedSetRefigureMode (vpaned, TRUE);
    if (file_name)
	NewFile (file_name);
    XtRealizeWidget (toplevel);
    XtMainLoop();
}

static void
SetPageNumber (number)
{
    Arg	arg[2];
    int	actual_number, last_page;

    XtSetArg (arg[0], XtNpageNumber, number);
    XtSetValues (dvi, arg, 1);
    XtSetArg (arg[0], XtNpageNumber, &actual_number);
    XtSetArg (arg[1], XtNlastPageNumber, &last_page);
    XtGetValues (dvi, arg, 2);
    if (last_page > 0)
	sprintf (pageLabel, "Page %d of %d", actual_number, last_page);
    else
	sprintf (pageLabel, "Page %d", actual_number);
    XtSetArg (arg[0], XtNlabel, pageLabel);
    XtSetValues (page, arg, 1);
}

static void
SelectPage (number_string)
char	*number_string;
{
	SetPageNumber (atoi(number_string));
}

static void
NewFile (name)
char	*name;
{
    Arg	    arg[2];
    char    *n, *rindex ();

    XtSetArg (arg[0], XtNfileName, name);
    XtSetValues (dvi, arg, 1);
    XtSetArg (arg[0], XtNtitle, name);
    if (n = rindex (name, '/'))
	name = n + 1;
    XtSetArg (arg[1], XtNiconName, name);
    XtSetValues (toplevel, arg, 2);
    SelectPage ("1");
}

static void
commandCall (cw, closure, data)
    Widget  cw;
    int	    closure, data;
{
    Arg	dviArg[1];
    int	number;
    if (cw == nextPage) {
	XtSetArg (dviArg[0], XtNpageNumber, &number);
	XtGetValues (dvi, dviArg, 1);
	SetPageNumber (number+1);
    } else if (cw == prevPage) {
	XtSetArg (dviArg[0], XtNpageNumber, &number);
	XtGetValues (dvi, dviArg, 1);
	SetPageNumber (number-1);
    } else if (cw == selectPage) {
	MakePrompt ("Page number", SelectPage);
    } else if (cw == open) {
	MakePrompt ("File to open:", NewFile);
    } else if (cw == quit) {
	exit (0);
    }
}


Widget	promptwidget;
void	(*promptfunction)();

/* ARGSUSED */
void DestroyPromptWidget(widget, client_data, call_data)
    Widget widget;		/* unused */
    caddr_t client_data;	/* scrn */
    caddr_t call_data;		/* unused */
{
    if (promptwidget) {
	XtSetKeyboardFocus(toplevel, vpaned);
	XtDestroyWidget(promptwidget);
	promptwidget = NULL;
    }
}


/* ARGSUSED */
void TellPrompt(widget, client_data, call_data)
    Widget widget;
    caddr_t client_data;	/* scrn */
    caddr_t call_data;

{
    (*promptfunction)(XtDialogGetValueString(promptwidget));
    DestroyPromptWidget(widget, client_data, call_data);
}

MakePrompt(prompt, func)
char *prompt;
void (*func)();
{
    static Arg args[] = {
	{XtNlabel, NULL},
	{XtNvalue, NULL},
    };
    args[0].value = (XtArgVal)prompt;
    args[1].value = (XtArgVal)"";
    DestroyPromptWidget((Widget)NULL, (caddr_t)0, NULL);
    promptwidget = XtCreateWidget( "prompt", dialogWidgetClass, vpaned,
				   args, (Cardinal)2 );
    XtDialogAddButton(promptwidget, "Accept", TellPrompt, (caddr_t)0);
    XtDialogAddButton(promptwidget, "Cancel", DestroyPromptWidget, (caddr_t)0);
    XtRealizeWidget(promptwidget);
    XtSetKeyboardFocus(promptwidget, XtNameToWidget(promptwidget,"value"));
    XtSetKeyboardFocus(toplevel, (Widget)None);
    XtMapWidget( promptwidget );
    promptfunction = func;
}
