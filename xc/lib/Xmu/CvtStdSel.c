/* $XConsortium$
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * This file contains routines to handle common selection targets.
 *
 * Public entry points:
 *
 *	XmuConvertStandardSelection()	return a known selection
 */

#include <X11/copyright.h>

#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include "Xmu.h"

Boolean XmuConvertStandardSelection(w, time, selection, target,
				    type, value, length, format)
    Widget w;
    Time time;
    Atom *selection, *target, *type;
    caddr_t *value;
    unsigned long *length;
    int *format;
{
    Display *d = XtDisplay(w);
    if (*target == XA_TIMESTAMP(d)) {
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
    }
    if (*target == XA_HOSTNAME(d)) {
	char hostname[1024];
	gethostname(hostname, 1024);
	*value = XtNewString(hostname);
	*type = XA_STRING;
	*length = strlen(hostname);
	*format = 8;
	return True;
    }
    if (*target == XA_IP_ADDRESS(d)) {
	return False;
    }
    if (*target == XA_DECNET_ADDRESS(d)) {
	return False;
    }
    if (*target == XA_USER(d)) {
	char *name = (char*)getenv("USER");
	if (name == NULL) return False;
	*value = XtNewString(name);
	*type = XA_STRING;
	*length = strlen(name);
	*format = 8;
	return True;
    }
    if (*target == XA_CLASS(d)) {
	return False;
    }
    if (*target == XA_NAME(d)) {
 	return False;
    }
    if (*target == XA_CLIENT_WINDOW(d)) {
	return False;
    }
    if (*target == XA_OWNER_OS(d)) {
	return False;
    }
    if (*target == XA_TARGETS(d)) {
#define NUM_TARGETS 9
	Atom* std_targets = (Atom*)XtMalloc(NUM_TARGETS*sizeof(Atom));
	std_targets[0] = XA_TIMESTAMP(d);
	std_targets[1] = XA_HOSTNAME(d);
	std_targets[2] = XA_IP_ADDRESS(d);
	std_targets[3] = XA_DECNET_ADDRESS(d);
	std_targets[4] = XA_USER(d);
	std_targets[5] = XA_CLASS(d);
	std_targets[6] = XA_NAME(d);
	std_targets[7] = XA_CLIENT_WINDOW(d);
	std_targets[8] = XA_OWNER_OS(d);
	*value = (caddr_t)std_targets;
	*type = XA_ATOM;
	*length = NUM_TARGETS;
	*format = 32;
	return True;
    }
    /* else */
    return False;
}
