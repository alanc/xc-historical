#include "copyright.h"

/* $Header: XFetchName.c,v 11.19 87/11/11 14:32:05 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include <stdio.h>
#include "Xlibint.h"
#include "Xatom.h"
#include <X11/Xos.h>


Status XFetchName (dpy, w, name)
    register Display *dpy;
    Window w;
    char **name;
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    long leftover;
    unsigned char *data = NULL;
    if (XGetWindowProperty(dpy, w, XA_WM_NAME, 0L, (long)BUFSIZ, False, XA_STRING, 
	&actual_type,
	&actual_format, &nitems, &leftover, &data) != Success) {
        *name = NULL;
	return (0);
	}
    if ( (actual_type == XA_STRING) &&  (actual_format == 8) ) {

	/* The data returned by XGetWindowProperty is guarranteed to
	contain one extra byte that is null terminated to make retrieveing
	string properties easy. */

	*name = (char *)data;
	return(1);
	}
    if (data) Xfree ((char *)data);
    *name = NULL;
    return(0);
}

Status XGetIconName (dpy, w, icon_name)
    register Display *dpy;
    Window w;
    char **icon_name;
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    long leftover;
    unsigned char *data = NULL;
    if (XGetWindowProperty(dpy, w, XA_WM_ICON_NAME, 0L, (long)BUFSIZ, False,
        XA_STRING, 
	&actual_type,
	&actual_format, &nitems, &leftover, &data) != Success) {
        *icon_name = NULL;
	return (0);
	}
    if ( (actual_type == XA_STRING) &&  (actual_format == 8) ) {

	/* The data returned by XGetWindowProperty is guarranteed to
	contain one extra byte that is null terminated to make retrieveing
	string properties easy. */

	*icon_name = (char*)data;
	return(1);
	}
    if (data) Xfree ((char *)data);
    *icon_name = NULL;
    return(0);
}
