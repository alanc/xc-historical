/* $XConsortium: xditview.c,v 1.23 91/03/19 12:39:12 gildea Exp $ */
/*
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * xditview -- 
 *
 *   Display ditroff output in an X window
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <X11/Xos.h>		/* rindex declaration */
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Panner.h>
#include <X11/Xaw/Porthole.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>

#include "libXdvi/Dvi.h"

#include "xdit.bm"
#include "xdit_mask.bm"
#include <stdio.h>

#ifndef sgi			/* SGI declares popen() in stdio.h */
extern FILE *popen();
#endif
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
static Widget	toplevel, paned, form, panner, porthole, dvi;
static Widget	page;
static Widget	simpleMenu;

static void	NextPage(), PreviousPage(), SelectPage(), OpenFile(), Quit();

static struct menuEntry {
    char    *name;
    void    (*function)();
} menuEntries[] = {
    "nextPage",	    NextPage,
    "previousPage", PreviousPage,
    "selectPage",   SelectPage,
    "openFile",	    OpenFile,
    "quit",	    Quit,
};

static void	NextPageAction(), PreviousPageAction(), SelectPageAction();
static void	OpenFileAction(), QuitAction();
static void	AcceptAction(), CancelAction();

XtActionsRec xditview_actions[] = {
    "NextPage",	    NextPageAction,
    "PreviousPage", PreviousPageAction,
    "SelectPage",   SelectPageAction,
    "OpenFile",	    OpenFileAction,
    "Quit",	    QuitAction,
    "Accept",	    AcceptAction,
    "Cancel",	    CancelAction,
};

static Atom wm_delete_window;

/*	Function Name: PannerCallback
 *	Description: called when the panner has moved.
 *	Arguments: panner - the panner widget.
 *                 closure - *** NOT USED ***.
 *                 report_ptr - the panner record.
 *	Returns: none.
 */

/* ARGSUSED */
void 
PannerCallback(w, closure, report_ptr)
Widget w;
XtPointer closure, report_ptr;
{
    Arg args[2];
    XawPannerReport *report = (XawPannerReport *) report_ptr;

    if (!dvi)
	return;
    XtSetArg (args[0], XtNx, -report->slider_x);
    XtSetArg (args[1], XtNy, -report->slider_y);

    XtSetValues(dvi, args, 2);
}

/*	Function Name: PortholeCallback
 *	Description: called when the porthole or its child has
 *                   changed 
 *	Arguments: porthole - the porthole widget.
 *                 panner_ptr - the panner widget.
 *                 report_ptr - the porthole record.
 *	Returns: none.
 */

