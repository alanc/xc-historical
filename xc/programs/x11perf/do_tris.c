#include "x11perf.h"

#define NUM_POINTS 3   /* 3 points to a triangle */
static XPoint *points;
static GC whitegc, blackgc;
static Window w[4];
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };

void InitTriangles(d, p)
    Display *d;
    Parms p;
{
    int i, numPoints;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    numPoints = (p->objects) * NUM_POINTS;  
    points = (XPoint *)malloc(numPoints * sizeof(XPoint));
    for (i = 0; i < numPoints; i++)
    {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &whitegc, &blackgc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
    
}

void DoTriangles(d, p)
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

void EndTriangles(d, p)
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

