/*
 * $XConsortium: xclipboard.c,v 1.19 90/11/07 09:53:38 dave Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
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
 * Author:  Ralph Swick, DEC/Project Athena
 * Updated for R4:  Chris D. Peterson,  MIT X Consortium.
 * Reauthored by: Keith Packard, MIT X Consortium.
 */

/* $XConsortium: xclipboard.c,v 1.19 90/11/07 09:53:38 dave Exp $ */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xfuncs.h>

#define Command commandWidgetClass
#define Label	labelWidgetClass
#define Text    asciiTextWidgetClass

#define INFINITY 10000000	/* pretty big, huh? */

typedef struct _Clip {
    struct _Clip    *next, *prev;
    char	    *clip;
    char	    *filename;
    int		    avail;
} ClipRec, *ClipPtr;

extern char *malloc ();

static long TextLength (w)
    Widget  w;
{
    return XawTextSourceScan (XawTextGetSource (w),
			      (XawTextPosition) 0,
 			      XawstAll, XawsdRight, 1, TRUE);
}

SaveClip (w, clip)
    Widget  w;
    ClipPtr clip;
{
    Arg	    args[1];
    char    *data;
    int	    len;
    Widget  source;

    source = XawTextGetSource (w);
    XtSetArg (args[0], XtNstring, &data);
    XtGetValues (source, args, 1);
    len = strlen (data);
    if (len >= clip->avail)
    {
	if (clip->clip)
	    free (clip->clip);
	clip->clip = malloc (len + 1);
	if (!clip->clip)
	    clip->avail = 0;
	else
	    clip->avail = len + 1;
    }
    if (clip->avail)
    {
	strcpy (clip->clip, data);
    }
}

RestoreClip (w, clip)
    Widget  w;
    ClipPtr clip;
{
    Arg	    args[1];
    Widget  source;

    source = XawTextGetSource (w);
    XtSetArg (args[0], XtNstring, clip->clip);
    XtSetValues (source, args, 1);
}

/*ARGSUSED*/
ClipPtr 
NewClip (w, old)
    Widget  w;
    ClipPtr old;
{
    ClipPtr newClip;

    newClip = (ClipPtr) malloc (sizeof (ClipRec));
    if (!newClip)
	return newClip;
    newClip->clip = 0;
    newClip->avail = 0;
    newClip->prev = old;
    newClip->next = NULL;
    newClip->filename = NULL;
    if (old)
    {
	newClip->next = old->next;
	old->next = newClip;
    }
    return newClip;
}

/*ARGSUSED*/
DeleteClip (w, clip)
    Widget  w;
    ClipPtr clip;
{
    if (clip->prev)
	clip->prev->next = clip->next;
    if (clip->next)
	clip->next->prev = clip->prev;
    if (clip->clip)
	free (clip->clip);
    free ((char *) clip);
}

static ClipPtr	currentClip;
static Widget	top;
static Widget	text, nextButton, prevButton, indexLabel;
static Widget	fileDialog, fileDialogShell;
static Widget	failDialog, failDialogShell;

static int
IndexCurrentClip ()
{
    int	i = 0;
    ClipPtr clip;

    for (clip = currentClip; clip; clip = clip->prev)
	i++;
    return i;
}

static void
set_button_state ()
{
    Boolean prevvalid, nextvalid;
    Arg arg;
    char labelString[10];

    prevvalid = currentClip->prev != NULL;
    nextvalid = currentClip->next != NULL;
    XtSetArg (arg, XtNsensitive, prevvalid);
    XtSetValues (prevButton, &arg, ONE);
    XtSetArg (arg, XtNsensitive, nextvalid);
    XtSetValues (nextButton, &arg, ONE);
    sprintf (labelString, "%d", IndexCurrentClip ());
    XtSetArg (arg, XtNlabel, labelString);
    XtSetValues (indexLabel, &arg, ONE);
}

static void
NextCurrentClip ()
{
    if (currentClip->next)
    {
	SaveClip (text, currentClip);
	currentClip = currentClip->next;
	RestoreClip (text, currentClip);
	set_button_state ();
    }
}

static void
PrevCurrentClip ()
{
    if (currentClip->prev)
    {
	SaveClip (text, currentClip);
	currentClip = currentClip->prev;
	RestoreClip (text, currentClip);
	set_button_state ();
    }
}

static void
DeleteCurrentClip ()
{
    ClipPtr newCurrent;

    if (currentClip->prev)
	newCurrent = currentClip->prev;
    else
	newCurrent = currentClip->next;
    if (newCurrent)
    {
	DeleteClip (text, currentClip);
	currentClip = newCurrent;
	RestoreClip (text, currentClip);
    }
    else
	EraseTextWidget ();
    set_button_state ();
}

