/* $XConsortium: XScrResStr.c,v 1.1 91/02/03 13:26:35 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1991	*/

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

#include "Xlibint.h"
#include <X11/Xatom.h>

char *XScreenResourceString(screen)
	Screen *screen;
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long leftover;
    char *val = NULL;

    if (XGetWindowProperty(screen->display, screen->root,
			   XInternAtom(screen->display, "SCREEN_RESOURCES"),
			   0L, 100000000L, False,
			   XA_STRING, &actual_type, &actual_format,
			   &nitems, &leftover,
			   (unsigned char **) &val) == Success) {
	if ((actual_type == XA_STRING) && (actual_format == 8))
	    return val;
	if (val)
	    Xfree(val);
    }
    return (char *)NULL;
}
