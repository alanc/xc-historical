#include "VarargsI.h"
#include "StringDefs.h"

static Widget
_XtVaCreateWidget(name, widgetClass, parent, var)
    String      name;
    WidgetClass widgetClass;
    Widget      parent;
    va_list     var;
{
    register Widget         widget;
    XtTypedArgList	    typed_args = NULL;
    Cardinal		    num_args;

    XtVaToTypedArgList(var, &typed_args, &num_args);

    widget = _XtCreateWidget(name, widgetClass, parent, (ArgList)NULL, 
		    (Cardinal)0, typed_args, num_args);

    if (typed_args != NULL) {
        XtFree((char *)typed_args);
    }    

    return(widget);
}


Widget
XtVaCreateWidget(String name, WidgetClass widgetClass, Widget parent, ...)
{
    va_list                 var;
    register Widget         widget;

    va_start(var,parent);
    widget = _XtVaCreateWidget(name, widgetClass, parent, var);
    va_end(var);

    return(widget);
}


Widget
XtVaCreateManagedWidget(String name, WidgetClass widgetClass, Widget parent, ...)
{
    va_list		var;
    register Widget	widget;

    va_start(var,parent);
    widget = _XtVaCreateWidget(name, widgetClass, parent, var);
    XtManageChild(widget);
    va_end(var);

    return (widget);
}


Widget
XtVaAppCreateShell(String name, String class, WidgetClass widgetClass, Display* display, ...)
{
    va_list                 var;
    register Widget         widget;
    XtTypedArgList          typed_args = NULL;
    Cardinal                num_args;

    va_start(var,display);

    XtVaToTypedArgList(var, &typed_args, &num_args);
    widget = _XtAppCreateShell(name, class, widgetClass, display,
		(ArgList)NULL, (Cardinal)0, typed_args, num_args);
    if (typed_args != NULL) {
	XtFree((char *)typed_args);
    }
 
    va_end(var);
    return(widget);
}


Widget
XtVaCreatePopupShell(String name, WidgetClass widgetClass, Widget parent, ...)
{
    va_list                 var;
    register Widget         widget;
    XtTypedArgList          typed_args = NULL;
    Cardinal                num_args;

    va_start(var,parent);

    XtVaToTypedArgList(var, &typed_args, &num_args);
    widget = _XtCreatePopupShell(name, widgetClass, parent,
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
