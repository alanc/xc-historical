/*
 * $XConsortium: sharedlib.c,v 1.3 89/09/18 17:54:09 kit Exp $
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

#endif /* SUNSHLIB */
