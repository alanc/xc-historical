#ifndef VMS
#include <X11/Xatom.h>
#else
#include <decw$include/Xatom.h>
#endif

#include "x11perf.h"

static Atom XA_PK_TEMP;

void DoAtom(d, p)
    Display *d;
    Parms p;
{
    char   *atom;
    int     i;

    for (i = 0; i < p->reps; i++) {
	atom = XGetAtomName (d, 1);
    }
}

void InitGetProp(d, p)
    Display *d;
    Parms p;
{
    int foo = 41;

    XA_PK_TEMP = XInternAtom (d, "_PK_TEMP", False);
    XChangeProperty (
	    d, root, XA_PK_TEMP, XA_INTEGER, 32,
	    PropModeReplace, &foo, sizeof (int));
}

void DoGetProp(d, p)
    Display *d;
    Parms p;
{
    char   *atom;
    int     i, status;
    int     prop, actual_format, actual_length, bytes_remaining;
    Atom actual_type;

    for (i = 0; i < p->reps; i++) {
	status = XGetWindowProperty (
		d, root, XA_PK_TEMP, 0, sizeof (int),
		False, AnyPropertyType, &actual_type, &actual_format,
		&actual_length, &bytes_remaining, &prop);
    }
}