/* ARGSUSED */
void 
PortholeCallback(w, panner_ptr, report_ptr)
Widget w;
XtPointer panner_ptr, report_ptr;
{
    Arg args[10];
    Cardinal n = 0;
    XawPannerReport *report = (XawPannerReport *) report_ptr;
    Widget panner = (Widget) panner_ptr;

    XtSetArg (args[n], XtNsliderX, report->slider_x); n++;
    XtSetArg (args[n], XtNsliderY, report->slider_y); n++;
    if (report->changed != (XawPRSliderX | XawPRSliderY)) {
	XtSetArg (args[n], XtNsliderWidth, report->slider_width); n++;
	XtSetArg (args[n], XtNsliderHeight, report->slider_height); n++;
	XtSetArg (args[n], XtNcanvasWidth, report->canvas_width); n++;
	XtSetArg (args[n], XtNcanvasHeight, report->canvas_height); n++;
    }
    XtSetValues (panner, args, n);
}

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
    char	    *file_name = 0;
    int		    i;
    XtAppContext    xtcontext;
    static Arg	    labelArgs[] = {
			{XtNlabel, (XtArgVal) pageLabel},
		    };
    Arg		    topLevelArgs[2];
    Widget          entry;

    toplevel = XtAppInitialize(&xtcontext, "Xditview",
			       options, XtNumber (options),
			       &argc, argv, NULL, NULL, 0);
    if (argc > 2)
	Syntax(argv[0]);

    XtAppAddActions(xtcontext, xditview_actions, XtNumber (xditview_actions));
    XtOverrideTranslations
	(toplevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: Quit()"));

    XtSetArg (topLevelArgs[0], XtNiconPixmap,
	      XCreateBitmapFromData (XtDisplay (toplevel),
				     XtScreen(toplevel)->root,
				     (char *) xdit_bits,
				     xdit_width, xdit_height));
				    
    XtSetArg (topLevelArgs[1], XtNiconMask,
	      XCreateBitmapFromData (XtDisplay (toplevel),
				     XtScreen(toplevel)->root,
				     (char *) xdit_mask_bits, 
				     xdit_mask_width, xdit_mask_height));
    XtSetValues (toplevel, topLevelArgs, 2);
    if (argc > 1)
	file_name = argv[1];

    /*
     * create the menu and insert the entries
     */
    simpleMenu = XtCreatePopupShell ("menu", simpleMenuWidgetClass, toplevel,
				    NULL, 0);
    for (i = 0; i < XtNumber (menuEntries); i++) {
	entry = XtCreateManagedWidget(menuEntries[i].name, 
				      smeBSBObjectClass, simpleMenu,
				      NULL, (Cardinal) 0);
	XtAddCallback(entry, XtNcallback, menuEntries[i].function, NULL);
    }

    paned = XtCreateManagedWidget("paned", panedWidgetClass, toplevel,
				    NULL, (Cardinal) 0);
    page = XtCreateManagedWidget ("label", labelWidgetClass, paned,
					labelArgs, XtNumber (labelArgs));
    form = XtCreateManagedWidget ("form", formWidgetClass, paned,
				    NULL, (Cardinal) 0);
    panner = XtCreateManagedWidget ("panner", pannerWidgetClass,
				    form, NULL, 0);
    porthole = XtCreateManagedWidget ("porthole", portholeWidgetClass,
				      form, NULL, 0);
    XtAddCallback(porthole, 
		  XtNreportCallback, PortholeCallback, (XtPointer) panner);
    XtAddCallback(panner, 
		  XtNreportCallback, PannerCallback, (XtPointer) porthole);

    dvi = XtCreateManagedWidget ("dvi", dviWidgetClass, porthole, NULL, 0);
    if (file_name)
	NewFile (file_name);
    XtRealizeWidget (toplevel);
    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                            &wm_delete_window, 1);
    XtAppMainLoop(xtcontext);
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
SelectPageNumber (number_string)
char	*number_string;
{
	SetPageNumber (atoi(number_string));
}

static void
NewFile (name)
char	*name;
{
    Arg	    arg[2];
    char    *n;
    FILE    *new_file;
    Boolean seek = 0;

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
    SelectPageNumber ("1");
    strcpy (current_file_name, name);
    current_file = new_file;
}

static char fileBuf[1024];

ResetMenuEntry (entry)
    Widget  entry;
{
    Arg	arg[1];

    XtSetArg (arg[0], XtNpopupOnEntry, entry);
    XtSetValues (XtParent(entry) , arg, (Cardinal) 1);
}

static void
NextPage (entry, name, data)
    Widget  entry;
    caddr_t name, data;
{
    NextPageAction();
    ResetMenuEntry (entry);
}

static void
NextPageAction ()
{
    Arg	args[1];
    int	number;

    XtSetArg (args[0], XtNpageNumber, &number);
    XtGetValues (dvi, args, 1);
    SetPageNumber (number+1);
}

static void
PreviousPage (entry, name, data)
    Widget  entry;
    caddr_t name, data;
{
    PreviousPageAction ();
    ResetMenuEntry (entry);
}

static void
PreviousPageAction ()
{
    Arg	args[1];
    int	number;

    XtSetArg (args[0], XtNpageNumber, &number);
    XtGetValues (dvi, args, 1);
    SetPageNumber (number-1);
}

