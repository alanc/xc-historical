/* $XConsortium: copyright.h,v 1.4 89/07/25 16:12:03 rws Exp $ */
#ifndef lint
static char Xrcsid[] = "$XConsortium: Intrinsic.c,v 1.138 89/10/08 18:23:06 jim Exp $";
#endif

/*

Copyright 1985, 1986, 1987, 1988, 1989 by the
Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
M.I.T. makes no representations about the suitability of
this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#include "VarargsI.h"
#include "StringDefs.h"

static Widget
_XtVaCreateWidget(name, widget_class, parent, var)
    String      name;
    WidgetClass widget_class;
    Widget      parent;
    va_list     var;
{
    register Widget         widget;
    XtTypedArgList	    typed_args = NULL;
    Cardinal		    num_args;

    XtVaToTypedArgList(var, &typed_args, &num_args);

    widget = _XtCreateWidget(name, widget_class, parent, (ArgList)NULL, 
		    (Cardinal)0, typed_args, num_args);

    if (typed_args != NULL) {
        XtFree((char *)typed_args);
    }    

    return(widget);
}


Widget
XtVaCreateWidget(String name, WidgetClass widget_class, Widget parent, ...)
{
    va_list                 var;
    register Widget         widget;

    va_start(var,parent);
    widget = _XtVaCreateWidget(name, widget_class, parent, var);
    va_end(var);

    return(widget);
}


Widget
XtVaCreateManagedWidget(String name, WidgetClass widget_class, Widget parent, ...)
{
    va_list		var;
    register Widget	widget;

    va_start(var,parent);
    widget = _XtVaCreateWidget(name, widget_class, parent, var);
    XtManageChild(widget);
    va_end(var);

    return (widget);
}


Widget
XtVaAppCreateShell(String name, String class, WidgetClass widget_class, Display* display, ...)
{
    va_list                 var;
    register Widget         widget;
    XtTypedArgList          typed_args = NULL;
    Cardinal                num_args;

    va_start(var,display);

    XtVaToTypedArgList(var, &typed_args, &num_args);
    widget = _XtAppCreateShell(name, class, widget_class, display,
		(ArgList)NULL, (Cardinal)0, typed_args, num_args);
    if (typed_args != NULL) {
	XtFree((char *)typed_args);
    }
 
    va_end(var);
    return(widget);
}


Widget
XtVaCreatePopupShell(String name, WidgetClass widget_class, Widget parent, ...)
{
    va_list                 var;
    register Widget         widget;
    XtTypedArgList          typed_args = NULL;
    Cardinal                num_args;

    va_start(var,parent);

    XtVaToTypedArgList(var, &typed_args, &num_args);
    widget = _XtCreatePopupShell(name, widget_class, parent,
		(ArgList)NULL, (Cardinal)0, typed_args, num_args);
    if (typed_args != NULL) {
	XtFree((char *)typed_args);
    }

    va_end(var);
    return widget;
}

void
XtVaSetValues(Widget widget, ...)
{
    va_list                 var;
    ArgList                 args = NULL;
    Cardinal                num_args;

    va_start(var,widget);

    XtVaToArgList(widget, &args, &num_args, var);
    XtSetValues(widget, args, num_args);
    if (args != NULL) {
	XtFree((char *)args);
    }

    va_end(var);
}


void
XtVaSetSubvalues(caddr_t base, XtResourceList resources, Cardinal num_resources, ...)
{
    va_list	var;
    ArgList    	args;
    Cardinal   	num_args;
    int		total_count, typed_count;		

    va_start(var, num_resources);

    _XtCountVaList(var, &total_count, &typed_count);

    if (typed_count != 0) {
	XtWarning("XtVaTyped is an invalid argument to XtVaSetSubvalues()\n");
    }

    XtVaToArgList((Widget)NULL, &args, &num_args, var);

    XtSetSubvalues(base, resources, num_resources, args, num_args);

    if (num_args != 0) {
        XtFree((char *)args);
    }    

    va_end(var);
}
/*
Widget
XtVaAppInitialize(XtAppContext *app_context_return, String application_class, XrmOptionsDescList options, Cardinal num_options, Cardinal *argc_in_out, String *argv_in_out, String *fallback_resources, ...)
{
	Widget		w;
	ArgList		args = (ArgList)NULL;
	int		num_args;
	va_list		var;
	int		total_count, typed_count;		

	va_start(var, fallback_resources);

	_XtCountVaList(var, &total_count, &typed_count);

	if (typed_count != 0) {
		XtWarning("XtVaTyped is an invalid argument to XtVaAppInitialize()\n");
	}

	XtVaToArgList((Widget)NULL, &args, &num_args, var);
	w = XtAppInitialize(app_context_return, application_class, options,
			    num_options, argc_in_out, argv_in_out,
			    fallback_resources, args, num_args);

	if (args != (ArgList)NULL) {
		XtFree((char *)args);
	}

	va_end(var);

	return	w;
}
*/
