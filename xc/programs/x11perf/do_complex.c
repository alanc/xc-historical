#include "x11perf.h"

#define NUM_POINTS 5  /* 5 random points for the complex polygon */
static XPoint *points;
static GC whitegc, blackgc;
static Window w[4];
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };

void InitComplexPoly(d, p)
    Display *d;
    Parms p;
{
    int i, numPoints, j, temp1;

    for (i = 0; i < 4; i++)
	w[i] = None;
    numPoints = (p->objects) * NUM_POINTS;
    points = (XPoint *)malloc(numPoints * sizeof(XPoint));
    i = 0;
    /* for complex we are just using 4 random points */
    for (j = 0; j < numPoints; j++)
    {    
        points[j].x = rand() % WIDTH;
        points[j].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &whitegc, &blackgc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
    
}

void DoComplexPoly(d, p)
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
            XFillPolygon(d, w[0], pgc, curPoint, NUM_POINTS, Complex, 
			 CoordModeOrigin);
            curPoint += NUM_POINTS;
	  }
        if (pgc == whitegc)
            pgc = blackgc;
        else
            pgc = whitegc;
    }
}

void EndComplexPoly(d, p)
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

