/*
 * $XConsortium$
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
 */

/* $XConsortium: xclipboard.c,v 1.3 89/05/11 18:09:22 kit Exp $ */

#include <stdio.h>
#include <sys/param.h>

#ifdef UTEK
#undef dirty			/* Brain dead standard include files... */
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Cardinals.h>

static XrmOptionDescRec table[] = {
    {"-w",	"wordWrap",		XrmoptionNoArg,  "on"},
    {"-nw",	"wordWrap",		XrmoptionNoArg,  "off"},
};

typedef struct {
    Boolean word_wrap;
} app_resourceRec, *app_res;

app_resourceRec app_resources;

static XtResource resources[] = {
    {"wordWrap", "WordWrap", XtRBoolean, sizeof(Boolean),
	XtOffset(app_res,word_wrap), XtRImmediate, (caddr_t)False},
};


static void InsertClipboard(w, client_data, selection, type,
			    value, length, format)
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

#ifdef notdef
    XtSetArg( args[0], XtNlength, &end );
    XtGetValues( w, args, ONE );
#else
    XawTextSetInsertionPoint(w, 9999999);
    last = XawTextGetInsertionPoint(w);
#endif /*notdef*/

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
    TextWidget ctx = (TextWidget)w;

    if (*target == XA_TARGETS(d)) {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	XmuConvertStandardSelection(w, CurrentTime, selection, target, type,
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

#ifdef notdef
    if (*target == XA_STRING || *target == XA_TEXT(d)) {
	*type = XA_STRING;
	*value = _XawTextGetText(ctx, ctx->text.s.left, ctx->text.s.right);
	*length = strlen(*value);
	*format = 8;
	return True;
    }
    if (*target == XA_LIST_LENGTH(d)) {
	*value = XtMalloc(4);
	if (sizeof(long) == 4)
	    *(long*)*value = 1;
	else {
	    long temp = 1;
	    bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    }
    if (*target == XA_LENGTH(d)) {
	*value = XtMalloc(4);
	if (sizeof(long) == 4)
	    *(long*)*value = ctx->text.s.right - ctx->text.s.left;
	else {
	    long temp = ctx->text.s.right - ctx->text.s.left;
	    bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    }
    if (*target == XA_CHARACTER_POSITION(d)) {
	*value = XtMalloc(8);
	(*(long**)value)[0] = ctx->text.s.left + 1;
	(*(long**)value)[1] = ctx->text.s.right;
	*type = XA_SPAN(d);
	*length = 2;
	*format = 32;
	return True;
    }
#endif /*notdef*/

    if (XmuConvertStandardSelection(w, CurrentTime, selection, target, type,
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


static void Quit(w, client_data, call_data)
    Widget w;
    caddr_t client_data;
    caddr_t call_data;
{
    XtCloseDisplay( XtDisplay(w) );
    exit( 0 );
}


main(argc, argv)
unsigned int argc;
char **argv;
{
    static Arg textArgs[] = {
	{XtNfile, 0},
	{XtNtextOptions, (XtArgVal)scrollVertical },
	{XtNeditType, (XtArgVal)XawtextAppend},
	{XtNwidth, 500},
	{XtNheight, 100},
    };
    Widget top, p, w, text;
    char file[MAXPATHLEN];
    FILE *f;

    top = XtInitialize( "xclipboard", "XClipboard", table, XtNumber(table),
			  &argc, argv);

    XtGetApplicationResources(top, &app_resources, resources,
			      XtNumber(resources), NULL, ZERO);

    p = XtCreateManagedWidget("shell", formWidgetClass, top, NULL, ZERO);
    w = XtCreateManagedWidget("quit",  commandWidgetClass, p, NULL, ZERO);
    XtAddCallback(w, XtNcallback, Quit, NULL);
    w = XtCreateManagedWidget("erase", commandWidgetClass, p, NULL, ZERO);
    /*XtAddCallback(w, XtNcallback, Erase, NULL);*/
    XtSetSensitive(w, False);

    (void)tmpnam(file);
    if ((f = fopen(file, "w")) == NULL) {
	perror( argv[0] );
	exit(1);
    }
    fclose(f);

    textArgs[0].value = (XtArgVal)file;
    if (app_resources.word_wrap) textArgs[1].value |= wordBreak;

    text = XtCreateManagedWidget( "text", asciiDiskWidgetClass,
				  p, textArgs, XtNumber(textArgs) );

    XtRealizeWidget(top);
    unlink(file);

    (void)XmuInternAtom( XtDisplay(text), XmuMakeAtom("NULL") ); /* %%% */
    XtOwnSelection(text, XA_CLIPBOARD(XtDisplay(text)), CurrentTime,
		   ConvertSelection, LoseSelection, NULL);

    XtMainLoop();
}
