#include "x11perf.h"

static Window *parents;
static int childrows, childcolumns, childwindows;
static int parentrows, parentcolumns, parentwindows;
static int parentwidth, parentheight;
static Window w;
static Window popup;

void ComputeSizes(d, p)
    Display *d;
    Parms p;
{
    childwindows = p->objects;
    if (childwindows < 50) {
	childcolumns = childwindows;
	childrows = 1;
    } else {
	childcolumns = COLS;
	childrows = childwindows/COLS;
    }
    parentwidth = (CHILDSIZE+CHILDSPACE) * childcolumns;
    parentheight = (CHILDSIZE+CHILDSPACE) * childrows;
}

void CreateParents(d, p)
    Display *d;
    Parms p;
{
    Window isolate;
    int i, x, y;

    ComputeSizes(d, p);

    parentcolumns = WIDTH / parentwidth;
    parentrows = HEIGHT / parentheight;
    parentwindows = parentcolumns * parentrows;
    parents = (Window *)malloc(parentwindows * sizeof(Window));

    /* We will do parentwindows sets of childwindows, in order to get better
       timing accuracy.  Creating 4 windows at a millisecond apiece or so
       is a bit faster than the 60 Hz clock. */
    p->reps = parentwindows;

    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, NULL, NULL);

    /*
     *  Create isolation windows for the parents, and then the parents
     *  themselves.  These isolation windows ensure that parent and children
     *  windows created/mapped in DoWins and DoWin2 all see the same local
     *  environment...the parent is an only child, and each parent contains
     *  the number of children we are trying to get benchmarks on.
     */

    i = 0;
    for (x = 0; x < parentcolumns; x++) {
	for (y = 0; y < parentrows; y++) {
	    isolate = XCreateSimpleWindow(d, w,
		x*parentwidth, y*parentheight, parentwidth, parentheight,
		0, bgPixel, bgPixel);
	    parents[i] = XCreateSimpleWindow(d, isolate,
		0, 0, parentwidth, parentheight,
		0, bgPixel, bgPixel);
	    i++;
	}
    }
    XMapSubwindows(d, w);
} /* CreateParents */

void MapParents(d, p)
    Display *d;
    Parms p;
{
    int i;

    for (i = 0; i < parentwindows; i++) {
	XMapWindow(d, parents[i]);
    }
}


Bool InitCreate(d, p)
    Display *d;
    Parms p;
{
    CreateParents(d, p);
    MapParents(d, p);
    return True;
}

void CreateChildGroup(d, p, parent)
    Display *d;
    Parms p;
    Window  parent;
{
    int j, x, y;

    j = 0;
    for (x = 0; x < childcolumns; x++) {
	for (y = 0; y < childrows; y++) {
	    (void) XCreateSimpleWindow (d, parent,
		(CHILDSIZE+CHILDSPACE) * x + CHILDSPACE/2,
		(CHILDSIZE+CHILDSPACE) * y + CHILDSPACE/2,
		CHILDSIZE, CHILDSIZE, 0, bgPixel, fgPixel);
	    j++;
	    if (j == childwindows)
		goto Enough;
	}
    }
Enough: 
    if (p->special)
	XMapSubwindows (d, parent);
}

void CreateChildren(d, p)
    Display *d;
    Parms p;
{
    int     i;

    for (i = 0; i < parentwindows; i++) {
	CreateChildGroup(d, p, parents[i]);
    } /* end i */
}

void DeleteChildren(d, p)
    Display *d;
    Parms p;
{
    int i;

    for (i = 0; i < parentwindows; i++) {
	XDestroySubwindows(d, parents[i]);
    }
}

void EndCreate(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
}


Bool InitMap(d, p)
    Display *d;
    Parms p;
{
    int i;

    CreateParents(d, p);
    CreateChildren(d, p);
    return True;
}

void UnmapParents(d, p)
    Display *d;
    Parms p;
{
    int i;

    for (i = 0; i < parentwindows; i++) {
	XUnmapWindow(d, parents[i]);
    }
}


Bool InitPopups(d, p)
    Display *d;
    Parms p;
{
    int i, x, y;
    XSetWindowAttributes xswa;

    ComputeSizes(d, p);
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, NULL, NULL);
    CreateChildGroup(d, p, w);

    /* Now create simple window to pop up over children */
    xswa.override_redirect = True;
    popup =  XCreateSimpleWindow (
	    d, RootWindow(d, 0), 50, 50, parentwidth, parentheight,
	    0, fgPixel, bgPixel);
    XChangeWindowAttributes (d, popup, CWOverrideRedirect, &xswa);
    return True;
}

void DoPopUps(d, p)
    Display *d;
    Parms p;
{
    int i;
    for (i = 0; i < p->reps; i++)
    {
        XMapWindow(d, popup);
	XUnmapWindow(d, popup);
    }
}

void EndPopups(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
    XDestroyWindow(d, popup);
}

