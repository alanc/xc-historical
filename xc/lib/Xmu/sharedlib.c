/*
 * $XConsortium: sharedlib.c,v 1.1 89/10/09 13:49:45 jim Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * This file is used to force shared libraries to get the right routines.
 */

#if !defined(SHAREDCODE) && defined(SUNSHLIB)
#include <X11/IntrinsicP.h>

/*
 * The following hack is used by XmuConvertStandardSelection to get the
 * following class records.  Without these, a runtime undefined symbol error 
 * occurs.
 */
extern WidgetClass applicationShellWidgetClass,wmShellWidgetClass;

WidgetClass get_applicationShellWidgetClass()
{
    return applicationShellWidgetClass;
}

WidgetClass get_wmShellWidgetClass()
{
    return wmShellWidgetClass;
}

#else
static int dummy;                       /* avoid warning from ranlib */
#endif /* SHAREDCODE && SUNSHLIB */

