#include "x11perf.h"

static Window w;
static Window popup;
static rows, columns;

void InitWins(d, p)
    Display *d;
    Parms p;
{
    int i, x, y;

    if (p->objects < 50) {
	rows = 1;
	columns = p->objects;
    }
    else {
	columns = COLS;
	rows = p->objects/COLS;
    }
    p->objects = rows * columns;
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, NULL, NULL);

}

void DoWins(d, p)
    Display *d;
    Parms p;
{
    int     x, y, i;
    int     child;
    int     wp = WhitePixel (d, 0);
    int     bp = BlackPixel (d, 0);

    child = 0;
    for (x = 0; x < COLS; x++)
	for (y = 0; y < rows; y++) {
	    (void) XCreateSimpleWindow (
		    d, w, x*20 + 10, y*20 + 10, 10, 10, 0, wp, bp);
	    if ((++child) == p->objects)
		goto Enough;
	}
Enough: 
    if (p->special)
	XMapSubwindows (d, w);
}

static void CreateSubs(d, p, rows, cols)
    Display *d;
    Parms p;
    int rows, cols;
{
    int     x, y, i;
    int     child;
    int     wp = WhitePixel (d, 0);
    int     bp = BlackPixel (d, 0);

    child = 0;
    for (x = 0; x < cols; x++)
	for (y = 0; y < rows; y++) {
	    (void) XCreateSimpleWindow (
		    d, w, x*20 + 10, y*20 + 10, 10, 10, 0, wp, bp);
	    if ((++child) == p->objects)
	        goto Enough ;
	}
Enough:
    XMapSubwindows (d, w);
}

void InitWins2(d, p)
    Display *d;
    Parms p;
{
    int i, x, y;

    if (p->objects < 50) {
	rows = 1;
	columns = p->objects;
    }
    else {
	columns = COLS;
	rows = p->objects/COLS;
    }
    p->objects = rows * columns;
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, NULL, NULL);
    XUnmapWindow(d, w);
    CreateSubs(d, p, rows, columns);
}

void DoWins2(d, p)
    Display *d;
    Parms p;
{
    XMapWindow (d, w);
}

void UnmapWin(d, p)
    Display *d;
    Parms p;
{
    XUnmapWindow(d, w);
}

void InitPopups(d, p)
    Display *d;
    Parms p;
{
#define POPUPCOLS 8
    int i, x, y;
    XSetWindowAttributes xswa;
    int     wp = WhitePixel (d, 0);
    int     bp = BlackPixel (d, 0);

    if (p->objects < 10) {
	rows = 1;
	columns = p->objects;
    }
    else {
	columns = POPUPCOLS;
	rows = p->objects/POPUPCOLS;
    }
    p->objects = rows * columns;
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, NULL, NULL);
    CreateSubs(d, p, rows, columns);

    xswa.override_redirect = True;
    popup =  XCreateSimpleWindow (
	    d, RootWindow(d, 0), 30, 40, 500, 500, 0, bp, wp);
    XChangeWindowAttributes (d, popup, CWOverrideRedirect, &xswa);
#undef POPUPCOLS
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

void DeleteSubs(d, p)
    Display *d;
    Parms p;
{
    XDestroySubwindows(d, w);
}

void EndWins(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
}