static void
SelectPage (entry, name, data)
    Widget  entry;
    caddr_t name, data;
{
    SelectPageAction ();
    ResetMenuEntry (entry);
}

static void
SelectPageAction ()
{
    MakePrompt (toplevel, "Page number", SelectPageNumber, "");
}

static void
OpenFile (entry, name, data)
    Widget  entry;
    caddr_t name, data;
{
    OpenFileAction ();
    ResetMenuEntry (entry);
}

static void
OpenFileAction ()
{
    if (current_file_name[0])
	strcpy (fileBuf, current_file_name);
    else
	fileBuf[0] = '\0';
    MakePrompt (toplevel, "File to open:", NewFile, fileBuf);
}

static void
Quit (entry, closure, data)
    Widget  entry;
    caddr_t closure, data;
{
    QuitAction ();
}

static void
QuitAction ()
{
    exit (0);
}

Widget	promptShell, promptDialog;
void	(*promptfunction)();

/* ARGSUSED */
static
void CancelAction (widget, event, params, num_params)
    Widget	widget;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    if (promptShell) {
	XtSetKeyboardFocus(toplevel, (Widget) None);
	XtDestroyWidget(promptShell);
	promptShell = (Widget) 0;
    }
}


/* ARGSUSED */
static
void AcceptAction (widget, event, params, num_params)
    Widget	widget;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    (*promptfunction)(XawDialogGetValueString(promptDialog));
    CancelAction (widget, event, params, num_params);
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
    Arg centerArgs[2];
    Position	source_x, source_y;
    Position	dest_x, dest_y;
    Dimension center_width, center_height;
    Dimension prompt_width, prompt_height;
    Widget  valueWidget;
    
    CancelAction ((Widget)NULL, (XEvent *) 0, (String *) 0, (Cardinal *) 0);
    promptShell = XtCreatePopupShell ("promptShell", transientShellWidgetClass,
				      toplevel, NULL, (Cardinal) 0);
    dialogArgs[0].value = (XtArgVal)prompt;
    dialogArgs[1].value = (XtArgVal)def;
    promptDialog = XtCreateManagedWidget( "promptDialog", dialogWidgetClass,
		    promptShell, dialogArgs, XtNumber (dialogArgs));
    XawDialogAddButton(promptDialog, "accept", NULL, (caddr_t) 0);
    XawDialogAddButton(promptDialog, "cancel", NULL, (caddr_t) 0);
    valueWidget = XtNameToWidget (promptDialog, "value");
    if (valueWidget) {
    	XtSetArg (valueArgs[0], XtNresizable, TRUE);
    	XtSetValues (valueWidget, valueArgs, 1);
	/*
	 * as resizable isn't set until just above, the
	 * default value will be displayed incorrectly.
	 * rectify the situation by resetting the values
	 */
        XtSetValues (promptDialog, dialogArgs, XtNumber (dialogArgs));
    }
    XtSetKeyboardFocus (promptDialog, valueWidget);
    XtSetKeyboardFocus (toplevel, valueWidget);
    XtRealizeWidget (promptShell);
    /*
     * place the widget in the center of the "parent"
     */
    XtSetArg (centerArgs[0], XtNwidth, &center_width);
    XtSetArg (centerArgs[1], XtNheight, &center_height);
    XtGetValues (centerw, centerArgs, 2);
    XtSetArg (centerArgs[0], XtNwidth, &prompt_width);
    XtSetArg (centerArgs[1], XtNheight, &prompt_height);
    XtGetValues (promptShell, centerArgs, 2);
    source_x = (int)(center_width - prompt_width) / 2;
    source_y = (int)(center_height - prompt_height) / 3;
    XtTranslateCoords (centerw, source_x, source_y, &dest_x, &dest_y);
    XtSetArg (centerArgs[0], XtNx, dest_x);
    XtSetArg (centerArgs[1], XtNy, dest_y);
    XtSetValues (promptShell, centerArgs, 2);
    XtMapWidget(promptShell);
    promptfunction = func;
}
