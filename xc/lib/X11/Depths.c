/* Copyright 1989 Massachusetts Institute of Technology */

#include "copyright.h"

#include <stdio.h>
#include "Xlibint.h"

/*
 * XListDepths - return info from connection setup
 */
Bool XListDepths (dpy, scrnum, depthsp, countp)
    Display *dpy;
    int scrnum;
    int **depthsp;
    int *countp;
{
    Screen *scr;
    int count;
    int *depths;

    if (scrnum < 0 || scrnum > dpy->nscreens) return False;

    scr = &dpy->screens[scrnum];
    count = scr->ndepths;
    depths = NULL;
    if (count > 0) {
	register Depth *dp;
	register int i;

	depths = (int *) Xmalloc (count * sizeof(int));
	if (!depths) return False;
	for (i = 0, dp = scr->depths; i < count; i++, dp++) {
	    depths[i] = dp->depth;
	}
    }
    *countp = count;
    *depthsp = depths;
    return True;
}
