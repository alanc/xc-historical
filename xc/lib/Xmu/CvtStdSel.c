/* $XConsortium$
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * This file contains routines to handle common selection targets.
 *
 * Public entry points:
 *
 *	XmuInitAtoms(display)		initialize Selection Atoms
 *	XmuConvertStandardSelection()	return a known selection
 */

#include <X11/copyright.h>

#include <X11/Intrinsic.h>
#include <X11/Xatom.h>

#define _CvtStdSel_c_
#include "Xmu.h"

void XmuInitAtoms(d)
    Display *d;
{
    XA_TEXT		  = XInternAtom(d, "TEXT",		 False);
    XA_TIMESTAMP	  = XInternAtom(d, "TIMESTAMP",		 False);
    XA_LIST_LENGTH	  = XInternAtom(d, "LIST_LENGTH",	 False);
    XA_LENGTH		  = XInternAtom(d, "LENGTH",		 False);
    XA_TARGETS		  = XInternAtom(d, "TARGETS",		 False);
    XA_CHARACTER_POSITION = XInternAtom(d, "CHARACTER_POSITION", False);
    XA_DELETE		  = XInternAtom(d, "DELETE",		 False);
    XA_HOSTNAME		  = XInternAtom(d, "HOSTNAME",		 False);
    XA_IP_ADDRESS	  = XInternAtom(d, "IP_ADDRESS",	 False);
    XA_DECNET_ADDRESS	  = XInternAtom(d, "DECNET_ADDRESS",	 False);
    XA_USER		  = XInternAtom(d, "USER",		 False);
    XA_CLASS		  = XInternAtom(d, "CLASS",		 False);
    XA_NAME		  = XInternAtom(d, "NAME",		 False);
    XA_CLIENT_WINDOW	  = XInternAtom(d, "CLIENT_WINDOW",	 False);
    XA_ATOM_PAIR	  = XInternAtom(d, "ATOM_PAIR",		 False);
    XA_SPAN		  = XInternAtom(d, "SPAN",		 False);
    XA_NET_ADDRESS	  = XInternAtom(d, "NET_ADDRESS",	 False);
}

Boolean XmuConvertStandardSelection(w, time, selection, target,
				    type, value, length, format)
    Widget w;
    Time time;
    Atom *selection, *target, *type;
    caddr_t *value;
    unsigned long *length;
    int *format;
{
    static Boolean inited = False;
    if (!inited) {
	XmuInitAtoms(XtDisplay(w));
	inited = True;
    }
    if (*target == XA_TIMESTAMP) {
	*value = XtMalloc(4);
	if (sizeof(long) == 4)
	    *(long*)*value = time;
	else {
	    long temp = time;
	    bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    } else if (*target == XA_HOSTNAME) {
	char hostname[1024];
	gethostname(hostname, 1024);
	*value = XtNewString(hostname);
	*type = XA_STRING;
	*length = strlen(hostname);
	*format = 8;
	return True;
    } else if (*target == XA_IP_ADDRESS) {
    } else if (*target == XA_DECNET_ADDRESS) {
    } else if (*target == XA_USER) {
	char *name = (char*)getenv("USER");
	if (name == NULL) return False;
	*value = XtNewString(name);
	*type = XA_STRING;
	*length = strlen(name);
	*format = 8;
	return True;
    } else if (*target == XA_CLASS) {
    } else if (*target == XA_NAME) {
    } else if (*target == XA_CLIENT_WINDOW) {
    }
    /* else */
    return False;
}
