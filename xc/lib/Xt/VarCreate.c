#ifndef lint
static char Xrcsid[] =
    "$XConsortium: VarCreate.c,v 1.5 89/11/10 17:47:07 swick Exp $";
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

#include "IntrinsicI.h"
#include <X11/StringDefs.h>
#include <X11/Shell.h>
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


#if IncludePrototypes
Widget
XtVaCreateWidget(String name, WidgetClass widget_class, Widget parent, ...)
#else
/*VARARGS3*/
Widget XtVaCreateWidget(name, widget_class, parent, va_alist)
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


#if IncludePrototypes
Widget
XtVaCreateManagedWidget(String name, WidgetClass widget_class, Widget parent, ...)
#else
/*VARARGS3*/
Widget XtVaCreateManagedWidget(name, widget_class, parent, va_alist)
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


#if IncludePrototypes
Widget
XtVaAppCreateShell(String name, String class, WidgetClass widget_class, Display* display, ...)
#else
/*VARARGS4*/
Widget XtVaAppCreateShell(name, class, widget_class, display, va_alist)
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


#if IncludePrototypes
Widget
XtVaCreatePopupShell(String name, WidgetClass widget_class, Widget parent, ...)
#else
/*VARARGS3*/
Widget XtVaCreatePopupShell(name, widget_class, parent, va_alist)
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

#if IncludePrototypes
void
XtVaSetValues(Widget widget, ...)
#else
/*VARARGS1*/
void XtVaSetValues(widget, va_alist)
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


#if IncludePrototypes
void
XtVaSetSubvalues(XtPointer base, XtResourceList resources, Cardinal num_resources, ...)
#else
/*VARARGS3*/
void XtVaSetSubvalues(base, resources, num_resources, va_alist)
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

#if IncludePrototypes
Widget
XtVaAppInitialize(XtAppContext *app_context_return, String application_class,
		  XrmOptionDescList options, Cardinal num_options,
		  Cardinal *argc_in_out, String *argv_in_out,
		  String *fallback_resources, ...)
#else
/*VARARGS7*/
Widget XtVaAppInitialize(app_context_return, application_class, options,
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
    XtAppContext app_con;
    Display * dpy;
    String *saved_argv;
    register int i, saved_argc = *argc_in_out;
    Widget root;
    va_list var;
    String attr;
    int count = 0;
    XtTypedArgList typed_args;

    XtToolkitInitialize();
    
/*
 * Save away argv and argc so we can set the properties later 
 */
    
    saved_argv = (String *)
	ALLOCATE_LOCAL( (Cardinal)((*argc_in_out + 1) * sizeof(String)) );
    if (saved_argv == NULL) _XtAllocError(NULL);

    for (i = 0 ; i < saved_argc ; i++) saved_argv[i] = argv_in_out[i];
    saved_argv[i] = NULL;	/* NULL terminate that sucker. */


    app_con = XtCreateApplicationContext();

    if (fallback_resources != NULL) /* save a procedure call */
	XtAppSetFallbackResources(app_con, fallback_resources);

    dpy = XtOpenDisplay(app_con, (String) NULL, NULL, application_class,
			options, num_options, argc_in_out, argv_in_out);

    if (dpy == NULL)
	XtErrorMsg("invalidDisplay","xtInitialize","XtToolkitError",
                   "Can't Open display", (String *) NULL, (Cardinal *)NULL);

    Va_start(var, fallback_resources);
    for(attr = va_arg(var,String); attr != NULL; attr = va_arg(var,String)) {
        ++count;
        if (strcmp(attr, XtVaTypedArg) == 0) {
            va_arg(var, String);
            va_arg(var, String);
            va_arg(var, XtArgVal);
            va_arg(var, int);
        } else {
            va_arg(var, XtArgVal);
        }
    }
    va_end(var);

    Va_start(var, fallback_resources);
    typed_args = _XtVaCreateTypedArgList(var, count);
    va_end(var);
    
    root =
	XtVaAppCreateShell( NULL, application_class, 
			    applicationShellWidgetClass, dpy,
			    XtNscreen, (XtArgVal)DefaultScreenOfDisplay(dpy),
			    XtNargc, (XtArgVal)saved_argc,
			    XtNargv, (XtArgVal)saved_argv,
			    XtVaNestedList, (XtVarArgsList)typed_args,
			    NULL );
   
    if (app_context_return != NULL)
	*app_context_return = app_con;

    XtFree((XtPointer)typed_args);
    DEALLOCATE_LOCAL((XtPointer)saved_argv);
    return(root);
}

