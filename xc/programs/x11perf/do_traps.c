#include "x11perf.h"

#define NUM_POINTS 4  /* 4 points to a trapezoid */
static XPoint *points;
static GC whitegc, blackgc;
static Window w[4];
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };

void InitTraps(d, p)
    Display *d;
    Parms p;
{
    int i, numPoints, j;

    for (i = 0; i < 4; i++)
	w[i] = None;
    numPoints = (p->objects) * NUM_POINTS;
    points = (XPoint *)malloc(numPoints * sizeof(XPoint));
    i = 0;
    for (j = 0; j < p->objects; j++)
    {    
        points[i].x = rand() % (WIDTH - 100);
        points[i++].y = rand() % HEIGHT;
        points[i].x = rand() % (WIDTH - 100);
        points[i++].y = rand() % HEIGHT;
        /* make the last 2 points correspond to the first 2 => trapezoid */
        points[i].x = points[i-1].x + (rand() % (WIDTH - points[i-1].x));
        points[i].y = points[i-1].y;
        i++;
        points[i].x = points[i-3].x + (rand() % (WIDTH - points[i-3].x));
        points[i].y = points[i-3].y;
        i++;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &whitegc, &blackgc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
    
}

void DoTraps(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i, j;
    XPoint *curPoint;

    pgc = whitegc;
    for (i=0; i<p->reps; i++)
    {
        curPoint = points;
        for (j=0; j < p->objects; j++) {
            XFillPolygon(d, w[0], pgc, curPoint, NUM_POINTS, Convex, 
			 CoordModeOrigin);
            curPoint += NUM_POINTS;
	  }
        if (pgc == whitegc)
            pgc = blackgc;
        else
            pgc = whitegc;
    }
}

void EndTraps(d, p)
    Display *d;
    Parms p;
{
    int i;
    for (i = 0; i < 4; i++)
	if (w[i] != None)
	    XDestroyWindow(d, w[i]);
    XFreeGC(d, whitegc);
    XFreeGC(d, blackgc);
    free(points);
}