static void
Quit ()
{
    XtCloseDisplay  (XtDisplay (text));
    exit (0);
}

static void
CenterWidgetAtPoint (w, x, y)
    Widget  w;
    int	    x, y;
{
    Arg	args[2];
    Dimension	width, height;

    XtSetArg(args[0], XtNwidth, &width);
    XtSetArg(args[1], XtNheight, &height);
    XtGetValues (w, args, 2);
    x = x - (int) width / 2;
    y = y - (int) height / 2;
    if (x < 0)
	x = 0;
    else {
	int scr_width = WidthOfScreen (XtScreen(w));
	if (x + width > scr_width)
	    x = scr_width - width;
    }
    if (y < 0)
	y = 0;
    else {
	int scr_height = HeightOfScreen (XtScreen(w));
	if (y + height > scr_height)
	    y = scr_height - height;
    }
    XtSetArg(args[0], XtNx, x);
    XtSetArg(args[1], XtNy, y);
    XtSetValues (w, args, 2);
}

static void
CenterWidgetOnEvent (w, e)
    Widget  w;
    XEvent  *e;
{
    CenterWidgetAtPoint (w, e->xbutton.x_root, e->xbutton.y_root);
}

static void
CenterWidgetOnWidget (w, wT)
    Widget  w, wT;
{
    Position	rootX, rootY;
    Dimension	width, height;
    Arg		args[2];

    XtSetArg (args[0], XtNwidth, &width);
    XtSetArg (args[1], XtNheight, &height);
    XtGetValues (wT, args, 2);
    XtTranslateCoords (wT, (Position) width/2, (Position) height/2, &rootX, &rootY);
    CenterWidgetAtPoint (w, (int) rootX, (int) rootY);
}

/*ARGSUSED*/
static void
SaveToFile (w, e, argv, argc)
    Widget  w;
    XEvent  *e;
    String  *argv;
    Cardinal	    *argc;
{
    Arg	    args[1];
    char    *filename;

    filename = "clipboard";
    if (currentClip->filename)
	filename = currentClip->filename;
    XtSetArg(args[0], XtNvalue, filename);
    XtSetValues (fileDialog, args, 1);
    CenterWidgetOnEvent (fileDialogShell, e);
    XtPopup (fileDialogShell, XtGrabNone);
}

/*ARGSUSED*/
static void
AcceptSaveFile (w, e, argv, argc)
    Widget  w;
    XEvent  *e;
    String  *argv;
    Cardinal	*argc;
{
    char    *filename;
    Boolean success;
    Arg	    args[1];

    filename = XawDialogGetValueString (fileDialog);
    success = XawAsciiSaveAsFile (XawTextGetSource (text), filename);
    XtPopdown (fileDialogShell);
    if (!success)
    {
	char	failMessage[1024];

	sprintf (failMessage, "Can't open file \"%s\"", filename);
	XtSetArg (args[0], XtNlabel, failMessage);
	XtSetValues (failDialog, args, 1);
	CenterWidgetOnEvent (failDialogShell, e);
	XtPopup (failDialogShell, XtGrabNone);
    }
    else
    {
	if (currentClip->filename)
	    free (currentClip->filename);
	currentClip->filename = malloc (strlen (filename) + 1);
	if (currentClip->filename)
	    strcpy (currentClip->filename, filename);
    }
}

static void
CancelSaveFile ()
{
    XtPopdown (fileDialogShell);
}

static void
FailContinue ()
{
    XtPopdown (failDialogShell);
}

static void
NewCurrentClip ()
{
    NewCurrentClipContents ("", 0);
}

NewCurrentClipContents (data, len)
    char    *data;
    int	    len;
{
    XawTextBlock textBlock;

    SaveClip (text, currentClip);

    /* append new clips at the end */
    while (currentClip && currentClip->next)
	currentClip = currentClip->next;
    /* any trailing clips with no text get overwritten */
    if (strlen (currentClip->clip) != 0)
	currentClip = NewClip (text, currentClip);
    
    textBlock.ptr = data;
    textBlock.firstPos = 0;
    textBlock.length = len;
    textBlock.format = FMT8BIT;
    if (XawTextReplace(text, 0, TextLength (text), &textBlock))
	XBell( XtDisplay(text), 0);
    set_button_state ();
}

