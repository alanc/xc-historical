/*
 * xditview -- 
 *
 *   Display ditroff output in an X window
 */

#ifndef lint
static char rcsid[] = "$XConsortium: xditview.c,v 1.1 89/03/01 15:47:44 keith Exp $";
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
#include "xditview.bm"
#include "xditview_mask.bm"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-page",	    "*dvi.pageNumber",	    XrmoptionSepArg,	NULL},
{"-backingStore",   "*dvi.backingStore",    XrmoptionSepArg,	NULL},
{"-noPolyText",	    "*dvi.noPolyText",	    XrmoptionNoArg,	"TRUE"},
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
    Arg		    topLevelArgs[2];
    int		    height;

    toplevel = XtInitialize("main", "XDvi",
			    options, XtNumber (options),
 			    &argc, argv);
    if (argc > 2)
	Syntax(argv[0]);
    XtSetArg (topLevelArgs[0], XtNiconPixmap,
	      XCreateBitmapFromData (XtDisplay (toplevel),
				     XtScreen(toplevel)->root,
				     xditview_bits, xditview_width, xditview_height));
				    
    XtSetArg (topLevelArgs[1], XtNiconMask,
	      XCreateBitmapFromData (XtDisplay (toplevel),
				     XtScreen(toplevel)->root,
				     xditview_mask_bits, xditview_mask_width, xditview_mask_height));
    XtSetValues (toplevel, topLevelArgs, 2);
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
	MakePrompt (cw, "Page number", SelectPage);
    } else if (cw == open) {
	MakePrompt (cw, "File to open:", NewFile);
    } else if (cw == quit) {
	exit (0);
    }
}


Widget	promptShell, promptDialog;
void	(*promptfunction)();

/* ARGSUSED */
void DestroyPromptWidget(widget, client_data, call_data)
    Widget widget;		/* unused */
    caddr_t client_data;	/* scrn */
    caddr_t call_data;		/* unused */
{
    if (promptShell) {
	XtSetKeyboardFocus(toplevel, (Widget) None);
	XtDestroyWidget(promptShell);
	promptShell = (Widget) 0;
    }
}


/* ARGSUSED */
void TellPrompt(widget, client_data, call_data)
    Widget widget;
    caddr_t client_data;	/* scrn */
    caddr_t call_data;

{
    (*promptfunction)(XtDialogGetValueString(promptDialog));
    DestroyPromptWidget(widget, client_data, call_data);
}

MakePrompt(cw, prompt, func)
Widget	cw;
char *prompt;
void (*func)();
{
    static Boolean true = TRUE;
    static Arg dialogArgs[] = {
	{XtNlabel, NULL},
	{XtNvalue, NULL},
    };
    Arg valueArgs[1];
    static Arg shellArgs[] = {
	{XtNx, 0},
	{XtNy, 0},
	{XtNallowShellResize, TRUE},
    };
    XWindowAttributes	xwa;
    int	dest_x, dest_y, child_return;
    Widget  valueWidget;
    
    XGetWindowAttributes (XtDisplay (cw), XtWindow (cw), &xwa);
    XTranslateCoordinates (XtDisplay (cw), XtWindow (cw), xwa.root,
			    0, xwa.height * 2,
			    &dest_x, &dest_y, &child_return);
    shellArgs[0].value = dest_x;
    shellArgs[1].value = dest_y;
    DestroyPromptWidget((Widget)NULL, (caddr_t)0, NULL);
    promptShell = XtCreatePopupShell ("promptShell", transientShellWidgetClass,
		    toplevel, shellArgs, XtNumber (shellArgs));
    dialogArgs[0].value = (XtArgVal)prompt;
    dialogArgs[1].value = (XtArgVal)"";
    promptDialog = XtCreateManagedWidget( "promptDialog", dialogWidgetClass,
		    promptShell, dialogArgs, XtNumber (dialogArgs));
    XtDialogAddButton(promptDialog, "Accept", TellPrompt, (caddr_t)0);
    XtDialogAddButton(promptDialog, "Cancel", DestroyPromptWidget, (caddr_t)0);
    XtSetKeyboardFocus(toplevel, promptShell);
    XtSetKeyboardFocus(promptShell, promptDialog);
    valueWidget = XtNameToWidget (promptDialog, "value");
    XtSetArg (valueArgs[0], XtNresizable, TRUE);
    XtSetValues (valueWidget, valueArgs, 1);
    XtSetKeyboardFocus(promptDialog, valueWidget);
    XtRealizeWidget(promptShell);
    XtMapWidget(promptShell);
    promptfunction = func;
}
