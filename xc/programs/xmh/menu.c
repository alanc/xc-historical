/*
 * $XConsortium: menu.c,v 1.1 89/09/15 16:15:27 converse Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
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
 * Author:  Donna Converse, MIT X Consortium
 */

#include "xmh.h"
#include "bboxint.h"

 
void CreateMenu(button, dynamic)
    Button	button;		/* parent data structure */
    Boolean	dynamic;	/* whether menu size will change dynamically */
{
    Cardinal	i;
    Arg		args[3];

    i = 0;
    if (dynamic) {
	XtSetArg(args[i], XtNallowShellResize, True);	i++;
    }
    button->menu = XtCreatePopupShell("menu", simpleMenuWidgetClass,
				      button->widget, args, i);
}


void AddMenuEntry(button, name, callback_proc, callback_data, sensitive)
    Button		button;		/* the menu button */
    char *		name;		/* the name of the menu entry */
    XtCallbackProc	callback_proc;
    XtPointer		callback_data;
    Boolean		sensitive;
{
    Cardinal		i;
    Arg			args[3];
    static XtCallbackRec callbacks[] = {
	{ (XtCallbackProc) NULL, (XtPointer) NULL},
	{ (XtCallbackProc) NULL, (XtPointer) NULL},
    };

    i = 0;
    if (callback_proc) {
	callbacks[0].callback = callback_proc;
	callbacks[0].closure  = callback_data;
	XtSetArg(args[i], XtNcallback, callbacks);	i++;
    }

    /* The menu entry is sensitive by default. */
    if (! sensitive) {
	XtSetArg(args[i], XtNsensitive, False);		i++;
    }
    XawSimpleMenuAddEntry(button->menu, name, args, i);
}


void SendMenuEntryEnableMsg(button, entry_name, value)
    Button	button;
    char *	entry_name;
    int		value;
{
    static Arg	args[] = { XtNsensitive, (XtArgVal) NULL };
    args[0].value = ((value == 0) ? (XtArgVal) False : (XtArgVal) True);
    XawSimpleMenuSetEntryValues(button->menu, entry_name, args, (Cardinal) 1);
}
    
