/*
 * $XConsortium: xclipboard.c,v 1.9 89/12/08 16:09:37 kit Exp $
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
 */

/* $XConsortium: xclipboard.c,v 1.9 89/12/08 16:09:37 kit Exp $ */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Cardinals.h>

#define Command commandWidgetClass
#define Text    asciiTextWidgetClass

#define INFINITY 10000000	/* pretty big, huh? */

static XrmOptionDescRec table[] = {
    {"-w",	    "*text*wrap",		XrmoptionNoArg,  "Word"},
    {"-nw",	    "*text*wrap",		XrmoptionNoArg,  "Never"},
};

static void 
InsertClipboard(w, client_data, selection, type, value, length, format)
Widget w;
caddr_t client_data;
Atom *selection, *type;
caddr_t value;
unsigned long *length;
int *format;
{
    XawTextBlock text;
    Arg args[1];
    XawTextPosition last;

    if (*type == 0 /*XT_CONVERT_FAIL*/ || *length == 0) {
	XBell( XtDisplay(w), 0 );
	return;
    }

    XawTextSetInsertionPoint(w, INFINITY);
    last = XawTextGetInsertionPoint(w);

    text.ptr = (char*)value;
    text.firstPos = 0;
    text.length = *length;
    text.format = FMT8BIT;

    if (XawTextReplace(w, last, last, &text))
	XBell( XtDisplay(w), 0);
    else {
	XawTextPosition newend;
	XawTextSetInsertionPoint(w, last + text.length);
	newend = XawTextGetInsertionPoint(w);
	if (text.ptr[text.length-1] != '\n') {
	    text.ptr = "\n";
	    text.length = 1;
	    XawTextReplace(w, newend, newend, &text);
	    XawTextSetInsertionPoint(w, newend += 1);
	}
    }
    
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
	*value = XtMalloc(sizeof(Atom)*(std_length /* + 5 */));
	targetP = *(Atom**)value;
	*length = std_length /* + 5 */;
/*
	*targetP++ = XA_STRING;
	*targetP++ = XA_TEXT(d);
	*targetP++ = XA_LENGTH(d);
	*targetP++ = XA_LIST_LENGTH(d);
	*targetP++ = XA_CHARACTER_POSITION(d);
*/
	bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = 32;
	return True;
    }

    if (XmuConvertStandardSelection(w, req->time, selection, target, type,
				    value, length, format))
	return True;

    /* else */
    return False;
}


static void LoseSelection(w, selection)
    Widget w;
    Atom *selection;
{
    XtGetSelectionValue(w, *selection, XA_STRING, InsertClipboard,
			NULL, CurrentTime);

    XtOwnSelection(w, XA_CLIPBOARD(XtDisplay(w)), CurrentTime,
		   ConvertSelection, LoseSelection, NULL);
}

static void 
Erase(w, client_data, call_data)
Widget w;
caddr_t client_data, call_data;
{
    Widget text = (Widget) client_data;
    XawTextBlock block;

    block.ptr = NULL;
    block.length = 0;
    block.firstPos = 0;
    block.format = FMT8BIT;

    XawTextReplace(text, 0, INFINITY, &block);
    /* If this fails, too bad. */
}

static void 
Quit(w, client_data, call_data)
Widget w;
caddr_t client_data, call_data;
{
    XtCloseDisplay( XtDisplay(w) );
    exit( 0 );
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

void
main(argc, argv)
int argc;
char **argv;
{
    Arg args[2];
    Widget top, parent, quit, save, erase, text;
    Atom manager;

    top = XtInitialize( "xclipboard", "XClipboard", table, XtNumber(table),
			  &argc, argv);

    /* CLIPBOARD_MANAGER is a non-standard mechanism */
    manager = XInternAtom(XtDisplay(top), "CLIPBOARD_MANAGER", False);
    if (XGetSelectionOwner(XtDisplay(top), manager))
	XtError("another clipboard is already running\n");

    parent = XtCreateManagedWidget("form", formWidgetClass, top, NULL, ZERO);
    quit = XtCreateManagedWidget("quit", Command, parent, NULL, ZERO);
    erase = XtCreateManagedWidget("erase", Command, parent, NULL, ZERO);

    XtSetArg(args[0], XtNtype, XawAsciiFile);
    XtSetArg(args[1], XtNeditType, XawtextEdit);
    text = XtCreateManagedWidget( "text", Text, parent, args, TWO);

    XtAddCallback(quit, XtNcallback, Quit, NULL);
    XtAddCallback(erase, XtNcallback, Erase, text);

    XtRealizeWidget(top);

    XtOwnSelection(text, manager, CurrentTime,
		   RefuseSelection, LoseManager, NULL);
    XtOwnSelection(text, XA_CLIPBOARD(XtDisplay(text)), CurrentTime,
		   ConvertSelection, LoseSelection, NULL);

    XtMainLoop();
}
