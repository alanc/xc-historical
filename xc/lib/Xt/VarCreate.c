#ifndef lint
static char Xrcsid[] =
    "$XConsortium: VarCreate.c,v 1.2 89/10/09 13:38:15 swick Exp $";
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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "VarargsI.h"

extern Widget _XtCreateWidget();
extern Widget _XtAppCreateShell();
extern Widget _XtCreatePopupShell();

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

    _XtVaToTypedArgList(var, &typed_args, &num_args);

    widget = _XtCreateWidget(name, widget_class, parent, (ArgList)NULL, 
		    (Cardinal)0, typed_args, num_args);

    if (typed_args != NULL) {
        XtFree((char *)typed_args);
    }    

    return(widget);
}


#ifdef _STDC_
Widget
XtVaCreateWidget(String name, WidgetClass widget_class, Widget parent, ...)
#else
Widget
XtVaCreateWidget(name, widget_class, parent, va_alist)
    String name;
    WidgetClass widget_class;
    Widget parent;
    va_dcl
#endif
{
    va_list                 var;
    register Widget         widget;

    Va_start(var,parent);
    widget = _XtVaCreateWidget(name, widget_class, parent, var);
    va_end(var);

    return(widget);
}


#ifdef _STDC_
Widget
XtVaCreateManagedWidget(String name, WidgetClass widget_class, Widget parent, ...)
#else
Widget
XtVaCreateManagedWidget(name, widget_class, parent, va_alist)
    String name;
    WidgetClass widget_class;
    Widget parent;
    va_dcl
#endif
{
    va_list		var;
    register Widget	widget;

    Va_start(var,parent);
    widget = _XtVaCreateWidget(name, widget_class, parent, var);
    XtManageChild(widget);
    va_end(var);

    return (widget);
}


#ifdef _STDC_
Widget
XtVaAppCreateShell(String name, String class, WidgetClass widget_class, Display* display, ...)
#else
Widget
XtVaAppCreateShell(name, class, widget_class, display, va_alist)
    String name;
    String class;
    WidgetClass widget_class;
    Display* display;
    va_dcl
#endif
{
    va_list                 var;
    register Widget         widget;
    XtTypedArgList          typed_args = NULL;
    Cardinal                num_args;

    Va_start(var,display);

    _XtVaToTypedArgList(var, &typed_args, &num_args);
    widget = _XtAppCreateShell(name, class, widget_class, display,
		(ArgList)NULL, (Cardinal)0, typed_args, num_args);
    if (typed_args != NULL) {
	XtFree((char *)typed_args);
    }
 
    va_end(var);
    return(widget);
}


#ifdef _STDC_
Widget
XtVaCreatePopupShell(String name, WidgetClass widget_class, Widget parent, ...)
#else
Widget
XtVaCreatePopupShell(name, widget_class, parent, va_alist)
    String name;
    WidgetClass widget_class;
    Widget parent;
    va_dcl
#endif
{
    va_list                 var;
    register Widget         widget;
    XtTypedArgList          typed_args = NULL;
    Cardinal                num_args;

    Va_start(var,parent);

    _XtVaToTypedArgList(var, &typed_args, &num_args);
    widget = _XtCreatePopupShell(name, widget_class, parent,
		(ArgList)NULL, (Cardinal)0, typed_args, num_args);
    if (typed_args != NULL) {
	XtFree((char *)typed_args);
    }

    va_end(var);
    return widget;
}

#ifdef _STDC_
void
XtVaSetValues(Widget widget, ...)
#else
void
XtVaSetValues(widget, va_alist)
    Widget widget;
    va_dcl
#endif
{
    va_list                 var;
    ArgList                 args = NULL;
    Cardinal                num_args;

    Va_start(var,widget);

    _XtVaToArgList(widget, &args, &num_args, var);
    XtSetValues(widget, args, num_args);
    if (args != NULL) {
	XtFree((char *)args);
    }

    va_end(var);
}


#ifdef _STDC_
void
XtVaSetSubvalues(XtPointer base, XtResourceList resources, Cardinal num_resources, ...)
#else
void
XtVaSetSubvalues(base, resources, num_resources, va_alist)
    XtPointer base;
    XtResourceList resources;
    Cardinal num_resources;
    va_dcl
#endif
{
    va_list	var;
    ArgList    	args;
    Cardinal   	num_args;
    int		total_count, typed_count;		

    Va_start(var, num_resources);

    _XtCountVaList(var, &total_count, &typed_count);

    if (typed_count != 0) {
	XtWarning("XtVaTyped is an invalid argument to XtVaSetSubvalues()\n");
    }

    _XtVaToArgList((Widget)NULL, &args, &num_args, var);

    XtSetSubvalues(base, resources, num_resources, args, num_args);

    if (num_args != 0) {
        XtFree((char *)args);
    }    

    va_end(var);
}

#ifdef _STDC_
Widget
XtVaAppInitialize(XtAppContext *app_context_return, String application_class,
		  XrmOptionDescList options, Cardinal num_options,
		  Cardinal *argc_in_out, String *argv_in_out,
		  String *fallback_resources, ...)
#else
Widget
XtVaAppInitialize(app_context_return, application_class, options,
		  num_options, argc_in_out, argv_in_out,
		  fallback_resources, va_alist)
    XtAppContext *app_context_return;
    String application_class;
    XrmOptionDescList options;
    Cardinal num_options;
    Cardinal *argc_in_out;
    String *argv_in_out;
    String *fallback_resources;
    va_dcl
#endif
{
	Widget		w;
	ArgList		args = (ArgList)NULL;
	int		num_args;
	va_list		var;
	int		total_count, typed_count;		

	Va_start(var, fallback_resources);

	_XtCountVaList(var, &total_count, &typed_count);

	if (typed_count != 0) {
		XtWarning("XtVaTypedArg is an invalid argument to XtVaAppInitialize()\n");
	}

	_XtVaToArgList((Widget)NULL, &args, &num_args, var);
	w = XtAppInitialize(app_context_return, application_class, options,
			    num_options, argc_in_out, argv_in_out,
			    fallback_resources, args, num_args);

	if (args != (ArgList)NULL) {
		XtFree((char *)args);
	}

	va_end(var);

	return	w;
}

