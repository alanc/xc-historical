/*
 * xditview -- 
 *
 *   Display ditroff output in an X window
 */

#ifndef lint
static char rcsid[] = "$XConsortium: xditview.c,v 1.6 89/04/13 13:34:02 keith Exp $";
#endif  lint

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Paned.h>
#include <X11/Viewport.h>
#include <X11/Box.h>
#include <X11/Command.h>
#include <X11/Dialog.h>
#include <X11/Label.h>
#include "libXdvi/Dvi.h"
#include "libXdvi/Menu.h"
#include "xdit.bm"
#include "xdit_mask.bm"
#include "stdio.h"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-page",	    "*dvi.pageNumber",	    XrmoptionSepArg,	NULL},
{"-backingStore",   "*dvi.backingStore",    XrmoptionSepArg,	NULL},
{"-noPolyText",	    "*dvi.noPolyText",	    XrmoptionNoArg,	"TRUE"},
};

static char	current_file_name[1024];
static FILE	*current_file;

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
static Widget	toplevel, paned, viewport, dvi;
static Widget	page;

static char menuString[] = "\
Next Page\n\
Previous Page\n\
Select Page\n\
Open File\n\
Quit\
";

static Widget	menu;

#define MenuNextPage		0
#define MenuPreviousPage	1
#define MenuSelectPage		2
#define MenuOpenFile		3
#define	MenuQuit		4

static char	pageLabel[256] = "Page <none>";

void main(argc, argv)
    int argc;
    char **argv;
{
    Arg		    arg;
    Pixmap	    icon_pixmap = None;
    char	    *file_name = 0;
    static void	    commandCall(), menuCall();
    static Arg	    viewportArgs[] = {
			{XtNallowVert, True},
			{XtNallowHoriz, True},
			{XtNskipAdjust, False},
			{XtNwidth, 600},    /* this one isn't right */
			{XtNheight, 800},   /* neither is this */
			{XtNshowGrip, False},
    };
    static Arg	    panedArgs[] = {
			{XtNallowResize, True},
    };
    static Arg	    labelArgs[] = {
			{XtNlabel, (int) pageLabel},
			{XtNskipAdjust, True},
    };
    static XtCallbackRec menuCallback[] = {
			{ menuCall, (caddr_t) 0 },
			{ NULL, NULL }
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

    menu = XawMenuCreate ("menu", menuWidgetClass, toplevel, menuString,
			    "<Btn1Down>", menuCallback);
    paned = XtCreateManagedWidget("paned", panedWidgetClass, toplevel,
				    panedArgs, XtNumber (panedArgs));
    viewport = XtCreateManagedWidget("viewport", viewportWidgetClass, paned,
				     viewportArgs, XtNumber (viewportArgs));
    dvi = XtCreateManagedWidget ("dvi", dviWidgetClass, viewport, NULL, 0);
    page = XtCreateManagedWidget ("Page", labelWidgetClass, paned,
					labelArgs, XtNumber (labelArgs));
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
    if (actual_number == 0)
	sprintf (pageLabel, "Page <none>");
    else if (last_page > 0)
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
    FILE    *new_file;
    Boolean seek = 0;
    int	    c;

    if (current_file) {
	if (!strcmp (current_file_name, "-"))
	    ;
	else if (current_file_name[0] == '|')
	    pclose (current_file);
	else
	    fclose (current_file);
    }
    if (!strcmp (name, "-"))
	new_file = stdin;
    else if (name[0] == '|')
	new_file = popen (name+1, "r");
    else {
	new_file = fopen (name, "r");
	seek = 1;
    }
    if (!new_file) {
	/* XXX display error message */
	return;
    }
    XtSetArg (arg[0], XtNfile, new_file);
    XtSetArg (arg[1], XtNseek, seek);
    XtSetValues (dvi, arg, 2);
    XtSetArg (arg[0], XtNtitle, name);
    if (name[0] != '/' && (n = rindex (name, '/')))
	n = n + 1;
    else
	n = name;
    XtSetArg (arg[1], XtNiconName, n);
    XtSetValues (toplevel, arg, 2);
    SelectPage ("1");
    strcpy (current_file_name, name);
    current_file = new_file;
}

static fileBuf[1024];

static void
menuCall (mw, closure, data)
    Widget  mw;
    caddr_t closure, data;
{
    int	    menuItem = (int) data;
    Arg	    args[1];
    int	    number;
    int	    resetSelection = 0;

    switch (menuItem) {
    case MenuNextPage:
	XtSetArg (args[0], XtNpageNumber, &number);
	XtGetValues (dvi, args, 1);
	SetPageNumber (number+1);
	resetSelection = 1;
	break;
    case MenuPreviousPage:
	XtSetArg (args[0], XtNpageNumber, &number);
	XtGetValues (dvi, args, 1);
	SetPageNumber (number-1);
	resetSelection = 1;
	break;
    case MenuSelectPage:
	MakePrompt (toplevel, "Page number", SelectPage, "");
	break;
    case MenuOpenFile:
	if (current_file_name[0])
	    strcpy (fileBuf, current_file_name);
	else
	    fileBuf[0] = '\0';
	MakePrompt (toplevel, "File to open:", NewFile, fileBuf);
	resetSelection = 1;
	break;
    case MenuQuit:
	exit (0);
    }
    XtSetArg (args[0], XtNselection, (int) menuItem);
    XtSetValues (mw, args, 1);
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

MakePrompt(centerw, prompt, func, def)
Widget	centerw;
char *prompt;
void (*func)();
char	*def;
{
    static Boolean true = TRUE;
    static Arg dialogArgs[] = {
	{XtNlabel, NULL},
	{XtNvalue, NULL},
    };
    Arg valueArgs[1];
    static Arg shellArgs[] = {
	{XtNallowShellResize, TRUE},
    };
    Arg centerArgs[2];
    XWindowAttributes	xwa;
    int	source_x, source_y;
    int	dest_x, dest_y, child_return;
    Dimension center_width, center_height;
    Dimension prompt_width, prompt_height;
    Widget  valueWidget;
    
    DestroyPromptWidget((Widget)NULL, (caddr_t)0, NULL);
    promptShell = XtCreatePopupShell ("promptShell", transientShellWidgetClass,
		    toplevel, shellArgs, XtNumber (shellArgs));
    dialogArgs[0].value = (XtArgVal)prompt;
    dialogArgs[1].value = (XtArgVal)def;
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
    XtSetArg (centerArgs[0], XtNwidth, &center_width);
    XtSetArg (centerArgs[1], XtNheight, &center_height);
    XtGetValues (centerw, centerArgs, 2);
    XtSetArg (centerArgs[0], XtNwidth, &prompt_width);
    XtSetArg (centerArgs[1], XtNheight, &prompt_height);
    XtGetValues (promptShell, centerArgs, 2);
    source_x = (center_width - prompt_width) / 2;
    source_y = (center_height - prompt_height) / 3;
    XTranslateCoordinates (XtDisplay (centerw),
			    XtWindow (centerw),
			    RootWindowOfScreen (XtScreen (centerw)),
			    source_x, source_y,
			    &dest_x, &dest_y, &child_return);
    XtSetArg (centerArgs[0], XtNx, dest_x);
    XtSetArg (centerArgs[1], XtNy, dest_y);
    XtSetValues (promptShell, centerArgs, 2);
    XtMapWidget(promptShell);
    promptfunction = func;
}
