/*
 * $XConsortium: sharedlib.c,v 1.2 89/08/23 19:02:41 jim Exp $
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