EraseTextWidget ()
{
    XawTextBlock block;

    block.ptr = "";
    block.length = 0;
    block.firstPos = 0;
    block.format = FMT8BIT;

    XawTextReplace(text, 0, INFINITY, &block);
    /* If this fails, too bad. */
}


XtActionsRec xclipboard_actions[] = {
    "NewClip", NewCurrentClip,
    "NextClip",	NextCurrentClip,
    "PrevClip", PrevCurrentClip,
    "DeleteClip", DeleteCurrentClip,
    "Save", SaveToFile,
    "AcceptSave", AcceptSaveFile,
    "CancelSave", CancelSaveFile,
    "FailContinue", FailContinue,
    "Quit", Quit,
};

static Atom wm_delete_window;

static XrmOptionDescRec table[] = {
    {"-w",	    "wrap",		XrmoptionNoArg,  "on"},
/*    {"-nw",	    "wrap",		XrmoptionNoArg,  "False"} */
};

static void	LoseSelection ();
static void	InsertClipboard ();
static Boolean	ConvertSelection();
static Atom	ManagerAtom, ClipboardAtom;

/*ARGSUSED*/
static void 
InsertClipboard(w, client_data, selection, type, value, length, format)
Widget w;
caddr_t client_data;
Atom *selection, *type;
caddr_t value;
unsigned long *length;
int *format;
{
    if (*type != XT_CONVERT_FAIL)
	NewCurrentClipContents ((char *) value, *length);
    else
    {
	Arg arg;
	XtSetArg (arg, XtNlabel, "CLIPBOARD selection conversion failed");
	XtSetValues (failDialog, &arg, 1);
	CenterWidgetOnWidget (failDialogShell, text);
	XtPopup (failDialogShell, XtGrabNone);
	XBell( XtDisplay(w), 0 );
    }
    
    XtOwnSelection(top, ClipboardAtom, CurrentTime,
		   ConvertSelection, LoseSelection, NULL);

    XtFree(value);
}

static Boolean ConvertSelection(w, selection, target,
				type, value, length, format)
    Widget w;
    Atom *selection, *target, *type;
    caddr_t *value;
    unsigned long *length;
    int *format;
{
    Display* d = XtDisplay(w);
    XSelectionRequestEvent* req =
	XtGetSelectionRequest(w, *selection, (XtRequestId)NULL);

    if (*target == XA_TARGETS(d)) {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	XmuConvertStandardSelection(w, req->time, selection, target, type,
				  (caddr_t*)&std_targets, &std_length, format);
	*value = XtMalloc(sizeof(Atom)*(std_length + 5));
	targetP = *(Atom**)value;
	*targetP++ = XA_STRING;
	*targetP++ = XA_TEXT(d);
	*targetP++ = XA_LENGTH(d);
	*targetP++ = XA_LIST_LENGTH(d);
	*targetP++ = XA_CHARACTER_POSITION(d);
	*length = std_length + (targetP - (*(Atom **) value));
	bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = 32;
	return True;
    }

    if (*target == XA_LIST_LENGTH(d) ||
	*target == XA_LENGTH(d))
    {
    	long * temp;
    	
    	temp = (long *) XtMalloc(sizeof(long));
    	if (*target == XA_LIST_LENGTH(d))
      	  *temp = 1L;
    	else			/* *target == XA_LENGTH(d) */
      	  *temp = (long) TextLength (text);
    	
    	*value = (caddr_t) temp;
    	*type = XA_INTEGER;
    	*length = 1L;
    	*format = 32;
    	return True;
    }
    
    if (*target == XA_CHARACTER_POSITION(d))
    {
    	long * temp;
    	
    	temp = (long *) XtMalloc(2 * sizeof(long));
    	temp[0] = (long) 0;
    	temp[1] = TextLength (text);
    	*value = (caddr_t) temp;
    	*type = XA_SPAN(d);
    	*length = 2L;
    	*format = 32;
    	return True;
    }
    
    if (*target == XA_STRING ||
      *target == XA_TEXT(d) ||
      *target == XA_COMPOUND_TEXT(d))
    {
	extern char *_XawTextGetSTRING();
    	if (*target == XA_COMPOUND_TEXT(d))
	    *type = *target;
    	else
	    *type = XA_STRING;
	*length = TextLength (text);
    	*value = _XawTextGetSTRING((TextWidget) text, 0, *length);
    	*format = 8;
    	return True;
    }
    
    if (XmuConvertStandardSelection(w, req->time, selection, target, type,
				    value, length, format))
	return True;

    return False;
}

static void LoseSelection(w, selection)
    Widget w;
    Atom *selection;
{
    XtGetSelectionValue(w, *selection, XA_STRING, InsertClipboard,
			NULL, CurrentTime);
}

