#include "x11perf.h"

static XPoint *points;
static GC whitegc, blackgc;
static Window w;

void InitDots(d, p)
    Display *d;
    Parms p;
{
    int i, x, y;

    i = 0;
    points = (XPoint *)malloc(p->objects * sizeof(XPoint));
    for (x = 0; x < COLS; x++)
	for (y = 0; y < p->objects/COLS; y++) {
	    points[i].x = 2*x;
	    points[i].y = 2*y;
	    i++;
	}
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &whitegc, &blackgc);
}

void DoDots(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i;

    pgc = whitegc;
    for (i=0; i < p->reps; i++)
    {
        XDrawPoints(d, w, pgc, points, p->objects, CoordModeOrigin);
        if (pgc == whitegc)
            pgc = blackgc;
        else
            pgc = whitegc;
    }
}

void EndDots(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
    XFreeGC(d, whitegc);
    XFreeGC(d, blackgc);
    free(points);
}

