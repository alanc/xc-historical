#include "x11perf.h"

static XRectangle *rects;
static GC whitegc, blackgc;
static Window w;

void InitRects(d, p)
    Display *d;
    Parms p;
{
    int i, x, y;
    int width = p->special;
    int cols = COLS;

    if (width == 50) 
    {
        p->objects = 100;
        cols = 10;
    }
    rects = (XRectangle *)malloc(p->objects * sizeof(XRectangle));
    i = 0;
    for (x=0; x < cols; x++)
        for (y = 0; y < p->objects/cols; y++) {
	    rects[i].x = x*(width + 1);
            rects[i].y = y*(width+ 1);
            rects[i].width = rects[i].height = width;
	    i++;
	}
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &whitegc, &blackgc);

}

void DoRects(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i;

    pgc = whitegc;
    for (i=0; i < p->reps; i++)
    {
        XFillRectangles(d, w, pgc, rects, p->objects);
        if (pgc == whitegc)
            pgc = blackgc;
        else
            pgc = whitegc;
    }
}

void EndRects(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
    XFreeGC(d, whitegc);
    XFreeGC(d, blackgc);
    free(rects);
}