/*ARGSUSED*/
static Boolean RefuseSelection(w, selection, target,
			       type, value, length, format)
    Widget w;
    Atom *selection, *target, *type;
    caddr_t *value;
    unsigned long *length;
    int *format;
{
    return False;
}

/*ARGSUSED*/
static void LoseManager(w, selection)
    Widget w;
    Atom *selection;
{
    XtError("another clipboard has taken over control\n");
}

typedef struct {
  Boolean wrap;
} ResourceData, *ResourceDataPtr;

ResourceData userOptions;

XtResource resources[] = {
  {"wrap", "Wrap", XtRBoolean, sizeof(Boolean),
     XtOffset(ResourceDataPtr, wrap), XtRImmediate, (XtPointer)False}
};

void
main(argc, argv)
int argc;
char **argv;
{
    Arg args[4];
    Cardinal n;
    Widget parent, quit, delete, new, save;

    top = XtInitialize( "xclipboard", "XClipboard", table, XtNumber(table),
			 (Cardinal*) &argc, argv);

    XtGetApplicationResources(top, (XtPointer)&userOptions, resources, 
			      XtNumber(resources), NULL, 0);

    XtAddActions (xclipboard_actions, XtNumber (xclipboard_actions));
    /* CLIPBOARD_MANAGER is a non-standard mechanism */
    ManagerAtom = XInternAtom(XtDisplay(top), "CLIPBOARD_MANAGER", False);
    ClipboardAtom = XA_CLIPBOARD(XtDisplay(top));
    if (XGetSelectionOwner(XtDisplay(top), ManagerAtom))
	XtError("another clipboard is already running\n");
    XtOverrideTranslations
	(top, XtParseTranslationTable ("<Message>WM_PROTOCOLS: Quit()"));

    parent = XtCreateManagedWidget("form", formWidgetClass, top, NULL, ZERO);
    quit = XtCreateManagedWidget("quit", Command, parent, NULL, ZERO);
    delete = XtCreateManagedWidget("delete", Command, parent, NULL, ZERO);
    new = XtCreateManagedWidget("new", Command, parent, NULL, ZERO);
    save = XtCreateManagedWidget("save", Command, parent, NULL, ZERO);
    nextButton = XtCreateManagedWidget("next", Command, parent, NULL, ZERO);
    prevButton = XtCreateManagedWidget("prev", Command, parent, NULL, ZERO);
    indexLabel = XtCreateManagedWidget("index", Label, parent, NULL, ZERO);

    n=0;
    XtSetArg(args[n], XtNtype, XawAsciiString); n++;
    XtSetArg(args[n], XtNeditType, XawtextEdit); n++;
    if (userOptions.wrap) {
	XtSetArg(args[n], XtNwrap, XawtextWrapWord); n++;
	XtSetArg(args[n], XtNscrollHorizontal, False); n++;
    }

    text = XtCreateManagedWidget( "text", Text, parent, args, n);
    
    currentClip = NewClip (text, (ClipPtr) 0);

    set_button_state ();

    fileDialogShell = XtCreatePopupShell("fileDialogShell", transientShellWidgetClass,
					 top, NULL, ZERO);
    fileDialog = XtCreateManagedWidget ("fileDialog", dialogWidgetClass,
					fileDialogShell, NULL, ZERO);
    XawDialogAddButton(fileDialog, "accept", NULL, NULL);
    XawDialogAddButton(fileDialog, "cancel", NULL, NULL);

    XtRealizeWidget (fileDialogShell);

    failDialogShell = XtCreatePopupShell("failDialogShell", transientShellWidgetClass,
					 top, NULL, ZERO);
    failDialog = XtCreateManagedWidget ("failDialog", dialogWidgetClass,
					failDialogShell, NULL, ZERO);
    XawDialogAddButton (failDialog, "continue", NULL, NULL);

    XtRealizeWidget (failDialogShell);

    XtRealizeWidget(top);

    XtOwnSelection(top, ManagerAtom, CurrentTime,
		   RefuseSelection, LoseManager, NULL);
    if (XGetSelectionOwner (XtDisplay(top), ClipboardAtom))
    {
	LoseSelection (top, &ClipboardAtom);
    }
    else
    {
    	XtOwnSelection(top, ClipboardAtom, CurrentTime,
		       ConvertSelection, LoseSelection, NULL);
    }
    wm_delete_window = 
      XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW", False);
    (void) XSetWMProtocols (XtDisplay(top), XtWindow(top),
                            &wm_delete_window, 1);
    XtMainLoop();
}
