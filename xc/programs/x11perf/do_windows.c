/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#include "x11perf.h"

static Window *parents;
static Window *isolates;
static int childrows, childcolumns, childwindows;
static int parentrows, parentcolumns, parentwindows;
static int parentwidth, parentheight;
static Window popup;

void ComputeSizes(xp, p)
    XParms  xp;
    Parms   p;
{
    childwindows = p->objects;
    childrows = (childwindows + MAXCOLS - 1) / MAXCOLS;
    childcolumns = (childrows > 1 ? MAXCOLS : childwindows);

    parentwidth = (CHILDSIZE+CHILDSPACE) * childcolumns;
    parentheight = (CHILDSIZE+CHILDSPACE) * childrows;
}

void CreateParents(xp, p)
    XParms  xp;
    Parms   p;
{
    int     i;

    ComputeSizes(xp, p);

    parentcolumns = WIDTH / parentwidth;
    parentrows = HEIGHT / parentheight;
    parentwindows = parentcolumns * parentrows; /* Max reps we can fit */

    if (parentwindows > p->reps) {
	parentwindows = p->reps;
    } else {
	p->reps = parentwindows;
    }

    /* We will do parentwindows sets of childwindows, in order to get better
       timing accuracy.  Creating 4 windows at a millisecond apiece or so
       is a bit faster than the 60 Hz clock. */
    isolates = (Window *)malloc(parentwindows * sizeof(Window));
    parents = (Window *)malloc(parentwindows * sizeof(Window));

    /*
     *  Create isolation windows for the parents, and then the parents
     *  themselves.  These isolation windows ensure that parent and children
     *  windows created/mapped in DoWins and DoWin2 all see the same local
     *  environment...the parent is an only child, and each parent contains
     *  the number of children we are trying to get benchmarks on.
     */

    for (i = 0; i != parentwindows; i++) {
	isolates[i] = XCreateSimpleWindow(xp->d, xp->w,
	    (i/parentrows)*parentwidth, (i%parentrows)*parentheight,
	    parentwidth, parentheight, 0, xp->background, xp->background);
	parents[i] = XCreateSimpleWindow(xp->d, isolates[i],
	    0, 0, parentwidth, parentheight, 0, xp->background, xp->background);
    }

    XMapSubwindows(xp->d, xp->w);
} /* CreateParents */


void MapParents(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;

    for (i = 0; i != parentwindows; i++) {
	XMapWindow(xp->d, parents[i]);
    }
}


Bool InitCreate(xp, p)
    XParms  xp;
    Parms   p;
{
    CreateParents(xp, p);
    MapParents(xp, p);
    return True;
}

void CreateChildGroup(xp, p, parent)
    XParms  xp;
    Parms   p;
    Window  parent;
{
    int j;

    for (j = 0; j != childwindows; j++) {
	(void) XCreateSimpleWindow (xp->d, parent,
		(CHILDSIZE+CHILDSPACE) * (j/childrows) + CHILDSPACE/2,
		(CHILDSIZE+CHILDSPACE) * (j%childrows) + CHILDSPACE/2,
		CHILDSIZE, CHILDSIZE, 0, xp->background, xp->foreground);
    }

    if (p->special)
	XMapSubwindows (xp->d, parent);
}

void CreateChildren(xp, p)
    XParms  xp;
    Parms   p;
{
    int     i;

    for (i = 0; i != parentwindows; i++) {
	CreateChildGroup(xp, p, parents[i]);
    } /* end i */
}

void DestroyChildren(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;

    for (i = 0; i != parentwindows; i++) {
	XDestroySubwindows(xp->d, parents[i]);
    }
}

void EndCreate(xp, p)
    XParms  xp;
    Parms   p;
{
    XDestroySubwindows(xp->d, xp->w);
    free(parents);
    free(isolates);
}


Bool InitMap(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;

    CreateParents(xp, p);
    CreateChildren(xp, p);
    return True;
}

void UnmapParents(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;

    for (i = 0; i != parentwindows; i++) {
	XUnmapWindow(xp->d, parents[i]);
    }
}

Bool InitDestroy(xp, p)
    XParms  xp;
    Parms   p;
{
    CreateParents(xp, p);
    CreateChildren(xp, p);
    MapParents(xp, p);
    return True;
}

void DestroyParents(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;

    for (i = 0; i != parentwindows; i++) {
	XDestroyWindow(xp->d, parents[i]);
    }
}


void RenewParents(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;

    for (i = 0; i != parentwindows; i++) {
	parents[i] = XCreateSimpleWindow(xp->d, isolates[i],
	    0, 0, parentwidth, parentheight, 0, xp->background, xp->background);
    }
    CreateChildren(xp, p);
    MapParents(xp, p);
}

Bool InitPopups(xp, p)
    XParms  xp;
    Parms   p;
{
    XWindowAttributes    xwa;
    XSetWindowAttributes xswa;

    ComputeSizes(xp, p);
    CreateChildGroup(xp, p, xp->w);

    /* Now create simple window to pop up over children */
    (void) XGetWindowAttributes(xp->d, xp->w, &xwa);
    xswa.override_redirect = True;
    popup =  XCreateSimpleWindow (
	    xp->d, RootWindow(xp->d, 0), 
	    xwa.x + xwa.border_width, xwa.y + xwa.border_width,
	    parentwidth, parentheight,
	    0, xp->foreground, xp->background);
    XChangeWindowAttributes (xp->d, popup, CWOverrideRedirect, &xswa);
    return True;
}

void DoPopUps(xp, p)
    XParms  xp;
    Parms   p;
{
    int i;
    for (i = 0; i != p->reps; i++) {
        XMapWindow(xp->d, popup);
	XUnmapWindow(xp->d, popup);
    }
}

void EndPopups(xp, p)
    XParms  xp;
    Parms p;
{
    XDestroySubwindows(xp->d, xp->w);
    XDestroyWindow(xp->d, popup);
}

