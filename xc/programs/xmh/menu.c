/*
 * $XConsortium: menu.c,v 1.2 89/09/17 19:40:40 converse Exp $
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


void AttachMenuToButton(button, menu, menu_name)
    Button	button;
    Widget	menu;
    char *	menu_name;
{
    Arg		args[3];

    if (button == NULL) return;
    button->menu = menu;
    XtSetArg(args[0], XtNmenuName, XtNewString(menu_name));
    XtSetValues(button->widget, args, (Cardinal) 1);
}


/*ARGSUSED*/
void DoRememberMenuSelection(widget, client_data, call_data)
    Widget	widget;		/* menu */
    XtPointer	client_data;	/* menu entry name */
    XtPointer	call_data;
{
    static Arg	args[] = {
	{ XtNpopupOnEntry,	(XtArgVal) NULL },
    };
    args[0].value = (XtArgVal) client_data;
    XtSetValues(widget, args, XtNumber(args));
}


void SendMenuEntryEnableMsg(button, entry_name, value)
    Button	button;
    char *	entry_name;
    int		value;
{
    static Arg	args[] = { XtNsensitive, (XtArgVal) NULL };
    args[0].value = (XtArgVal) ((value == 0) ? False : True);
    XawSimpleMenuSetEntryValues(button->menu, entry_name, args, (Cardinal) 1);
}


void ToggleMenuItem(widget, name, state)
    Widget	widget;
    char 	*name;
    Boolean	state;
{
    Arg		args[1];
    
    XtSetArg(args[0], XtNleftBitmap, (state ? MenuItemBitmap : None));
    XawSimpleMenuSetEntryValues(widget, name, args, (Cardinal) 1);
}
