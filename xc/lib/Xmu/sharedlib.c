/*
 * $XConsortium: sharedlib.c,v 1.4 90/07/02 17:17:38 rws Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * This file is used to force shared libraries to get the right routines.
 */

#ifdef SUNSHLIB
#ifndef SHAREDCODE
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

/* this is really gross, but it works around bizarre Sun shared library bugs */
#include "xtsharedlib.c"

#endif /* not SHAREDCODE */
#endif /* SUNSHLIB */

#if !defined(SUNSHLIB) || defined(SHAREDCODE)
static int dummy;			/* avoid warning from ranlib */
#endif
