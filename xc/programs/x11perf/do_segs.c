#include "x11perf.h"

static XSegment *segments;
static GC bggc, fggc;
static Window w[4];
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };

void InitSizedSegs(d, p, size)
    Display *d;
    Parms   p;
    int     size;
{
    int i;
    int x, y;		/* base of square to draw in			*/
    int x1, y1;		/* offsets into square				*/
    int x1inc, y1inc;   /* How to get to next x1, y1			*/
    int minorphase;     /* # iterations left with current x1inc, y1inc  */
    int majorphase;     /* count 0..3 for which type of x1inc, y1inc    */

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    segments = (XSegment *)malloc((p->objects) * sizeof(XSegment));

    size--;		/* Because endcap counts as 1 pixel		*/

    /* All this x, x1, x1inc, etc. stuff is to create a pattern that
	(1) scans down the screen vertically, with each new segment going
	    into a square of size x size.

	(2) rotates the endpoints clockwise around the square

    */

    x     = 0;  y     = 0;
    x1    = 0;  y1    = 0;
    x1inc = 1;  y1inc = 0;
    minorphase = size;
    majorphase = 0;

    for (i = 0; i < (p->objects); i++) {    
	segments[i].x1 = x + x1;
	segments[i].y1 = y + y1;
	segments[i].x2 = x + size - x1;
	segments[i].y2 = y + size - y1;

	/* Change square to draw segment in */
	y += size + 1;
	if (y >= HEIGHT - size) {
	    /* Go to next column */
	    y = 0;
	    x += size + 1;
	    if (x >= WIDTH - size) {
		x = 0;
	    }
	}

	/* Change coordinates of offsets in square */
	x1 += x1inc;
	y1 += y1inc;

	/* Change increments if needed */
	minorphase--;
	if (minorphase <= 0) {
	    minorphase = size;
	    majorphase = (majorphase + 1) % 4;
	    switch (majorphase) {
		case 0: x1inc =  1; y1inc =  0; break;
		case 1: x1inc =  0; y1inc =  1; break;
		case 2: x1inc = -1; y1inc =  0; break;
		case 3: x1inc =  0; y1inc = -1; break;
	    }
	}
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
    
}
   
void InitSmallSegs(d, p)
    Display *d;
    Parms p;
{
    InitSizedSegs(d, p, 1);
}


void InitMedSegs(d, p)
    Display *d;
    Parms p;
{
    InitSizedSegs(d, p, 10);
}


void InitSegs(d, p)
    Display *d;
    Parms p;
{
    InitSizedSegs(d, p, 500);
}

void DoSegs(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i;

    pgc = bggc;
    for (i=0; i<p->reps; i++)
    {
        XDrawSegments(d, w[0], pgc, segments, p->objects);
        if (pgc == bggc)
            pgc = fggc;
        else
            pgc = bggc;
    }
}

void EndSegs(d, p)
    Display *d;
    Parms p;
{
    int i;
    for (i = 0; i < 4; i++)
	if (w[i] != None)
	    XDestroyWindow(d, w[i]);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(segments);
}

