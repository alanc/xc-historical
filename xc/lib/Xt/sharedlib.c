/*
 * $XConsortium$
 * 
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * This file is used to force shared libraries to get the right routines.
 */

#ifdef SUNSHLIB

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
 * One of the following three routines will be called by every toolkit
 * application, forcing this file to be statically linked.
 *
 * XXX - add XtAppInitialize when it is coded.
 */

void XtToolkitInitialize()
{
    extern void _XtToolkitInitialize();
    _XtToolkitInitialize();
}

Widget XtInitialize (name, classname, urlist, num_urs, argc, argv)
	char *name;		/* unused in R3 */
	char *classname;
	XrmOptionDescRec *urlist;
	Cardinal num_urs;
	Cardinal *argc;
	char *argv[];
{
    extern Widget _XtInitialize();
    return _XtInitialize (name, classname, urlist, num_urs, argc, argv);
}

#endif /* SUNSHLIB */
