/*
 * $XConsortium: sharedlib.c,v 1.4 89/09/26 23:34:44 jim Exp $
 * 
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * This file is used to force shared libraries to get the right routines.
 */

#ifndef SUNSHLIB
static int dummy;			/* avoid warning from ranlib */
#else

#include "IntrinsicI.h"

/*
 * _XtInherit needs to be statically linked since it is compared against as
 * well as called.
 */
void _XtInherit()
{
    extern void __XtInherit();
    __XtInherit();
}

/*
 * The following routine will be called by every toolkit
 * application, forcing this file to be statically linked.
 *
 * Note: Both XtInitialize and XtAppInitialize call XtToolkitInitialize.
 */

void XtToolkitInitialize()
{
    extern void _XtToolkitInitialize();
    _XtToolkitInitialize();
}

Widget 
XtInitialize(name, classname, options, num_options, argc, argv)
String name, classname;
XrmOptionDescRec *options;
Cardinal num_options, *argc;
String *argv;
{
    extern Widget _XtInitialize();
    return _XtInitialize (name, classname, options, num_options, argc, argv);
}

Widget
XtAppInitialize(app_context_return, application_class, options, num_options,
		argc_in_out, argv_in_out, fallback_resources, 
		args_in, num_args_in)
XtAppContext * app_context_return;
String application_class;
XrmOptionDescRec *options;
Cardinal num_options, *argc_in_out, num_args_in;
String *argv_in_out, * fallback_resources;     
ArgList args_in;
{
    extern Widget _XtAppInitialize();
    return _XtAppInitialize (app_context_return, application_class, options,
			     num_options, argc_in_out, argv_in_out, 
			     fallback_resources, args_in, num_args_in);
}

#endif /* SUNSHLIB */
